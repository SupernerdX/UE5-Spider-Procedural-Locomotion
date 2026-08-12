// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelState.h"
#include "VoxelWorld.h"
#include "VoxelLayers.h"
#include "VoxelSubsystem.h"
#include "VoxelTaskContext.h"
#include "VoxelInvalidationQueue.h"
#include "Bulk/VoxelBulkPtr.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#if WITH_EDITOR
#include "LevelEditorViewport.h"
#endif

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelTraceRegion, false,
	"voxel.TraceRegion",
	"Emit Unreal Insight trace regions when state is started/completed");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelHideTaskCount, false,
	"voxel.HideTaskCount",
	"Hide the debug task count");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelShowLatency, false,
	"voxel.ShowLatency",
	"Show how long the voxel world takes to update");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelLogStateTime, false,
	"voxel.LogStateTime",
	"Show how long the voxel world takes to compute, summing async cost across all threads");

VOXEL_RUN_ON_STARTUP_GAME()
{
	if (FVoxelUtilities::IsDevWorkflow())
	{
		GVoxelTraceRegion = true;
	}
}

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelState);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelState::FVoxelState(
	const TSharedRef<const FVoxelConfig>& Config,
	const TSharedRef<FVoxelLayers>& Layers,
	const TSharedPtr<const FVoxelState>& PreviousState)
	: Config(Config)
	, Layers(Layers)
	, SurfaceTypeTable(FVoxelSurfaceTypeTable::Get())
	, InvalidationQueue(FVoxelInvalidationQueue::Create())
	// Reuse the same shared ref, as otherwise chunks computed in the last state won't invalidate us
	, TimestampRef(PreviousState ? PreviousState->TimestampRef : MakeShared<FVoxelCounter64>())
	, Timestamp(TimestampRef->Get())
	, bHasPreviousState(PreviousState.IsValid())
	, StateIndex(INLINE_LAMBDA
	{
		static int32 StateIndexCounter;
		return ++StateIndexCounter;
	})
	, StatName(FString::Printf(TEXT("Voxel State %d"), StateIndex))
	, TaskContext(FVoxelTaskContext::Create(STATIC_FNAME("FVoxelState"), Config->MaxBackgroundTasks))
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (GVoxelTraceRegion)
	{
		const uint64 RegionId = TRACE_BEGIN_REGION_WITH_ID(*FString::Printf(
			TEXT("Computing Voxel State %d%s"),
			StateIndex,
			bHasPreviousState ? TEXT("") : TEXT(" FULL RECOMPUTE")));

		const uint64* RegionIdPtr = new uint64(RegionId);

		RegionIdRef = MakeShareable_CustomDestructor(RegionIdPtr, [=]
		{
			TRACE_END_REGION_WITH_ID(*RegionIdPtr);
			delete RegionIdPtr;
		});
	}
	if (GVoxelDumpStatus)
	{
		TaskContext->bTrackPromisesCallstacks = true;
	}
	if (GVoxelLogStateTime)
	{
		TaskContext->bComputeTotalTime = true;
	}

	TaskContext->BulkDataTimestamp = FVoxelBulkPtr::GetGlobalTimestamp();

	if (AreVoxelStatsEnabled())
	{
		TaskContext->LambdaWrapper = [StatName = StatName](TVoxelUniqueFunction<void()> Lambda)
		{
			return [StatName, Lambda = MoveTemp(Lambda)]
			{
				VOXEL_SCOPE_COUNTER_FNAME(StatName);
				Lambda();
			};
		};
	}

	FVoxelTaskScope Scope(*TaskContext);

	const bool bIsServer = INLINE_LAMBDA
	{
		if (!FApp::CanEverRender())
		{
			return true;
		}

		const UWorld* World = Config->World.Resolve();
		if (!ensure(World))
		{
			return false;
		}

		return World->GetNetMode() == NM_DedicatedServer;
	};

	static const TArray<UScriptStruct*> Structs = GetDerivedStructs<FVoxelSubsystem>();

	for (const UScriptStruct* Struct : Structs)
	{
		TUniquePtr<FVoxelSubsystem> Subsystem = MakeUniqueStruct<FVoxelSubsystem>(Struct);
		if (bIsServer &&
			!Subsystem->ShouldCreateOnServer())
		{
			continue;
		}

		Subsystem->PrivateState = this;
		StructToSubsystem.Add_CheckNew(Struct, MoveTemp(Subsystem));
	}

	if (PreviousState)
	{
		VOXEL_SCOPE_COUNTER("LoadFromPrevious");

		for (const auto& It : StructToSubsystem)
		{
			FVoxelSubsystem& Subsystem = *It.Value;
			FVoxelSubsystem& PreviousSubsystem = *PreviousState->StructToSubsystem.FindChecked(It.Key);

			ensure(!PreviousSubsystem.bPrivateIsPreviousSubsystem);
			PreviousSubsystem.bPrivateIsPreviousSubsystem = true;

			Subsystem.GCObjects_RequiresLock = MoveTemp(PreviousSubsystem.GCObjects_RequiresLock);
			Subsystem.LoadFromPrevious(PreviousSubsystem);

			if (Subsystem.GCObjects_RequiresLock.Num() > 0)
			{
				Voxel::AsyncTask([&Subsystem]
				{
					VOXEL_SCOPE_COUNTER("Cleanup GCObjects");

					Subsystem.GCObjects_RequiresLock.RemoveAllSwap([](const TWeakPtr<IVoxelSubsystemGCObject>& Object)
					{
						return !Object.IsValid();
					});
				});
			}
		}

		ActiveDrawGroups = PreviousState->ActiveDrawGroups;
	}

	DrawGroup = FVoxelDebugDrawGroup::Create();
	ActiveDrawGroups.Add(DrawGroup);

	TaskContext->DrawGroup = DrawGroup;

	for (const auto& It : StructToSubsystem)
	{
		It.Value->Initialize();
	}

	check(!bAllSubsystemInitialized);
	bAllSubsystemInitialized = true;

	for (const auto& It : StructToSubsystem)
	{
		Voxel::AsyncTask([Subsystem = It.Value.Get()]
		{
			Subsystem->Compute();
		});
	}

	LOG_VOXEL(VeryVerbose, "Starting to compute State %d", StateIndex);
}

