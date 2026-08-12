// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightStamp.h"

struct FVoxelWeakStackLayer;

struct FVoxelHeightStampWrapper
{
public:
	static void Apply(
		const FVoxelWeakStackLayer& Layer,
		const FVoxelHeightStampRuntime& Stamp,
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& StampToQuery);

	static void Apply(
		const FVoxelWeakStackLayer& Layer,
		const FVoxelHeightStampRuntime& Stamp,
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery);

private:
	static void ApplyImpl(
		const FVoxelWeakStackLayer& Layer,
		const FVoxelHeightStampRuntime& Stamp,
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& StampToQuery);

	static void ApplyImpl(
		const FVoxelWeakStackLayer& Layer,
		const FVoxelHeightStampRuntime& Stamp,
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery);
};