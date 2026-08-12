// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPCGHelpers.h"
#include "VoxelMessage.h"
#include "VoxelTaskContext.h"
#include "VoxelPCGTracker.h"
#include "VoxelPCGCallstack.h"
#include "VoxelInvalidationQueue.h"
#include "PCGGraph.h"
#include "PCGComponent.h"

thread_local TVoxelArray<TSharedPtr<const FVoxelPCGCallstack>> GVoxelPCGCallstacks;

class FVoxelPCGHelperSingleton : public FVoxelSingleton
{
public:
#if WITH_EDITOR
	FVoxelCriticalSection CriticalSection;
	TVoxelMap<FName, TVoxelArray<TWeakPtr<FVoxelTaskContext>>> NameToContexts_RequiresLock;
#endif

	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override
	{
		GVoxelMessageManager->GatherCallstacks.Add([](const TSharedRef<FVoxelMessage>& Message)
		{
			if (GVoxelPCGCallstacks.Num() > 0)
			{
				const TSharedRef<const FVoxelPCGCallstack> Callstack = GVoxelPCGCallstacks.Last().ToSharedRef();

				const TSharedRef<FVoxelMessageToken_PCGCallstack> Token = MakeShared<FVoxelMessageToken_PCGCallstack>();
				Token->Callstack = Callstack;
				Token->WeakObject = Callstack->Node;
				Message->AddToken(Token);

				Message->AddToken(FVoxelMessageTokenFactory::CreateTextToken(" Component: "));
				Message->AddToken(FVoxelMessageTokenFactory::CreateObjectToken(Callstack->Component));
			}
		});
	}
#if WITH_EDITOR
	virtual void Tick() override
	{
		VOXEL_FUNCTION_COUNTER();

		extern VOXEL_API bool GVoxelHideTaskCount;
		if (GVoxelHideTaskCount)
		{
			return;
		}

		VOXEL_SCOPE_LOCK(CriticalSection);

		TVoxelArray<FString> Counters;
		for (auto& It : NameToContexts_RequiresLock)
		{
			int32 Count = 0;
			for (auto ContextIt = It.Value.CreateIterator(); ContextIt; ContextIt++)
			{
				const TSharedPtr<FVoxelTaskContext> Context = ContextIt->Pin();
				if (!Context)
				{
					ContextIt.RemoveCurrentSwap();
					continue;
				}

				Count += Context->GetNumPendingTasks();
			}

			if (Count == 0)
			{
				continue;
			}

			Counters.Add(FString::Printf(TEXT("%s: %d"), *It.Key.ToString(), Count));
		}

		if (Counters.Num() == 0)
		{
			return;
		}

		GEngine->AddOnScreenDebugMessage(
			0x34D7F1434B62_u64,
			0.1f,
			FColor::White,
			"Voxel PCG tasks: " + FString::Join(Counters, TEXT(", ")));
	}
#endif
	//~ End FVoxelSingleton Interface
};
FVoxelPCGHelperSingleton* GVoxelPCGHelperSingleton = new FVoxelPCGHelperSingleton();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FName UVoxelPCGSettings::GetDefaultNodeName() const
{
	FString Name = GetClass()->GetName();
	Name.RemoveFromStart("PCG");
	Name.RemoveFromEnd("Settings");
	return FName(Name);
}

FText UVoxelPCGSettings::GetDefaultNodeTitle() const
{
	return GetClass()->GetDisplayNameText();
}

FText UVoxelPCGSettings::GetNodeTooltipText() const
{
	return GetClass()->GetToolTipText();
}
#endif

bool UVoxelPCGSettings::CanCullTaskIfUnwired() const
{
	return false;
}

FPCGElementPtr UVoxelPCGSettings::CreateElement() const
{
	return MakeShared<FVoxelPCGElement>();
}

void UVoxelPCGSettings::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

