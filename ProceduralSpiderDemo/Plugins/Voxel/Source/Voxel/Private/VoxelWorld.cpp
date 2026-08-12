// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelWorld.h"
#include "VoxelState.h"
#include "VoxelLayer.h"
#include "VoxelRuntime.h"
#include "VoxelLayerStack.h"
#include "VoxelTaskContext.h"
#include "VoxelWorldRootComponent.h"
#include "VoxelStampComponentUtilities.h"
#include "Collision/VoxelCollisionInvoker.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "Navigation/VoxelNavigationComponent.h"
#include "GameFramework/Pawn.h"
#include "Misc/ScopedSlowTask.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Selection.h"
#include "ScopedTransaction.h"
#include "Editor/EditorEngine.h"
#endif

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelDumpStatus, false,
	"voxel.DumpStatus",
	"Dump status of the voxel world in the log");

VOXEL_RUN_ON_STARTUP_GAME()
{
	if (FParse::Param(FCommandLine::Get(), TEXT("DumpVoxelStatus")))
	{
		GVoxelDumpStatus = true;
	}
}

VOXEL_RUN_ON_STARTUP_GAME()
{
	Voxel::OnRefreshAll.AddLambda([]
	{
		ForEachObjectOfClass_Copy<AVoxelWorld>([&](AVoxelWorld& VoxelWorld)
		{
			if (!VoxelWorld.GetWorld())
			{
				return;
			}

			VoxelWorld.DestroyRuntime();
			VoxelWorld.CreateRuntime();
		});
	});
}

class FVoxelWorldDebugTicker : public FVoxelSingleton
{
public:
	//~ Begin FVoxelSingleton Interface
	virtual void Tick() override
	{
		VOXEL_FUNCTION_COUNTER();

#if WITH_EDITOR
		CheckForMissingCollisionInvokers();
#endif

		if (!GVoxelDumpStatus ||
			GFrameCounter % 10 != 0)
		{
			return;
		}

		ForEachObjectOfClass_Copy<AVoxelWorld>([](const AVoxelWorld& VoxelWorld)
		{
			if (VoxelWorld.IsTemplate())
			{
				return;
			}

			LOG_VOXEL(Log, "%s: %s",
				*VoxelWorld.GetPathName(),
				VoxelWorld.IsVoxelWorldReady() && !VoxelWorld.IsProcessingNewState()
				? TEXT("Ready")
				: VoxelWorld.IsVoxelWorldReady() && VoxelWorld.IsProcessingNewState()
				? TEXT("Ready, processing new state")
				: VoxelWorld.IsRuntimeCreated()
				? TEXT("Processing")
				: TEXT("Not created"));

			if (!VoxelWorld.IsRuntimeCreated() ||
				(VoxelWorld.IsVoxelWorldReady() && !VoxelWorld.IsProcessingNewState()))
			{
				return;
			}

			const TSharedPtr<FVoxelState> NewState = VoxelWorld.GetRuntime()->GetNewestState();
			if (!ensure(NewState))
			{
				return;
			}

			const FVoxelTaskContext* TaskContext = NewState->GetTaskContext();
			if (!ensure(TaskContext))
			{
				return;
			}

			TaskContext->DumpToLog();
		});
	}
	//~ End FVoxelSingleton Interface

#if WITH_EDITOR
	static void CheckForMissingCollisionInvokers()
	{
		VOXEL_FUNCTION_COUNTER();

		ForEachObjectOfClass_Copy<AVoxelWorld>([](AVoxelWorld& VoxelWorld)
		{
			if (VoxelWorld.IsTemplate() ||
				!VoxelWorld.bWarnAboutMissingInvokers ||
				VoxelWorld.bCollisionInvokerComponentSeen)
			{
				return;
			}

			const UWorld* World = VoxelWorld.GetWorld();
			if (!World ||
				World->GetNetMode() != NM_DedicatedServer)
			{
				return;
			}

			if (!VoxelWorld.bPlayerSeen)
			{
				if (UGameplayStatics::GetNumPlayerStates(World) > 0)
				{
					VoxelWorld.bPlayerSeen = true;
				}
			}

			ForEachObjectOfClass<UVoxelCollisionInvokerComponent>([&](const UVoxelCollisionInvokerComponent& Component)
			{
				if (Component.GetWorld() == World)
				{
					VoxelWorld.bCollisionInvokerComponentSeen = true;
				}
			});
		});
	}
#endif
};
FVoxelWorldDebugTicker* GVoxelWorldDebugTicker = new FVoxelWorldDebugTicker();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelWorld::AVoxelWorld()
{
	RootComponent = CreateDefaultSubobject<UVoxelWorldRootComponent>("RootComponent");

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif
	ComponentSettings.bCastFarShadow = true;

	LayerStack = UVoxelLayerStack::Default();

	VisibilityCollision.SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	InvokerCollision.SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Make sure defaults are initialized properly

		UVoxelLayerStack::Default();
		UVoxelHeightLayer::Default();
		UVoxelVolumeLayer::Default();
	}

	NavigationMeshComponent = UVoxelNavigationComponent::StaticClass();
}

