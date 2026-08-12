// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampManager.h"
#include "VoxelStampRef.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"
#include "VoxelInvalidationQueue.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	Voxel::OnDependencyFlush.AddLambda([]
	{
		for (const TSharedRef<FVoxelStampManager>& StampManager : FVoxelStampManager::GetAll())
		{
			StampManager->FlushUpdates();
		}
	});
}

const TVoxelChunkedSparseArray<TSharedRef<const FVoxelStampRuntime>>& FVoxelStampLayerManager::GetStamps() const
{
	check(IsInGameThread());
	return Stamps;
}

FVoxelStampLayerManager::FVoxelStampLayerManager(
	const TVoxelObjectPtr<UWorld> World,
	const TVoxelObjectPtr<UVoxelLayer> Layer)
	: World(World)
	, Layer(Layer)
	, NumStamps(FName("Num Registered Stamps " + World.GetName() + "." + Layer.GetName()))
{
}

void FVoxelStampLayerManager::NotifyStampsChanged()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	ensure(StampChanges.Num() > 0);

	FVoxelInvalidationScope Scope(FVoxelInvalidationCallstack::Create(Layer));

	NumStamps = Stamps.Num();

	const TVoxelChunkedArray<FVoxelStampChange> LocalChangedStamps = MoveTemp(StampChanges);
	OnStampChanged.Broadcast(LocalChangedStamps);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampManager::RegisterStamps(
	const TConstVoxelArrayView<FVoxelStampRef> StampRefs,
	const TVoxelArrayView<FVoxelStampIndex> OutStampIndices,
	USceneComponent& Component)
{
	VOXEL_FUNCTION_COUNTER_NUM(StampRefs.Num());
	check(IsInGameThread());
	check(StampRefs.Num() == OutStampIndices.Num());

	Callstack->AddCaller(FVoxelInvalidationCallstack::Create(Component));

	IndicesPendingUpdate.ReserveGrow(StampRefs.Num());

	for (int32 Index = 0 ; Index < StampRefs.Num(); Index++)
	{
		const FVoxelStampRef& StampRef = StampRefs[Index];
		FVoxelStampIndex& StampIndex = OutStampIndices[Index];
		checkVoxelSlow(!StampIndex.IsValid());

		StampIndex.SerialNumber = SerialCounter++;
		StampIndex.Index = StampInstances.Emplace(FStampInstance
		{
			StampIndex.SerialNumber,
			StampRef
		});
		StampIndex.WeakStampManager = WeakFromThis(this);

		IndicesPendingUpdate.Add(StampIndex.Index);

		QueuedUpdates.Add(FUpdate
		{
			EUpdateType::Register,
			StampIndex,
			StampRef,
			StampRef->MakeSharedCopy(),
			Component
		});

		LOG_VOXEL(Verbose, "%s: Registering stamp %d Class: %s Component: %s LOD Range: %d-%d Transform: %s",
			*GetWorld().GetName(),
			StampIndex.SerialNumber,
			*StampRef.GetStruct()->GetName(),
			*Component.GetReadableName(),
			StampRef->LODRange.Min,
			StampRef->LODRange.Max,
			*StampRef->Transform.ToHumanReadableString());
	}
}

void FVoxelStampManager::UnregisterStamps(const TConstVoxelArrayView<FVoxelStampIndex> StampIndices)
{
	check(IsInGameThread());
	check(StampIndices.Num() > 0);

	IndicesPendingUpdate.ReserveGrow(StampIndices.Num());

	for (const FVoxelStampIndex& StampIndex : StampIndices)
	{
		checkVoxelSlow(StampIndex.SerialNumber == StampInstances[StampIndex.Index].SerialNumber)
		checkVoxelSlow(StampIndex.WeakStampManager == WeakFromThis(this));

#if VOXEL_INVALIDATION_TRACKING
		const TSharedPtr<const FVoxelInvalidationCallstack> ThreadCallstack = FVoxelInvalidationScope::GetThreadCallstack();
		if (ensureVoxelSlow(ThreadCallstack))
		{
			Callstack->AddCaller(ThreadCallstack.ToSharedRef());
		}
#endif

		IndicesPendingUpdate.Add(StampIndex.Index);

		QueuedUpdates.Add(FUpdate
		{
			EUpdateType::Unregister,
			StampIndex
		});

		LOG_VOXEL(Verbose, "%s: Unregistering stamp %d",
			*GetWorld().GetName(),
			StampIndex.SerialNumber);
	}
}