FVoxelState::~FVoxelState()
{
	VOXEL_FUNCTION_COUNTER();

	if (TaskContext)
	{
		TaskContext->CancelTasks();
		// Flush to ensure no task is still in flight that can reference us
		TaskContext->FlushAllTasks();

		ensure(TaskContext.IsUnique());
		TaskContext.Reset();
	}

	{
		VOXEL_SCOPE_COUNTER("Delete subsystems");

		for (auto& It : StructToSubsystem)
		{
			VOXEL_SCOPE_COUNTER_FNAME(It.Key->GetFName());

			checkVoxelSlow(It.Value->PrivateState == this);
			It.Value->PrivateState = nullptr;

			It.Value.Reset();
		}
		StructToSubsystem.Empty();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelState::Tick()
{
	VOXEL_SCOPE_COUNTER_FNAME(StatName);
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (IsReadyToRender())
	{
		OnReadyToRender.Broadcast();
	}

	if (!TaskContext)
	{
		return;
	}

	const int32 NumPendingTasks = TaskContext->GetNumPendingTasks();

	ProgressInfo.MaxNumTasks = FMath::Max(ProgressInfo.MaxNumTasks, NumPendingTasks);
	ProgressInfo.Progress = FMath::Max(ProgressInfo.Progress, (ProgressInfo.MaxNumTasks - NumPendingTasks) / float(ProgressInfo.MaxNumTasks));

	if (GVoxelHideTaskCount)
	{
		return;
	}

	const AVoxelWorld* VoxelWorld = Config->VoxelWorld.Resolve();
	if (!ensure(VoxelWorld))
	{
		return;
	}

	UWorld* World = VoxelWorld->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FString WorldType;
	if (!World->IsGameWorld())
	{
		WorldType = "Editor";
	}
	else
	{
		WorldType = LexToString(World->WorldType);

		const ENetMode NetMode = World->GetNetMode();
		if (NetMode != NM_Standalone)
		{
			WorldType += " " + ToString(NetMode);
		}
	}

	GEngine->AddOnScreenDebugMessage(
		uint64(VoxelWorld),
		0.1f,
		FColor::White,
		FString::Printf(TEXT("Updating %s (%s): %d tasks left using %d worker threads"),
			*VoxelWorld->GetActorNameOrLabel(),
			*WorldType,
			NumPendingTasks,
			FVoxelUtilities::GetNumBackgroundWorkerThreads()));

#if WITH_EDITOR
	// Ensure message is visible
	extern UNREALED_API FLevelEditorViewportClient* GCurrentLevelEditingViewportClient;
	if (GCurrentLevelEditingViewportClient)
	{
		GCurrentLevelEditingViewportClient->SetShowStats(true);
	}
#endif
}

void FVoxelState::Render(FVoxelRuntime& Runtime)
{
	VOXEL_SCOPE_COUNTER_FNAME(StatName);
	VOXEL_FUNCTION_COUNTER();
	ensure(TaskContext->IsComplete());

	RegionIdRef.Reset();

	if (TaskContext->bComputeTotalTime)
	{
		LOG_VOXEL(Log, "Rendering state %d - took %s (total time: %s) to compute (%llu frames) %s",
			StateIndex,
			*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - StartTime),
			*FVoxelUtilities::SecondsToString(TaskContext->TotalTime.Get()),
			GFrameCounter - StartFrame,
			bHasPreviousState ? TEXT("") : TEXT("FULL RECOMPUTE"));
	}

	// Exit now so that tasks queued below are never queued in the task context
	// (eg render tasks for nanite)
	ensure(TaskContext.IsUnique());
	TaskContext.Reset();

	ProgressInfo.Progress = 1.f;

	ensure(!bIsRendered);
	bIsRendered = true;

	const AVoxelWorld* VoxelWorld = Config->VoxelWorld.Resolve();
	if (!ensure(VoxelWorld))
	{
		return;
	}

	LOG_VOXEL(VeryVerbose, "Rendering state %d - took %s to compute (%llu frames)",
		StateIndex,
		*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - StartTime),
		GFrameCounter - StartFrame);

	if (GVoxelShowLatency)
	{
		GEngine->AddOnScreenDebugMessage(
			uint64(VoxelWorld) + 32488457,
			10.f,
			FColor::Orange,
			FString::Printf(TEXT("%s State %5d: %3lld frames to compute (%s)"),
				*VoxelWorld->GetActorNameOrLabel(),
				StateIndex,
				int64(GFrameCounter) - StartFrame,
				*FVoxelUtilities::SecondsToString(FPlatformTime::Seconds() - StartTime)));
	}

	for (const auto& It : StructToSubsystem)
	{
		It.Value->Render(Runtime);
	}

	OnRendered.Broadcast();

	if (DrawGroup)
	{
		DrawGroup->PushGroup_EnsureNew_AnyThread(Layers->World);
	}
}

void FVoxelState::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();

	for (const auto& It : StructToSubsystem)
	{
		It.Value->AddReferencedObjects(Collector);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelState::IsReadyToRender() const
{
	return TaskContext->IsComplete();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSubsystem& FVoxelState::GetSubsystem(const UScriptStruct* Struct) const
{
	check(bAllSubsystemInitialized);
	return *StructToSubsystem.FindChecked(Struct);
}

FVoxelSubsystem* FVoxelState::GetSubsystemPtr(const UScriptStruct* Struct) const
{
	check(bAllSubsystemInitialized);

	const TUniquePtr<FVoxelSubsystem>* SubsystemPtr = StructToSubsystem.Find(Struct);
	if (!SubsystemPtr)
	{
		return nullptr;
	}

	return SubsystemPtr->Get();
}