AVoxelWorld::~AVoxelWorld()
{
	ensure(!Runtime.IsValid());
}

#if WITH_EDITOR
void AVoxelWorld::SetupForPreview()
{
	VOXEL_FUNCTION_COUNTER();

	MegaMaterial = UVoxelMegaMaterial::CreateTransient();

	LODQuality.EditorQuality.Min = 2.f;
	LODQuality.EditorQuality.Max = 2.f;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool AVoxelWorld::IsRuntimeCreated() const
{
	return Runtime.IsValid();
}

bool AVoxelWorld::IsVoxelWorldReady() const
{
	return
		Runtime &&
		Runtime->GetState();
}

bool AVoxelWorld::IsProcessingNewState() const
{
	return
		Runtime &&
		Runtime->IsProcessingNewState();
}

float AVoxelWorld::GetProgress() const
{
	if (!Runtime)
	{
		return 1.f;
	}

	const TSharedPtr<FVoxelState> NewState = Runtime->GetNewState();
	if (!NewState)
	{
		return 1.f;
	}

	return NewState->GetProgress();
}

int32 AVoxelWorld::GetNumPendingTasks() const
{
	if (!Runtime)
	{
		return 0;
	}

	const TSharedPtr<FVoxelState> NewState = Runtime->GetNewState();
	if (!NewState)
	{
		return 0;
	}

	const FVoxelTaskContext* TaskContext = NewState->GetTaskContext();
	if (!ensureVoxelSlow(TaskContext))
	{
		return 0;
	}

	return TaskContext->GetNumPendingTasks();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::OnInvalidated(const FOnVoxelWorldEvent Delegate)
{
	this->OnInvalidated(MakeLambdaDelegate([=]
	{
		Delegate.ExecuteIfBound();
	}));
}

void AVoxelWorld::OnInvalidated(const FSimpleDelegate Delegate)
{
	if (!Runtime)
	{
		VOXEL_MESSAGE(Error, "Cannot bind OnInvalidated: the runtime of {0} is not created", this);
		return;
	}

	Runtime->OnInvalidated.Add(Delegate);
}

void AVoxelWorld::OnNextStateRendered(const FOnVoxelWorldEvent Delegate)
{
	OnNextStateRendered(MakeLambdaDelegate([=]
	{
		Delegate.ExecuteIfBound();
	}));
}

void AVoxelWorld::OnNextStateRendered(const FSimpleDelegate Delegate)
{
	if (!Runtime)
	{
		VOXEL_MESSAGE(Error, "Cannot bind OnNextStateRendered: the runtime of {0} is not created", this);
		return;
	}

	if (!Runtime->OnNextStateRendered)
	{
		Runtime->OnNextStateRendered = MakeShared<FSimpleMulticastDelegate>();
	}

	Runtime->OnNextStateRendered->Add(Delegate);
}

void AVoxelWorld::BindOnRuntimeCreated(const FOnVoxelWorldEvent Delegate)
{
	OnRuntimeCreated.Add(MakeLambdaDelegate([=]
	{
		Delegate.ExecuteIfBound();
	}));
}

void AVoxelWorld::BindOnRuntimeDestroyed(const FOnVoxelWorldEvent Delegate)
{
	OnRuntimeDestroyed.Add(MakeLambdaDelegate([=]
	{
		Delegate.ExecuteIfBound();
	}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::CreateRuntime()
{
	VOXEL_FUNCTION_COUNTER();

	if (Runtime)
	{
		return;
	}

	Runtime = MakeShared<FVoxelRuntime>(*this);
	Runtime->Initialize();

	OnRuntimeCreated.Broadcast();
}

void AVoxelWorld::DestroyRuntime()
{
	VOXEL_FUNCTION_COUNTER();

	if (!Runtime)
	{
		return;
	}

	for (const TVoxelObjectPtr<USceneComponent>& WeakComponent : Runtime->AllComponents)
	{
		if (USceneComponent* Component = WeakComponent.Resolve())
		{
			Component->DestroyComponent();
		}
	}

	ensure(Runtime.IsUnique());
	Runtime.Reset();

	OnRuntimeDestroyed.Broadcast();
}

TSharedPtr<FVoxelRuntime> AVoxelWorld::GetRuntime() const
{
	return Runtime;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::BeginPlay()
{
	VOXEL_FUNCTION_COUNTER();

	Super::BeginPlay();

	// Make sure all stamps are loaded first
	FVoxelStampComponentUtilities::DispatchBeginPlay(GetWorld());

	if (!bCreateRuntimeOnBeginPlay)
	{
		return;
	}

	CreateRuntime();

	if (!bWaitOnBeginPlay)
	{
		return;
	}

	VOXEL_SCOPE_COUNTER_FORMAT("Waiting for Voxel World %s", *GetActorNameOrLabel());

	TOptional<FScopedSlowTask> SlowTask;
	if (GIsEditor)
	{
		SlowTask.Emplace(1.f, INVTEXT("Generating Voxel World"));
		SlowTask->MakeDialog(true, true);
		SlowTask->EnterProgressFrame(0.f);
	}

	float LastProgress = 0.f;
	double LastLogTime = FPlatformTime::Seconds();

	// Stop as soon as a state is ready to render
	while (
		!Runtime->GetState() ||
		!Runtime->GetState()->IsRendered())
	{
		VOXEL_SCOPE_COUNTER("Loop");

		if (SlowTask &&
			SlowTask->ShouldCancel())
		{
			break;
		}

		if (FPlatformTime::Seconds() - LastLogTime > 0.1)
		{
			LastLogTime = FPlatformTime::Seconds();
			LOG_VOXEL(Log, "Waiting for Voxel World %s", *GetActorNameOrLabel());

			if (GVoxelDumpStatus)
			{
				if (const TSharedPtr<FVoxelState> State = Runtime->GetNewestState())
				{
					if (const FVoxelTaskContext* TaskContext = State->GetTaskContext())
					{
						TaskContext->DumpToLog();
					}
				}
			}
		}

		INLINE_LAMBDA
		{
			if (!SlowTask)
			{
				return;
			}

			const TSharedPtr<FVoxelState> State = Runtime->GetNewestState();
			if (!ensure(State))
			{
				return;
			}

			const FVoxelTaskContext* TaskContext = State->GetTaskContext();
			if (!ensure(TaskContext))
			{
				return;
			}

			const float NewProgress = State->GetProgress();
			const float ProgressDelta = NewProgress - LastProgress;
			LastProgress = NewProgress;

			SlowTask->EnterProgressFrame(ProgressDelta, FText::FromString(FString::Printf(
				TEXT("Waiting for Voxel World %s - %d voxel tasks remaining\n\nSet WaitOnBeginPlay to false on your Voxel World to disable this\n\n"),
				*GetActorNameOrLabel(),
				TaskContext->GetNumPendingTasks())));
		};

		Runtime->Tick();

		Voxel::ForceTick();

		if (!ensure(Runtime))
		{
			// Some game task could have destroyed the runtime
			break;
		}

		if (Runtime->IsProcessingNewState())
		{
			VOXEL_SCOPE_COUNTER("Sleep");

			// Force ticking as fast as we can is a waste, we're waiting on async tasks
			FPlatformProcess::Sleep(0.001f);
		}
	}
}

void AVoxelWorld::BeginDestroy()
{
	VOXEL_FUNCTION_COUNTER();

	if (Runtime)
	{
		DestroyRuntime();
	}

	Super::BeginDestroy();
}

void AVoxelWorld::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	VOXEL_FUNCTION_COUNTER();

	// In the editor, Destroyed is called but EndPlay isn't

	if (Runtime)
	{
		DestroyRuntime();
	}

#if WITH_EDITOR
	if (bWarnAboutMissingInvokers &&
		bPlayerSeen &&
		!bCollisionInvokerComponentSeen)
	{
		const UWorld* World = GetWorld();
		if (World &&
			World->GetNetMode() == NM_DedicatedServer)
		{
			VOXEL_MESSAGE(Error,
				"No voxel collision invokers found on dedicated server. This will lead to players falling through the world.\n"
				"Either add a Voxel Collision Invoker to your character class or set WarnAboutMissingInvokers to false on your Voxel World.");
		}
	}
#endif

	Super::EndPlay(EndPlayReason);
}

void AVoxelWorld::Destroyed()
{
	VOXEL_FUNCTION_COUNTER();

	if (Runtime)
	{
		DestroyRuntime();
	}

	Super::Destroyed();
}

void AVoxelWorld::OnConstruction(const FTransform& Transform)
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (!Runtime &&
		GetWorld() &&
		!GetWorld()->IsGameWorld() &&
		!IsTemplate() &&
		// IsAllowCommandletRendering sometimes need the voxel world in editor
		(!IsRunningCommandlet() || IsAllowCommandletRendering()))
	{
		CreateRuntime();
	}
#endif
}

void AVoxelWorld::PostLoad()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostLoad();

	VisibilityCollision.LoadProfileData(false);
	InvokerCollision.LoadProfileData(false);
}

void AVoxelWorld::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);

	{
		constexpr uint64 Magic = MAKE_TAG_64("VOXEL_LA");
		const int64 Start = Ar.Tell();

		uint64 MagicValue = Magic;
		Ar << MagicValue;

		if (!ensure(MagicValue == Magic))
		{
			Ar.Seek(Start);
			return;
		}
	}

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion,
		RemoveSculptData,
		RemoveSculptDataV2
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	if (Version < FVersion::RemoveSculptData)
	{
		FVoxelUtilities::SerializeBulkData(
			this,
			BulkData,
			Ar,
			[](FArchive& BulkDataAr)
			{
				int32 LegacyVersion = 1;
				BulkDataAr << LegacyVersion;
				if (!ensure(LegacyVersion == 1))
				{
					BulkDataAr.SetError();
					return;
				}

				int32 NumLODs = 0;
				BulkDataAr << NumLODs;

				for (int32 LOD = 0; LOD < NumLODs; LOD++)
				{
					bool bIsValid = false;
					BulkDataAr << bIsValid;

					if (!bIsValid)
					{
						continue;
					}

					TVoxelArray<FIntVector> Keys;
					BulkDataAr << Keys;

					for (const FIntVector& Key : Keys)
					{
						int32 ChunkVersion = 1;
						BulkDataAr << ChunkVersion;
						if (!ensure(ChunkVersion == 1))
						{
							BulkDataAr.SetError();
							return;
						}

						TVoxelStaticArray<float, 512> Distances{ NoInit };
						BulkDataAr << Distances;

						TVoxelStaticArray<uint64, 512> Materials{ NoInit };
						BulkDataAr << Materials;
					}
				}
			});
	}
	else if (Version < FVersion::RemoveSculptDataV2)
	{
		FVoxelUtilities::SerializeBulkData(
			this,
			BulkData,
			Ar,
			[&](FArchive& BulkDataAr)
			{
				ensure(BulkDataAr.IsLoading());

				TVoxelMap<FGuid, TVoxelArray64<uint8>> GuidToBytes;
				BulkDataAr << GuidToBytes;
			});
	}
	else
	{
		ensure(Version == FVersion::LatestVersion);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool AVoxelWorld::Modify(const bool bAlwaysMarkDirty)
{
	if (bDisableModify)
	{
		return false;
	}

	return Super::Modify(bAlwaysMarkDirty);
}

void AVoxelWorld::PostEditUndo()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditUndo();

	if (IsValid(this))
	{
		if (!Runtime)
		{
			CreateRuntime();
		}
	}
	else
	{
		if (Runtime)
		{
			DestroyRuntime();
		}
	}
}

void AVoxelWorld::PreEditChange(FProperty* PropertyThatWillChange)
{
	VOXEL_FUNCTION_COUNTER();

	// Temporarily remove runtime components to avoid expensive re-registration

	TSet<UActorComponent*>& Components = ConstCast(GetComponents());

	if (Runtime)
	{
		VOXEL_SCOPE_COUNTER_NUM("Remove", Runtime->AllComponents.Num(), 1);

		for (const TVoxelObjectPtr<USceneComponent>& WeakComponent : Runtime->AllComponents)
		{
			const USceneComponent* Component = WeakComponent.Resolve();
			if (!ensure(Component))
			{
				continue;
			}

			ensure(Components.Remove(Component));
		}
	}

	Super::PreEditChange(PropertyThatWillChange);

	if (Runtime)
	{
		VOXEL_SCOPE_COUNTER_NUM("Add", Runtime->AllComponents.Num(), 1);

		for (const TVoxelObjectPtr<USceneComponent>& WeakComponent : Runtime->AllComponents)
		{
			USceneComponent* Component = WeakComponent.Resolve();
			if (!ensure(Component))
			{
				continue;
			}

			Components.Add(Component);
		}
	}
}

void AVoxelWorld::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	// Temporarily remove runtime components to avoid expensive re-registration

	TSet<UActorComponent*>& Components = ConstCast(GetComponents());

	if (Runtime)
	{
		VOXEL_SCOPE_COUNTER_NUM("Remove", Runtime->AllComponents.Num(), 1);

		for (const TVoxelObjectPtr<USceneComponent>& WeakComponent : Runtime->AllComponents)
		{
			const USceneComponent* Component = WeakComponent.Resolve();
			if (!ensure(Component))
			{
				continue;
			}

			ensure(Components.Remove(Component));
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (Runtime)
	{
		VOXEL_SCOPE_COUNTER_NUM("Add", Runtime->AllComponents.Num(), 1);

		for (const TVoxelObjectPtr<USceneComponent>& WeakComponent : Runtime->AllComponents)
		{
			USceneComponent* Component = WeakComponent.Resolve();
			if (!ensure(Component))
			{
				continue;
			}

			Components.Add(Component);
		}
	}
}

bool AVoxelWorld::CanBeBaseForCharacter(APawn* Pawn) const
{
	if (!Pawn)
	{
		return Super::CanBeBaseForCharacter(Pawn);
	}

	const UPrimitiveComponent* MovementBase = Pawn->GetMovementBase();
	if (!MovementBase ||
		MovementBase->GetOwner() != this ||
		MovementBase == GetRootComponent())
	{
		return Super::CanBeBaseForCharacter(Pawn);
	}

	// MovementBase is a component owned by us that isn't our root component - this is unsafe

	VOXEL_MESSAGE(Error,
		"{0} should inherit from VoxelCharacter or manually override SetBase.\n"
		"Not doing so will lead to character teleportation when the Voxel World updates.\n"
		"See VoxelCharacter.h for more info",
		Pawn->GetClass());

	return Super::CanBeBaseForCharacter(Pawn);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();

	Super::AddReferencedObjects(InThis, Collector);

	const TSharedPtr<FVoxelRuntime> Runtime = CastChecked<AVoxelWorld>(InThis)->Runtime;
	if (!Runtime)
	{
		return;
	}

	Runtime->AddReferencedObjects(Collector);
}

#if WITH_EDITOR
void AVoxelWorld::CreateNewIfNeeded_EditorOnly(const UObject* WorldContext)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return;
	}

	for (AVoxelWorld* It : TActorRange<AVoxelWorld>(World))
	{
		return;
	}

	AVoxelWorld* Actor = World->SpawnActor<AVoxelWorld>();
	if (!ensure(Actor))
	{
		return;
	}

	if (!GEditor)
	{
		return;
	}

	const TSharedRef<FVoxelNotification> Notification = FVoxelNotification::Create("Voxel World added to " + World->GetName());

	Notification->AddButton_ExpireOnClick(
		"Select",
		"Will select newly created Voxel World",
		MakeWeakObjectPtrLambda(Actor, [Actor]
		{
			const FScopedTransaction Transaction(INVTEXT("Select Voxel World"));
			GEditor->GetSelectedActors()->Modify();
			GEditor->SelectNone(true, true);
			GEditor->SelectActor(Actor, true, true);
		}));

	Notification->ExpireAndFadeoutIn(10.f);
}
#endif