void FVoxelStampManager::UpdateStamp(
	const FVoxelStampIndex& StampIndex,
	const FVoxelStampRef& StampRef,
	USceneComponent& Component)
{
	check(IsInGameThread());
	check(StampIndex.SerialNumber == StampInstances[StampIndex.Index].SerialNumber)
	check(StampIndex.WeakStampManager == WeakFromThis(this));

#if VOXEL_INVALIDATION_TRACKING
	const TSharedPtr<const FVoxelInvalidationCallstack> ThreadCallstack = FVoxelInvalidationScope::GetThreadCallstack();
	if (ensureVoxelSlow(ThreadCallstack))
	{
		Callstack->AddCaller(ThreadCallstack.ToSharedRef());
	}
#endif

	IndicesPendingUpdate.Add(StampIndex.Index);

	QueuedUpdates.Add(FUpdate
	{
		EUpdateType::Update,
		StampIndex,
		StampRef,
		StampRef->MakeSharedCopy(),
		Component
	});

	LOG_VOXEL(Verbose, "%s: Updating stamp %d Class: %s Component: %s LOD Range: %d-%d Transform: %s",
		*GetWorld().GetName(),
		StampIndex.SerialNumber,
		*StampRef.GetStruct()->GetName(),
		*Component.GetReadableName(),
		StampRef->LODRange.Min,
		StampRef->LODRange.Max,
		*StampRef->Transform.ToHumanReadableString());
}

