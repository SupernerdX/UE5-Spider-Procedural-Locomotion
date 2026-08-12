// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampChange.h"
#include "VoxelStampRuntime.h"
#include "VoxelInvalidationCallstack.h"

class VOXEL_API FVoxelStampLayerManager : public TSharedFromThis<FVoxelStampLayerManager>
{
public:
	const TVoxelObjectPtr<UWorld> World;
	const TVoxelObjectPtr<UVoxelLayer> Layer;

	TMulticastDelegate<void(const TVoxelChunkedArray<FVoxelStampChange>& StampChanges)> OnStampChanged;

	const TVoxelChunkedSparseArray<TSharedRef<const FVoxelStampRuntime>>& GetStamps() const;

private:
	TVoxelChunkedSparseArray<TSharedRef<const FVoxelStampRuntime>> Stamps;
	TVoxelChunkedArray<FVoxelStampChange> StampChanges;
	VOXEL_CUSTOM_COUNTER_HELPER(NumStamps);

	FVoxelStampLayerManager(
		TVoxelObjectPtr<UWorld> World,
		TVoxelObjectPtr<UVoxelLayer> Layer);

	UE_NONCOPYABLE(FVoxelStampLayerManager);

	void NotifyStampsChanged();

	friend class FVoxelStampManager;
};

class VOXEL_API FVoxelStampManager : public IVoxelWorldSubsystem
{
public:
	GENERATED_VOXEL_WORLD_SUBSYSTEM_BODY(FVoxelStampManager);

public:
	void RegisterStamps(
		TConstVoxelArrayView<FVoxelStampRef> StampRefs,
		TVoxelArrayView<FVoxelStampIndex> OutStampIndices,
		USceneComponent& Component);

	void UnregisterStamps(TConstVoxelArrayView<FVoxelStampIndex> StampIndices);

	void UpdateStamp(
		const FVoxelStampIndex& StampIndex,
		const FVoxelStampRef& StampRef,
		USceneComponent& Component);

	TSharedPtr<const FVoxelStampRuntime> ResolveStampRuntime(const FVoxelStampIndex& StampIndex);

public:
	FSimpleMulticastDelegate OnChanged;

	void FlushUpdates();
	TSharedRef<FVoxelStampLayerManager> FindOrAddLayer(TVoxelObjectPtr<UVoxelLayer> Layer);

public:
	//~ Begin IVoxelWorldSubsystem Interface
	virtual void Tick() override;
	//~ End IVoxelWorldSubsystem Interface

private:
	int32 SerialCounter = 1000;
	TSharedRef<FVoxelInvalidationCallstack> Callstack = FVoxelInvalidationCallstack::Create("Stamp Manager");

	enum class EUpdateType : uint8
	{
		Register,
		Unregister,
		Update
	};
	struct FUpdate
	{
		EUpdateType Type = {};
		FVoxelStampIndex StampIndex;
		FVoxelWeakStampRef WeakStampRef;
		TSharedPtr<FVoxelStamp> NewStamp;
		TVoxelObjectPtr<USceneComponent> Component;
	};

	TVoxelSet<int32> IndicesPendingUpdate;
	TVoxelChunkedArray<FUpdate> QueuedUpdates;

	struct FStampLayerRef
	{
		FVoxelStampLayerManager* LayerManager;
		int32 IndexInLayer = 0;
	};
	struct FStampInstance
	{
		int32 SerialNumber = 0;
		FVoxelWeakStampRef WeakStampRef;
		TSharedPtr<FVoxelStampRuntime> Runtime;
		TVoxelInlineArray<FStampLayerRef, 2> LayerRefs;
	};
	TVoxelChunkedSparseArray<FStampInstance> StampInstances;

	TVoxelMap<TVoxelObjectPtr<const UVoxelLayer>, TSharedPtr<FVoxelStampLayerManager>> LayerToLayerManager;
};