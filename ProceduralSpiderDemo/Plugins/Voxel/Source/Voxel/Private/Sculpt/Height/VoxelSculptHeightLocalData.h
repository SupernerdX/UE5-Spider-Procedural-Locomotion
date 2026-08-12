// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Height/VoxelSculptHeightDataSource.h"

class UVoxelSculptHeightAsset;
class IVoxelHeightModifierRuntime;

class FVoxelSculptHeightLocalData
	: public IVoxelSculptHeightDataSource
	, public TSharedFromThis<FVoxelSculptHeightLocalData>
{
public:
	explicit FVoxelSculptHeightLocalData(AVoxelSculptHeight& SculptActor);
	virtual ~FVoxelSculptHeightLocalData() override;

	//~ Begin IVoxelSculptHeightDataSource Interface
	virtual TSharedRef<IVoxelBulkLoader> GetBulkLoader() const override;

	virtual void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptHeightData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader) override;

	virtual FVoxelFuture ApplyModifier(const TSharedRef<FVoxelHeightModifier>& Modifier) override;
	virtual TSharedPtr<FVoxelSculptHeightLocalData> AsLocalData() override;
	//~ End IVoxelSculptHeightDataSource Interface

public:
	void SetAsset(UVoxelSculptHeightAsset* Asset);
	TSharedRef<IVoxelSculptHeightDataSource> Duplicate(AVoxelSculptHeight& SculptActor) const;

private:
	TSharedRef<IVoxelBulkLoader> BulkLoader;

	FSharedVoidPtr OnAssetDataChangedPtr;
	TVoxelObjectPtr<UVoxelSculptHeightAsset> WeakAsset;

	struct FPendingModifier
	{
		FVoxelPromise Promise;
		TVoxelFuture<const FVoxelSculptHeightData> Future;
	};
	TVoxelOptional<FPendingModifier> PendingModifier;

	struct FQueuedModifier
	{
		FVoxelPromise Promise;
		TSharedPtr<IVoxelHeightModifierRuntime> ModifierRuntime;
	};
	TVoxelArray<FQueuedModifier> QueuedModifiers;

	void ClearQueue();
	void ProcessQueue();
	bool ProcessQueue_ShouldContinue();
};
