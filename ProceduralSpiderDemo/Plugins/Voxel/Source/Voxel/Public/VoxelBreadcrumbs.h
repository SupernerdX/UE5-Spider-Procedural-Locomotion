// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampQuery.h"

struct FVoxelWeakStackLayer;
struct FVoxelHeightStampRuntime;
struct FVoxelVolumeStampRuntime;

struct VOXEL_API FVoxelBreadcrumbs
{
	struct FDelegates
	{
		TFunction<void(
			const FVoxelWeakStackLayer& Layer,
			const FVoxelHeightStampRuntime& Stamp,
			const FVoxelHeightBulkQuery& Query)> BulkHeight;

		TFunction<void(
			const FVoxelWeakStackLayer& Layer,
			const FVoxelHeightStampRuntime& Stamp,
			const FVoxelHeightSparseQuery& Query)> SparseHeight;

		TFunction<void(
			const FVoxelWeakStackLayer& Layer,
			const FVoxelVolumeStampRuntime& Stamp,
			const FVoxelVolumeBulkQuery& Query)> BulkVolume;

		TFunction<void(
			const FVoxelWeakStackLayer& Layer,
			const FVoxelVolumeStampRuntime& Stamp,
			const FVoxelVolumeSparseQuery& Query)> SparseVolume;
	};

	FDelegates PreApplyStamp;
	FDelegates PostApplyStamp;
};