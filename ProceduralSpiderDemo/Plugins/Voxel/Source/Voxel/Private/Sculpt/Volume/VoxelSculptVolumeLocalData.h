// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/VoxelSculptVolumeDataSource.h"

class UVoxelSculptVolumeAsset;
class IVoxelVolumeModifierRuntime;

class FVoxelSculptVolumeLocalData
	: public IVoxelSculptVolumeDataSource
	, public TSharedFromThis<FVoxelSculptVolumeLocalData>
{
public:
	explicit FVoxelSculptVolumeLocalData(AVoxelSculptVolume& SculptActor);
	virtual ~FVoxelSculptVolumeLocalData() override;

	//~ Begin IVoxelSculptVolumeDataSource Interface
	virtual TSharedRef<IVoxelBulkLoader> GetBulkLoader() const override;

	virtual void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader) override;

	virtual FVoxelFuture ApplyModifier(const TSharedRef<FVoxelVolumeModifier>& Modifier) override;
	virtual TSharedPtr<FVoxelSculptVolumeLocalData> AsLocalData() override;
	//~ End IVoxelSculptVolumeDataSource Interface

public:
	void SetAsset(UVoxelSculptVolumeAsset* Asset);
	TSharedRef<IVoxelSculptVolumeDataSource> Duplicate(AVoxelSculptVolume& SculptActor) const;

private:
	TSharedRef<IVoxelBulkLoader> BulkLoader;

	FSharedVoidPtr OnAssetDataChangedPtr;
	TVoxelObjectPtr<UVoxelSculptVolumeAsset> WeakAsset;

	struct FPendingModifier
	{
		FVoxelPromise Promise;
		TVoxelFuture<const FVoxelSculptVolumeData> Future;
	};
	TVoxelOptional<FPendingModifier> PendingModifier;

	struct FQueuedModifier
	{
		FVoxelPromise Promise;
		TSharedPtr<IVoxelVolumeModifierRuntime> ModifierRuntime;
	};
	TVoxelArray<FQueuedModifier> QueuedModifiers;

	void ClearQueue();
	void ProcessQueue();
	bool ProcessQueue_ShouldContinue();
};