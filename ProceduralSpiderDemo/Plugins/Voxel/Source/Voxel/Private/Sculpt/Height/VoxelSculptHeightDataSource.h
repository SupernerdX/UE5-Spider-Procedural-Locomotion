// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkPtr.h"
#include "Sculpt/Height/VoxelSculptHeightContext.h"

class AVoxelSculptHeight;
class FVoxelSculptHeightLocalData;
struct FVoxelHeightModifier;
struct FVoxelSculptHeightData;

class IVoxelSculptHeightDataSource
{
public:
	explicit IVoxelSculptHeightDataSource(AVoxelSculptHeight& SculptActor);
	virtual ~IVoxelSculptHeightDataSource() = default;

	TVoxelBulkRef<FVoxelSculptHeightData> GetData() const;

public:
	virtual TSharedRef<IVoxelBulkLoader> GetBulkLoader() const = 0;

	virtual void SetSculptData(
		const TVoxelBulkRef<FVoxelSculptHeightData>& NewData,
		const TSharedRef<IVoxelBulkLoader>& NewBulkLoader) = 0;

	virtual FVoxelFuture ApplyModifier(const TSharedRef<FVoxelHeightModifier>& Modifier) = 0;

	virtual TSharedPtr<FVoxelSculptHeightLocalData> AsLocalData() { return {}; }

protected:
	bool IsEditor() const;
	TVoxelOptional<FVoxelSculptHeightContext> GetContext() const;
	virtual void SetData(const TVoxelBulkRef<FVoxelSculptHeightData>& Data);

private:
	const TVoxelObjectPtr<AVoxelSculptHeight> WeakSculptActor;
	TVoxelBulkRef<FVoxelSculptHeightData> PrivateData;
};
