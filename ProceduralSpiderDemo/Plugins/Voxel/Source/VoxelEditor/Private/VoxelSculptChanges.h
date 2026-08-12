// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Misc/Change.h"
#include "Bulk/VoxelBulkPtr.h"

struct FVoxelSculptVolumeData;
struct FVoxelSculptHeightData;

class FVoxelHeightSculptChange : public FSwapChange
{
public:
	const TVoxelBulkRef<FVoxelSculptHeightData> Snapshot;

	explicit FVoxelHeightSculptChange(const TVoxelBulkRef<FVoxelSculptHeightData>& Snapshot)
		: Snapshot(Snapshot)
	{
	}

	//~ Begin FSwapChange Interface
	virtual FString ToString() const override;
	virtual TUniquePtr<FChange> Execute(UObject* Object) override;
	//~ End FSwapChange Interface
};

class FVoxelVolumeSculptChange : public FSwapChange
{
public:
	const TVoxelBulkRef<FVoxelSculptVolumeData> Snapshot;

	explicit FVoxelVolumeSculptChange(const TVoxelBulkRef<FVoxelSculptVolumeData>& Snapshot)
		: Snapshot(Snapshot)
	{
	}

	//~ Begin FSwapChange Interface
	virtual FString ToString() const override;
	virtual TUniquePtr<FChange> Execute(UObject* Object) override;
	//~ End FSwapChange Interface
};