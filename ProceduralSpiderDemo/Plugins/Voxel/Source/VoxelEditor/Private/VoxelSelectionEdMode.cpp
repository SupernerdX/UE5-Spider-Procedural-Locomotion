// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSelectionEdMode.h"
#include "VoxelWorld.h"
#include "VoxelLayers.h"
#include "Collision/VoxelCollisionChannels.h"
#include "EditorModes.h"
#include "EditorModeManager.h"
#include "LevelEditor.h"
#include "LevelEditorViewport.h"
#include "VoxelState.h"
#include "VoxelRuntime.h"
#include "VoxelStampDelta.h"
#include "VoxelStampRuntime.h"
#include "VoxelInstancedStampComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

#if VOXEL_ENGINE_VERSION >= 507
#include "ViewportSelectionUtilities.h"
#else
#include "LevelViewportClickHandlers.h"
#endif

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditor.OnMapChanged().AddLambda([](UWorld* World, const EMapChangeType ChangeType)
	{
		if (ChangeType == EMapChangeType::SaveMap ||
			// Otherwise the project browser crashes
			!FPaths::IsProjectFilePathSet())
		{
			return;
		}

		FVoxelUtilities::DelayedCall([]
		{
			GLevelEditorModeTools().AddDefaultMode(GetDefault<UVoxelSelectionEdMode>()->GetID());
			GLevelEditorModeTools().ActivateDefaultMode();
		});
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelSelectionEdMode::UVoxelSelectionEdMode()
{
	Info = FEditorModeInfo(
		"VoxelSelectionEdMode",
		INVTEXT("VoxelSelectionEdMode"),
		FSlateIcon(),
		false,
		MAX_int32
	);

	SettingsClass = UVoxelSelectionEdModeSettings::StaticClass();
}

bool UVoxelSelectionEdMode::HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	AVoxelWorld* WorldSelectedISMC = nullptr;
	UInstancedStaticMeshComponent* SelectedISMC = nullptr;
	bool bClearISMCSelection = true;

	ON_SCOPE_EXIT
	{
		if (!bClearISMCSelection)
		{
			return;
		}

		AVoxelWorld* World = WeakWorld.Resolve();
		UInstancedStaticMeshComponent* ISMC = WeakISMC.Resolve();

		WeakWorld = WorldSelectedISMC;
		WeakISMC = SelectedISMC;

		if (!World ||
			!ISMC)
		{
			return;
		}

		const FScopedTransaction Transaction(INVTEXT("Clearing Instanced Static Mesh Component selection"));

		World->RemoveInstanceComponent(ISMC);
		ISMC->CreationMethod = EComponentCreationMethod::Native;

		FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditor.BroadcastComponentsEdited();
	};

	if (Click.GetKey() != EKeys::LeftMouseButton)
	{
		return false;
	}

	if (Click.GetEvent() != IE_Released &&
		Click.GetEvent() != IE_DoubleClick)
	{
		return false;
	}

	ON_SCOPE_EXIT
	{
		LastViewLocation = ViewportClient->GetViewLocation();
		LastViewRotation = ViewportClient->GetViewRotation();
	};

	if (HInstancedStaticMeshInstance* ISMProxy = HitProxyCast<HInstancedStaticMeshInstance>(HitProxy))
	{
		if (!ISMProxy->Component)
		{
			return false;
		}

		AVoxelWorld* VoxelWorld = ISMProxy->Component->GetOwner<AVoxelWorld>();
		if (!VoxelWorld)
		{
			return false;
		}

		if (ISMProxy->Component == WeakISMC)
		{
			bClearISMCSelection = false;
			return false;
		}

		if (ISMProxy->Component->CreationMethod == EComponentCreationMethod::Native)
		{
			if (Click.GetEvent() == IE_DoubleClick ||
				WeakISMC.IsValid_Slow())
			{
				WorldSelectedISMC = VoxelWorld;
				SelectedISMC = ISMProxy->Component;

				const FScopedTransaction Transaction(INVTEXT("Selecting Instanced Static Mesh Component"));

				VoxelWorld->AddInstanceComponent(SelectedISMC);

				FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
				LevelEditor.BroadcastComponentsEdited();

				GEditor->GetSelectedComponents()->Modify();
				GEditor->GetSelectedActors()->Modify();

				GEditor->GetSelectedActors()->DeselectAll();
				GEditor->GetSelectedComponents()->DeselectAll();

				GEditor->SelectComponent(SelectedISMC, true, true, true);
				return true;
			}
		}

		return false;
	}

	const HActor* ActorProxy = HitProxyCast<HActor>(HitProxy);
	if (!ActorProxy ||
		!ActorProxy->Actor)
	{
		return false;
	}

	if (!ActorProxy->Actor->IsA<AVoxelWorld>())
	{
		return false;
	}

	if (ActorProxy->PrimComponent &&
		ActorProxy->PrimComponent->IsA<UInstancedStaticMeshComponent>())
	{
		return false;
	}

	const AVoxelWorld* VoxelWorld = CastChecked<AVoxelWorld>(ActorProxy->Actor);
	if (Click.GetEvent() == IE_DoubleClick)
	{
#if VOXEL_ENGINE_VERSION >= 507
		UE::Editor::ViewportSelectionUtilities::ClickActor(
			ViewportClient,
			ConstCast(VoxelWorld),
			Click,
			true);
#else
		LevelViewportClickHandlers::ClickActor(
			static_cast<FLevelEditorViewportClient*>(ViewportClient),
			ConstCast(VoxelWorld),
			Click,
			true);
#endif

		return true;
	}

	const TSharedPtr<FVoxelRuntime> Runtime = VoxelWorld->GetRuntime();
	if (!Runtime)
	{
		return false;
	}

	const TSharedPtr<FVoxelState> State = Runtime->GetState();
	if (!State)
	{
		return false;
	}

	FVector Start;
	FVector End;
	if (!ensure(FVoxelEditorUtilities::GetRayInfo(ViewportClient, Start, End)))
	{
		return false;
	}

	FHitResult HitResult;
	{
		FCollisionQueryParams QueryParams;
		while (true)
		{
			if (!GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_VoxelEditor, QueryParams))
			{
				return false;
			}

			AActor* Actor = HitResult.GetActor();
			if (!Actor)
			{
				return false;
			}

			if (Actor == VoxelWorld)
			{
				break;
			}

			QueryParams.AddIgnoredActor(Actor);
		}
	}

	const FVector LocalPosition = VoxelWorld->ActorToWorld().InverseTransformPosition(HitResult.Location);

	TVoxelArray<FVoxelStampDelta> StampDeltas = State->Layers->GetStampDeltas(
		State->Config->LayerToRender,
		LocalPosition,
		0);

	if (StampDeltas.Num() == 0)
	{
		return false;
	}

	Algo::Reverse(StampDeltas);

	{
		TVoxelSet<TSharedPtr<const FVoxelStampRuntime>> CurrentStampDeltas;
		StampDeltas.RemoveAll([&](const FVoxelStampDelta& StampDelta)
		{
			if (StampDelta.Stamp->GetStamp().bDisableStampSelection)
			{
				return true;
			}

			CurrentStampDeltas.Add(StampDelta.Stamp);
			return false;
		});

		bool bAllIgnoredStampsExist = true;
		for (const TWeakPtr<const FVoxelStampRuntime>& Stamp : IgnoredStamps)
		{
			if (!CurrentStampDeltas.Contains(Stamp.Pin()))
			{
				bAllIgnoredStampsExist = false;
				break;
			}
		}

		if (!bAllIgnoredStampsExist ||
			LastViewLocation != ViewportClient->GetViewLocation() ||
			LastViewRotation != ViewportClient->GetViewRotation())
		{
			IgnoredStamps = {};
		}
	}

	const FVoxelStampDelta StampDeltaToSelect = INLINE_LAMBDA -> FVoxelStampDelta
	{
		if (IgnoredStamps.Num() > 0)
		{
			for (const FVoxelStampDelta& StampDelta : StampDeltas)
			{
				if (IgnoredStamps.Contains(StampDelta.Stamp))
				{
					continue;
				}

				return StampDelta;
			}
		}

		const bool bHasAnyVolumeStamp = INLINE_LAMBDA
		{
			for (const FVoxelStampDelta& StampDelta : StampDeltas)
			{
				if (StampDelta.Layer.Type == EVoxelLayerType::Volume &&
					StampDelta.GetDistanceDeltaAbs() > 100.f)
				{
					return true;
				}
			}

			return false;
		};

		if (!bHasAnyVolumeStamp)
		{
			// If we don't have any big delta coming from a volume stamp, pick the last height stamp that did a significant change
			for (const FVoxelStampDelta& StampDelta : StampDeltas)
			{
				if (StampDelta.Layer.Type == EVoxelLayerType::Height &&
					StampDelta.GetDistanceDeltaAbs() > 100.f)
				{
					return StampDelta;
				}
			}
		}

		// Pick the volume stamp with the highest delta
		{
			const FVoxelStampDelta* BestDelta = nullptr;

			// Only check stamps that actually changed the distance from positive to negative (or the other way around)
			for (const FVoxelStampDelta& StampDelta : StampDeltas)
			{
				if (StampDelta.Layer.Type != EVoxelLayerType::Volume)
				{
					break;
				}

				if (!StampDelta.HasDifferentSign())
				{
					continue;
				}

				if (!BestDelta ||
					BestDelta->GetDistanceDeltaAbs() < StampDelta.GetDistanceDeltaAbs())
				{
					BestDelta = &StampDelta;
				}
			}

			if (BestDelta)
			{
				return *BestDelta;
			}

			// Check all stamps
			for (const FVoxelStampDelta& StampDelta : StampDeltas)
			{
				if (StampDelta.Layer.Type != EVoxelLayerType::Volume)
				{
					break;
				}

				if (!BestDelta ||
					BestDelta->GetDistanceDeltaAbs() < StampDelta.GetDistanceDeltaAbs())
				{
					BestDelta = &StampDelta;
				}
			}

			if (BestDelta)
			{
				return *BestDelta;
			}
		}

		// Fallback to the highest priority stamp
		return StampDeltas[0];
	};

	USceneComponent* Component = ConstCast(StampDeltaToSelect.Stamp->GetComponent().Resolve());
	if (!ensureVoxelSlow(Component))
	{
		return false;
	}

	IgnoredStamps.Add(StampDeltaToSelect.Stamp);

	AActor* Actor = Component->GetOwner();

	bool bSelectActor =
		Actor &&
		Actor->GetRootComponent() == Component;

	if (Actor &&
		!Actor->SupportsSubRootSelection())
	{
		if (AActor* RootSelection = Actor->GetRootSelectionParent())
		{
			if (Actor != RootSelection)
			{
				bSelectActor = true;
				Actor = RootSelection;

				// Do not modify instanced stamp component
				Component = nullptr;
			}
		}
	}

	const FScopedTransaction Transaction(INVTEXT("Clicking on Components"));
	GEditor->GetSelectedComponents()->Modify();
	GEditor->GetSelectedActors()->Modify();

	if (UVoxelInstancedStampComponent* StampComponent = Cast<UVoxelInstancedStampComponent>(Component))
	{
		if (const FVoxelStampRef StampRef = StampDeltaToSelect.Stamp->GetWeakStampRef().Pin())
		{
			StampComponent->Modify();
			StampComponent->SetActiveInstance(StampRef);
		}
	}

	if (Click.IsControlDown())
	{
		if (bSelectActor)
		{
			const bool bSelect = !Actor->IsSelected();
			GEditor->SelectActor(Actor, bSelect, true, true);
		}
		else
		{
			const bool bSelect = !Component->IsSelected();
			GEditor->SelectComponent(Component, bSelect, true, true);
		}
	}
	else if (Click.IsShiftDown())
	{
		if (bSelectActor)
		{
			if (!Actor->IsSelected())
			{
				GEditor->SelectActor(Actor, true, true, true);
			}
		}
		else
		{
			if (!Component->IsSelected())
			{
				GEditor->SelectComponent(Component, true, true, true);
			}
		}
	}
	else
	{
		GEditor->GetSelectedActors()->DeselectAll();
		GEditor->GetSelectedComponents()->DeselectAll();
		if (bSelectActor)
		{
			GEditor->SelectActor(Actor, true, true, true);
		}
		else
		{
			GEditor->SelectComponent(Component, true, true, true);
		}
	}

	return true;
}