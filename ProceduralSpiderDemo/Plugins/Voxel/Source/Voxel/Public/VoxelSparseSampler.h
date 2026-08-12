// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class FVoxelQuery;
class FVoxelLayers;
class FVoxelVolumeLayer;

// Shared utility for sparse sampling to determine which regions need full-resolution querying.
// Used by both cubic and surface nets meshers to skip uniform regions.
class VOXEL_API FVoxelSparseSampler
{
public:
	// Computes which subregions of VolumeBounds need full-resolution querying.
	// Uses 4x4x4 sparse sampling with distance threshold filtering,
	// sign transition detection, and neighbor propagation.
	//
	// @param Query - The voxel query context
	// @param VolumeLayer - The volume layer to sample distances from
	// @param VolumeBounds - The integer bounds to process (relative to grid origin)
	// @param Start - World-space start position of the grid
	// @param CellSize - Size of each cell in world units
	// @param Layers - Optional, for debug visualization
	// @param LOD - Optional, used for debug visualization colors
	// @return Array of bounds regions that need full-resolution querying, empty if uniform
	static TVoxelArray<FVoxelIntBox> ComputeBoundsToQuery(
		const FVoxelQuery& Query,
		const FVoxelVolumeLayer& VolumeLayer,
		const FVoxelIntBox& VolumeBounds,
		const FVector& Start,
		float CellSize,
		const FVoxelLayers* Layers = nullptr,
		int32 LOD = 0);

private:
	static constexpr int32 SparseMultiplier = 4;
};
