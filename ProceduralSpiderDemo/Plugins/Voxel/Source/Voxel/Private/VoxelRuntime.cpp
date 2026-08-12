// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelRuntime.h"
#include "VoxelWorld.h"
#include "VoxelState.h"
#include "VoxelLayers.h"
#include "Render/VoxelRenderSubsystem.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "PCGComponent.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelShowBoundsToGenerate, false,
	"voxel.render.ShowBoundsToGenerate",
	"Show the generated voxel world bounds");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelDisableEditorPreview, false,
	"voxel.DisableEditorPreview",
	"Disable voxel world preview in editor");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelOnlyRenderInBeginPlay, false,
	"voxel.OnlyRenderInBeginPlay",
	"Use this to check WaitOnBeginPlay is working properly");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRuntime::FVoxelRuntime(AVoxelWorld& VoxelWorld)
	: WeakVoxelWorld(&VoxelWorld)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if VOXEL_ENGINE_VERSION < 506
DEFINE_PRIVATE_ACCESS(UWorld, CleanupWorldTag)
#endif

void FVoxelRuntime::Initialize()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	ensure(!OldState);
	ensure(!NewState);

	// Tick here and not only in FVoxelTicker to ensure that meshes created this frame will be sent this frame to the render thread
	// FVoxelTicker runs at the end of the frame
	// Still tick in FVoxelTicker to avoid being stuck on startup
	FWorldDelegates::OnWorldPreSendAllEndOfFrameUpdates.Add(MakeWeakPtrDelegate(this, [this](UWorld* World)
	{
		const AVoxelWorld* VoxelWorld = WeakVoxelWorld.Resolve();
		if (!ensure(VoxelWorld) ||
			VoxelWorld->GetWorld() != World)
		{
			return;
		}

#if VOXEL_ENGINE_VERSION >= 506
		if (!VoxelWorld->IsActorInitialized() ||
			World->IsCleanedUp())
		{
			return;
		}
#else
		if (!VoxelWorld->IsActorInitialized() ||
			PrivateAccess::CleanupWorldTag(*World) != 0)
		{
			return;
		}
#endif

		Tick();
	}));

	AVoxelWorld* VoxelWorld = WeakVoxelWorld.Resolve();
	if (!ensure(VoxelWorld))
	{
		return;
	}

	const TSharedRef<FVoxelLayers> Layers = FVoxelLayers::Get(VoxelWorld->GetWorld(), *VoxelWorld);
	const TSharedRef<FVoxelConfig> NewConfig = MakeShared_Stats<FVoxelConfig>(*VoxelWorld);

	NewState = MakeShared<FVoxelState>(
		NewConfig,
		Layers,
		nullptr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRuntime::Tick()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	AVoxelWorld* VoxelWorld = WeakVoxelWorld.Resolve();
	if (!ensure(VoxelWorld))
	{
		return;
	}

	UWorld* World = VoxelWorld->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	if (GVoxelDisableEditorPreview &&
		!World->IsGameWorld())
	{
		return;
	}

	if (GVoxelOnlyRenderInBeginPlay &&
		World->HasBegunPlay())
	{
		return;
	}

	DrawDebug();

	if (NewState)
	{
		NewState->Tick();

		const bool bCanRender = INLINE_LAMBDA
		{
			if (!NewState->IsReadyToRender())
			{
				return false;
			}

#if WITH_EDITOR
			if (PCGSystemSwitches::CVarPausePCGExecution.GetValueOnAnyThread())
			{
				return true;
			}
#endif

			if (VoxelWorld->bWaitForPCG)
			{
				for (const UPCGComponent* Component : TObjectRange<UPCGComponent>())
				{
					if (Component->GetWorld() == World &&
						Component->IsGenerating())
					{
						if (GVoxelDumpStatus)
						{
							LOG_VOXEL(Log, "Waiting for PCG component %s", *Component->GetReadableName())
						}
						return false;
					}
				}
			}

			return true;
		};

		if (!bCanRender)
		{
			return;
		}

		NewState->Render(*this);

		OldState = NewState;
		NewState = nullptr;
	}
	ensure(!NewState);

#if WITH_EDITOR
	if (FVoxelUtilities::IsPlayInEditor() &&
		FVoxelUtilities::IsLevelEditorWorld(World))
	{
		// We're in PIE, don't compute any new state on the editor voxel world
		// Try to still render the voxel worlds in preview scenes (eg, heightmap asset)
		return;
	}
#endif

	ON_SCOPE_EXIT
	{
		ensure(!OnNextStateRendered);
	};

	// Create layers now to ensure stamps are flushed
	const TSharedRef<FVoxelLayers> Layers = FVoxelLayers::Get(World, *VoxelWorld);
	const TSharedRef<FVoxelConfig> NewConfig = MakeShared_Stats<FVoxelConfig>(*VoxelWorld);

	const bool bShouldComputeNewState = INLINE_LAMBDA
	{
		if (!OldState ||
			!OldState->Config->Equals(*NewConfig))
		{
			return true;
		}

		if (OldState->IsInvalidated())
		{
			VOXEL_SCOPE_COUNTER("OnInvalidated");
			OnInvalidated.Broadcast();
			return true;
		}

		const FVoxelRenderSubsystem* RenderSubsystem = OldState->GetSubsystemPtr<FVoxelRenderSubsystem>();
		if (!RenderSubsystem)
		{
			ensure(!FApp::CanEverRender() || World->GetNetMode() == NM_DedicatedServer);
			return false;
		}

		return RenderSubsystem->HasChunksToSubdivide();
	};

	if (!bShouldComputeNewState)
	{
		if (const TSharedPtr<FSimpleMulticastDelegate> LocalOnNextStateRendered = MoveTemp(OnNextStateRendered))
		{
			VOXEL_SCOPE_COUNTER("OnNextStateRendered");
			LocalOnNextStateRendered->Broadcast();
		}
		return;
	}

	NewState = MakeShared_Stats<FVoxelState>(
		NewConfig,
		Layers,
		OldState);

	if (const TSharedPtr<FSimpleMulticastDelegate> LocalOnNextStateRendered = MoveTemp(OnNextStateRendered))
	{
		NewState->OnRendered.AddLambda([=]
		{
			VOXEL_SCOPE_COUNTER("OnNextStateRendered");
			LocalOnNextStateRendered->Broadcast();
		});
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRuntime::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();

	if (OldState)
	{
		OldState->AddReferencedObjects(Collector);
	}

	if (NewState)
	{
		NewState->AddReferencedObjects(Collector);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USceneComponent* FVoxelRuntime::NewComponent(const UClass* Class)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	AVoxelWorld* VoxelWorld = WeakVoxelWorld.Resolve();
	if (!ensure(VoxelWorld))
	{
		return nullptr;
	}

	ensure(!VoxelWorld->bDisableModify);
	VoxelWorld->bDisableModify = true;
	ON_SCOPE_EXIT
	{
		ensure(VoxelWorld->bDisableModify);
		VoxelWorld->bDisableModify = false;
	};

	USceneComponent* Component = nullptr;

	if (TVoxelChunkedArray<TVoxelObjectPtr<USceneComponent>>* Pool = ClassToPooledComponents.Find(Class))
	{
		while (
			!Component &&
			Pool->Num() > 0)
		{
			Component = Pool->Pop().Resolve();
		}
	}

	if (!Component)
	{
		USceneComponent* RootComponent = VoxelWorld->RootComponent;
		if (!ensure(RootComponent))
		{
			return nullptr;
		}

		Component = NewObject<USceneComponent>(
			RootComponent,
			Class,
			NAME_None,
			RF_Transient |
			RF_DuplicateTransient |
			RF_TextExportTransient);

		if (!ensure(Component))
		{
			return nullptr;
		}

		Component->SetupAttachment(RootComponent);
		Component->RegisterComponent();

		AllComponents.Add(Component);
	}

	checkVoxelSlow(Component->GetRelativeTransform().Equals(FTransform::Identity));
	return Component;
}

void FVoxelRuntime::RemoveComponent(USceneComponent* Component)
{
	if (!ensureVoxelSlow(Component))
	{
		return;
	}

	RemoveComponents(
		Component->GetClass(),
		MakeVoxelArrayView(Component));
}

void FVoxelRuntime::RemoveComponents(
	UClass* Class,
	const TConstVoxelArrayView<USceneComponent*> ComponentsToRemove)
{
	VOXEL_FUNCTION_COUNTER_NUM(ComponentsToRemove.Num());
	check(IsInGameThread());

#if VOXEL_DEBUG
	for (USceneComponent* Component : ComponentsToRemove)
	{
		check(Component);
		check(Component->IsA(Class));
		ensure(AllComponents.Contains(Component));
	}
#endif

	TVoxelChunkedArray<TVoxelObjectPtr<USceneComponent>>& PooledComponents = ClassToPooledComponents.FindOrAdd(Class);

	for (USceneComponent* Component : ComponentsToRemove)
	{
		// Place component back at origin to avoid issues when computing bounds
		Component->SetRelativeTransform(FTransform::Identity);

		PooledComponents.Add(Component);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRuntime::DrawDebug() const
{
	VOXEL_FUNCTION_COUNTER();

	AVoxelWorld* VoxelWorld = WeakVoxelWorld.Resolve();
	if (!ensure(VoxelWorld))
	{
		return;
	}

	UWorld* World = VoxelWorld->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	if (OldState)
	{
		if (GVoxelShowBoundsToGenerate)
		{
			const FVoxelOptionalBox BoundsToGenerate = OldState->GetSubsystem<FVoxelRenderSubsystem>().GetBoundsToGenerate();
			if (BoundsToGenerate.IsValid())
			{
				FVoxelDebugDrawer(World)
				.LifeTime(0.1f)
				.Color(FLinearColor::Red)
				.DrawBox(BoundsToGenerate.GetBox(), OldState->Config->LocalToWorld);
			}
		}
	}

#if WITH_EDITOR
	if (OldState &&
		World->WorldType != EWorldType::EditorPreview)
	{
		TSet<EVoxelMegaMaterialTarget> Targets;
		if (OldState->Config->bEnableNanite)
		{
			Targets.Add(EVoxelMegaMaterialTarget::NaniteWPO);
			Targets.Add(EVoxelMegaMaterialTarget::NaniteDisplacement);
			Targets.Add(EVoxelMegaMaterialTarget::NaniteMaterialSelection);
		}

		if (!OldState->Config->bEnableNanite ||
			OldState->Config->RuntimeVirtualTextures.Num() > 0)
		{
			Targets.Add(EVoxelMegaMaterialTarget::NonNanite);
		}

		if (OldState->Config->bEnableLumen ||
			OldState->Config->bEnableRaytracing)
		{
			Targets.Add(EVoxelMegaMaterialTarget::Lumen);
		}

		OldState->Config->MegaMaterialProxy->LogErrors(
			OldState->Config->FeatureLevel,
			Targets,
			VoxelWorld);
	}
#endif
}