FString UVoxelPCGSettings::GetNodeDebugInfo() const
{
#if WITH_EDITOR
	return GetDefaultNodeTitle().ToString();
#else
	return GetClass()->GetName();
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPCGContext::~FVoxelPCGContext()
{
	if (!PrivateOutput)
	{
		return;
	}

	PrivateOutput->PrivateContext->CancelTasks();

	ensure(PrivateOutput->PrivateOwner == this);
	PrivateOutput->PrivateOwner = nullptr;
}

TSharedPtr<FVoxelPCGOutput> FVoxelPCGContext::GetOutput() const
{
	return PrivateOutput;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FPCGContext* FVoxelPCGElement::CreateContext()
{
	return new FVoxelPCGContext();
}

#if VOXEL_ENGINE_VERSION < 506
void FVoxelPCGElement::GetDependenciesCrc(
	const FPCGDataCollection& InInput,
	const UPCGSettings* InSettings,
	UPCGComponent* InComponent,
	FPCGCrc& OutCrc) const
{
	IPCGElement::GetDependenciesCrc(InInput, InSettings, InComponent, OutCrc);

	FString NodeInfo = InSettings->GetName();
	if (const UVoxelPCGSettings* VoxelSettings = Cast<UVoxelPCGSettings>(InSettings))
	{
		NodeInfo = VoxelSettings->GetNodeDebugInfo();
	}

	const uint32 CRC = GVoxelPCGTracker->GetCRC(
		*InComponent,
		InSettings->GetStableUID(),
		FName(NodeInfo));

	OutCrc.Combine(CRC);
}
#else
void FVoxelPCGElement::GetDependenciesCrc(
	const FPCGGetDependenciesCrcParams& InParams,
	FPCGCrc& OutCrc) const
{
	IPCGElement::GetDependenciesCrc(InParams, OutCrc);

	UPCGComponent* Component = CastEnsured<UPCGComponent>(InParams.ExecutionSource);
	if (!ensure(Component))
	{
		return;
	}

	FString NodeInfo = InParams.Settings->GetName();
	if (const UVoxelPCGSettings* VoxelSettings = Cast<UVoxelPCGSettings>(InParams.Settings))
	{
		NodeInfo = VoxelSettings->GetNodeDebugInfo();
	}

	const uint32 CRC = GVoxelPCGTracker->GetCRC(
		*Component,
		InParams.Settings->GetStableUID(),
		FName(NodeInfo));

	OutCrc.Combine(CRC);
}
#endif

bool FVoxelPCGElement::PrepareDataInternal(FPCGContext* Context) const
{
	VOXEL_FUNCTION_COUNTER();

	const UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensureVoxelSlow(Component))
	{
		return true;
	}

	const TSharedRef<FVoxelPCGCallstack> Callstack = MakeShared<FVoxelPCGCallstack>(*Context);

	GVoxelPCGCallstacks.Add(Callstack);
	ON_SCOPE_EXIT
	{
		ensure(GVoxelPCGCallstacks.Pop() == Callstack);
	};

	const TSharedRef<FVoxelInvalidationQueue> DependencySnapshot = FVoxelInvalidationQueue::Create();

	const TSharedPtr<FVoxelPCGOutput> Output = Context->GetInputSettings<UVoxelPCGSettings>()->CreateOutput(*Context);
	if (!Output)
	{
		return true;
	}

	ensure(!Output->PrivateOwner);
	Output->PrivateOwner = Context;
	Output->PrivateDependencyCollector = MakeShared<FVoxelDependencyCollector>(Component->GetFName());
	Output->PrivateInvalidationQueue = DependencySnapshot;
	Output->PrivateSeed = Context->GetSeed();
	Output->PrivateContext = FVoxelTaskContext::Create(STATIC_FNAME("FVoxelPCGElement"));
	Output->PrivateContext->LambdaWrapper = [PCGComponent = MakeVoxelObjectPtr(Component), Callstack = MakeShared<FVoxelPCGCallstack>(*Context)](TVoxelUniqueFunction<void()> Lambda)
	{
		return [Lambda = MoveTemp(Lambda), Callstack]
		{
			GVoxelPCGCallstacks.Add(Callstack);
			{
				Lambda();
			}
			ensure(GVoxelPCGCallstacks.Pop() == Callstack);
		};
	};

	FVoxelPCGContext& TypedContext = *static_cast<FVoxelPCGContext*>(Context);
	ensure(!TypedContext.PrivateOutput);
	TypedContext.PrivateOutput = Output;

	return true;
}

bool FVoxelPCGElement::ExecuteInternal(FPCGContext* Context) const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const FVoxelPCGContext& TypedContext = *static_cast<FVoxelPCGContext*>(Context);
	if (!TypedContext.GetOutput())
	{
		return true;
	}
	FVoxelPCGOutput& Output = *TypedContext.GetOutput();

	UPCGComponent* Component = GetPCGComponent(*Context);
	if (!ensureVoxelSlow(Component))
	{
		return true;
	}

	if (!ensure(Output.PrivateContext) ||
		!ensure(!Output.PrivateContext->IsCancellingTasks()))
	{
		return true;
	}

	if (Output.PrivateFuture.IsSet())
	{
		// Flush any game task queued by FuturePoints
		Voxel::FlushGameTasks();

		return Output.PrivateFuture->IsComplete();
	}

	check(Output.PrivateOwner);

	ensure(!Context->bIsPaused);
	Context->bIsPaused = true;

	const UVoxelPCGSettings& Settings = *Context->GetInputSettings<UVoxelPCGSettings>();
	const uint64 SettingsId = Settings.GetStableUID();

#if VOXEL_INVALIDATION_TRACKING
	for (const TSharedRef<const FVoxelInvalidationCallstack>& Callstack : GVoxelPCGTracker->GetCallstacks(*Component, SettingsId))
	{
		Callstack->ForeachFrame<FVoxelInvalidationFrame_PCG>([&](
			const FVoxelInvalidationFrame_PCG& Frame,
			const TConstVoxelArrayView<const FVoxelInvalidationFrame*> Parents)
		{
			if (Frame.Component != Component ||
				Frame.SettingsId != SettingsId)
			{
				return;
			}

			const UPCGNode* OtherNode = nullptr;
			if (Parents.Num() > 0)
			{
				if (const FVoxelInvalidationFrame_PCG* OtherFrame = CastStruct<FVoxelInvalidationFrame_PCG>(Parents.Last()))
				{
					if (const UPCGSettings* OtherSettings = OtherFrame->Settings.Resolve())
					{
						OtherNode = OtherSettings->GetTypedOuter<UPCGNode>();
					}
				}
			}

			LOG_VOXEL(Error,
				"PCG graph is invalidating itself\n"
				"Node: %s\n"
				"Invalidation callstack: %s",
				*MakeVoxelObjectPtr(Context->Node).GetPathName(),
				*Callstack->ToString().Replace(TEXT("\n"), TEXT("\n\t")));

			VOXEL_MESSAGE(Error,
				"{0}: PCG graph is invalidating itself: {1} -> {2}. Check log for more info",
				Context->Node ? Context->Node->GetTypedOuter<UPCGGraph>() : nullptr,
				OtherNode,
				Context->Node);
		});
	}
#endif

#if WITH_EDITOR
	{
		const FName Name = FName(Settings.GetDefaultNodeTitle().ToString());

		VOXEL_SCOPE_LOCK(GVoxelPCGHelperSingleton->CriticalSection);
		GVoxelPCGHelperSingleton->NameToContexts_RequiresLock.FindOrAdd(Name).Add(Output.PrivateContext);
	}
#endif

	FVoxelTaskScope Scope(*Output.PrivateContext);

	const FName NodeInfo = FName(Settings.GetNodeDebugInfo());

	ensure(!Output.PrivateFuture);
	Output.PrivateFuture = Output.Run().Then_GameThread(MakeStrongPtrLambda(Output, [
		&Output,
		SettingsId,
		NodeInfo,
		bTrackLayerChanges = Settings.bTrackLayerChanges]
	{
		if (!Output.PrivateOwner)
		{
			return;
		}

		ensureVoxelSlow(Output.PrivateOwner->bIsPaused);
		Output.PrivateOwner->bIsPaused = false;

		if (!bTrackLayerChanges)
		{
			return;
		}

		UPCGComponent* LocalComponent = GetPCGComponent(*Output.PrivateOwner);
		if (!ensureVoxelSlow(LocalComponent))
		{
			return;
		}

		GVoxelPCGTracker->RegisterDependencyCollector(
			Output.GetDependencyCollector(),
			*Output.PrivateInvalidationQueue,
			*LocalComponent,
			SettingsId,
			NodeInfo);
	}));

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UPCGComponent* GetPCGComponent(const FPCGContext& Context)
{
#if VOXEL_ENGINE_VERSION < 506
	return Context.SourceComponent.Get();
#else
	return CastEnsured<UPCGComponent>(Context.ExecutionSource.GetObject());
#endif
}