TSharedPtr<const FVoxelStampRuntime> FVoxelStampManager::ResolveStampRuntime(const FVoxelStampIndex& StampIndex)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (IndicesPendingUpdate.Contains(StampIndex.Index))
	{
		FlushUpdates();
	}
	if (!ensure(!IndicesPendingUpdate.Contains(StampIndex.Index)))
	{
		// Fail otherwise stack overflow
		return {};
	}

	if (!ensure(StampInstances.IsValidIndex(StampIndex.Index)))
	{
		return {};
	}

	FStampInstance& StampInstance = StampInstances[StampIndex.Index];
	if (!ensure(StampInstance.SerialNumber == StampIndex.SerialNumber))
	{
		return {};
	}

	return StampInstance.Runtime;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampManager::FlushUpdates()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (QueuedUpdates.Num() == 0)
	{
		return;
	}

	const TSharedRef<FVoxelInvalidationCallstack> LocalCallstack = Callstack;
	Callstack = FVoxelInvalidationCallstack::Create("Stamp Manager");
	FVoxelInvalidationScope Scope(LocalCallstack);

	TVoxelMap<int32, FUpdate> IndexToUpdate;
	IndexToUpdate.Reserve(QueuedUpdates.Num());

	for (const FUpdate& Update : QueuedUpdates)
	{
		FStampInstance& StampInstance = StampInstances[Update.StampIndex.Index];
		checkVoxelSlow(StampInstance.SerialNumber == Update.StampIndex.SerialNumber);

		switch (Update.Type)
		{
		default: VOXEL_ASSUME(false);
		case EUpdateType::Register:
		{
			checkVoxelSlow(!StampInstance.Runtime);
			checkVoxelSlow(StampInstance.LayerRefs.Num() == 0);

			IndexToUpdate.FindOrAdd(Update.StampIndex.Index) = Update;
		}
		break;
		case EUpdateType::Unregister:
		{
			for (const FStampLayerRef& LayerRef : StampInstance.LayerRefs)
			{
				checkVoxelSlow(LayerToLayerManager[LayerRef.LayerManager->Layer].Get() == LayerRef.LayerManager);

				FVoxelStampLayerManager& LayerManager = *LayerRef.LayerManager;

				const TSharedPtr<const FVoxelStampRuntime> OldStamp = LayerManager.Stamps.RemoveAt_ReturnValue(LayerRef.IndexInLayer);

				LayerManager.StampChanges.Add(FVoxelStampChange
				{
					OldStamp,
					nullptr
				});
			}

			IndexToUpdate.Remove(Update.StampIndex.Index);
			StampInstances.RemoveAt(Update.StampIndex.Index);
		}
		break;
		case EUpdateType::Update:
		{
			checkVoxelSlow(StampInstance.Runtime);

			IndexToUpdate.FindOrAdd(Update.StampIndex.Index) = Update;
		}
		break;
		}
	}

	IndicesPendingUpdate.Reset();
	QueuedUpdates.Reset();

	TVoxelArray<FVoxelStampRuntime::FStampInfo> StampInfos;
	{
		VOXEL_SCOPE_COUNTER("Create runtimes");

		StampInfos.Reserve(IndexToUpdate.Num());

		for (const auto& It : IndexToUpdate)
		{
			StampInfos.Add_EnsureNoGrow(FVoxelStampRuntime::FStampInfo
			{
				It.Value.StampIndex,
				It.Value.WeakStampRef,
				It.Value.NewStamp.ToSharedRef(),
				It.Value.Component
			});
		}

		FVoxelStampRuntime::BulkCreate(
			GetWorld(),
			StampInfos);
	}

	int32 StampIndex = 0;
	for (const auto& It : IndexToUpdate)
	{
		const FUpdate& Update = It.Value;
		checkVoxelSlow(Update.Type != EUpdateType::Unregister);
		checkVoxelSlow(Update.StampIndex.WeakStampManager == WeakFromThis(this));

		FStampInstance& StampInstance = StampInstances[Update.StampIndex.Index];
		checkVoxelSlow(StampInstance.SerialNumber == Update.StampIndex.SerialNumber);

		const TSharedRef<FVoxelStampRuntime> NewStamp = StampInfos[StampIndex++].GetRuntime();
		checkVoxelSlow(NewStamp->StampIndex == Update.StampIndex);
		checkVoxelSlow(NewStamp->WeakStampRef == Update.WeakStampRef);

		const TSharedPtr<FVoxelStampRuntime> OldStamp = StampInstance.Runtime;

		StampInstance.WeakStampRef = Update.WeakStampRef;
		StampInstance.Runtime = NewStamp;

		TConstVoxelArrayView<TVoxelObjectPtr<UVoxelLayer>> Layers;
		if (!NewStamp->FailedToInitialize())
		{
			Layers = NewStamp->GetLayers();
		}

		TVoxelInlineArray<FStampLayerRef, 2> OldLayerRefs = MoveTemp(StampInstance.LayerRefs);

		StampInstance.LayerRefs.Reset();
		StampInstance.LayerRefs.Reserve(Layers.Num());

		if (Update.Type == EUpdateType::Register)
		{
			checkVoxelSlow(!OldStamp);
			checkVoxelSlow(OldLayerRefs.Num() == 0);
		}

		int32 NumLayersAffected = 0;
		for (const TVoxelObjectPtr<UVoxelLayer>& Layer : Layers)
		{
			if (Layer.IsExplicitlyNull())
			{
				continue;
			}

			NumLayersAffected++;

			FVoxelStampLayerManager& LayerManager = INLINE_LAMBDA -> FVoxelStampLayerManager&
			{
				if (const TSharedPtr<FVoxelStampLayerManager>* LayerManagerPtr = LayerToLayerManager.Find(Layer))
				{
					return *LayerManagerPtr->Get();
				}
				else
				{
					return FindOrAddLayer(Layer).Get();
				}
			};

			// Look for the same layer in the previous stamp if we have one
			bool bHasOldStamp = false;
			for (int32 Index = 0; Index < OldLayerRefs.Num(); Index++)
			{
				const FStampLayerRef OldLayerRef = OldLayerRefs[Index];
				if (OldLayerRef.LayerManager != &LayerManager)
				{
					continue;
				}

				OldLayerRefs.RemoveAtSwap(Index);

				ensureVoxelSlow(OldStamp == LayerManager.Stamps.RemoveAt_ReturnValue(OldLayerRef.IndexInLayer));

				checkVoxelSlow(!bHasOldStamp);
				bHasOldStamp = true;
			}

			const int32 IndexInLayer = LayerManager.Stamps.Add(NewStamp);

			StampInstance.LayerRefs.Add_EnsureNoGrow(FStampLayerRef
			{
				&LayerManager,
				IndexInLayer
			});

			checkVoxelSlow(!bHasOldStamp || !OldStamp->FailedToInitialize());
			checkVoxelSlow(!NewStamp->FailedToInitialize());

			LayerManager.StampChanges.Add(FVoxelStampChange
			{
				bHasOldStamp ? OldStamp : nullptr,
				NewStamp
			});
		}

#if WITH_EDITOR
		if (NumLayersAffected == 0 &&
			!StampInstance.Runtime->FailedToInitialize())
		{
			const int32 InstanceIndex = StampInstance.Runtime->GetInstanceIndex();
			if (InstanceIndex != -1)
			{
				VOXEL_MESSAGE(Error, "{0}: Instanced Stamp[{1}] does not have a valid layer", StampInstance.Runtime->GetActor(), InstanceIndex);
			}
			else
			{
				VOXEL_MESSAGE(Error, "{0}: Stamp does not have a valid layer", StampInstance.Runtime->GetActor());
			}
		}
#endif

		// OldLayerRefs won't be empty if stamp layers changed
		for (const FStampLayerRef& LayerRef : OldLayerRefs)
		{
			checkVoxelSlow(LayerToLayerManager[LayerRef.LayerManager->Layer].Get() == LayerRef.LayerManager);

			FVoxelStampLayerManager& LayerManager = *LayerRef.LayerManager;

			ensureVoxelSlow(OldStamp == LayerManager.Stamps.RemoveAt_ReturnValue(LayerRef.IndexInLayer));

			checkVoxelSlow(!OldStamp->FailedToInitialize());

			LayerManager.StampChanges.Add(FVoxelStampChange
			{
				OldStamp,
				nullptr
			});
		}
	}
	check(StampIndex == StampInfos.Num());

	const TVoxelArray<TSharedPtr<FVoxelStampLayerManager>> LocalLayerManagers = LayerToLayerManager.ValueArray();
	for (const TSharedPtr<FVoxelStampLayerManager>& LayerManager : LocalLayerManagers)
	{
		if (LayerManager->StampChanges.Num() > 0)
		{
			LayerManager->NotifyStampsChanged();
		}
	}

	OnChanged.Broadcast();
}

TSharedRef<FVoxelStampLayerManager> FVoxelStampManager::FindOrAddLayer(const TVoxelObjectPtr<UVoxelLayer> Layer)
{
	checkVoxelSlow(IsInGameThread());
	checkVoxelSlow(!Layer.IsExplicitlyNull());

	TSharedPtr<FVoxelStampLayerManager> LayerManager = LayerToLayerManager.FindRef(Layer);
	if (!LayerManager)
	{
		for (auto It = LayerToLayerManager.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid_Slow())
			{
				It.RemoveCurrent();
			}
		}

		LayerManager = MakeShareable(new FVoxelStampLayerManager(GetWorld(), Layer));
		LayerToLayerManager.Add_CheckNew(Layer, LayerManager);
	}
	return LayerManager.ToSharedRef();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampManager::Tick()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	FlushUpdates();
}