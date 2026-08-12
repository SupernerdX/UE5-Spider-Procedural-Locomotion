// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeStamp.h"

struct FVoxelWeakStackLayer;

struct FVoxelVolumeStampWrapper
{
public:
	static void Apply(
		const FVoxelWeakStackLayer& Layer,
		const FVoxelVolumeStampRuntime& Stamp,
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& StampToQuery);

	static void Apply(
		const FVoxelWeakStackLayer& Layer,
		const FVoxelVolumeStampRuntime& Stamp,
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery);

private:
	static void ApplyImpl(
		const FVoxelWeakStackLayer& Layer,
		const FVoxelVolumeStampRuntime& Stamp,
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& StampToQuery);

	static void ApplyImpl(
		const FVoxelWeakStackLayer& Layer,
		const FVoxelVolumeStampRuntime& Stamp,
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery);
};