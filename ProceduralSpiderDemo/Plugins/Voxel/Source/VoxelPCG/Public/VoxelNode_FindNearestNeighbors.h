// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelPointSet.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "VoxelNode_FindNearestNeighbors.generated.h"

// Finds the nearest neighbor positions from a reference point set for each query point.
// Uses a spatial hash map for efficient lookups within the max search distance.
USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter", Keywords = "distance"))
struct VOXELPCG_API FVoxelNode_FindNearestNeighbors : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// The query points - find neighbors FOR each of these points
	VOXEL_INPUT_PIN(FVoxelPointSet, QueryPoints, nullptr);
	// The reference points - search WITHIN these points
	VOXEL_INPUT_PIN(FVoxelPointSet, ReferencePoints, nullptr);
	// Maximum search distance. Points beyond this distance will not be considered.
	VOXEL_INPUT_PIN(float, MaxDistance, 1000.f);
	// Extends the requested chunk bounds for reference points, to find neighbors across chunk boundaries.
	// For example: MaxDistance 200, BoundsExtension 1.5, we'll extend the bounds by 300.
	VOXEL_INPUT_PIN(float, BoundsExtension, 1.5f, AdvancedDisplay);

	// Closest neighbor position. Will be NaN if no neighbor found within MaxDistance.
	VOXEL_OUTPUT_PIN(FVoxelDoubleVectorBuffer, FirstNeighborPosition, DisplayName("Position"), Category("Neighbor 1::Collapsed"));
	// Second closest neighbor position. Will be NaN if no neighbor found within MaxDistance.
	VOXEL_OUTPUT_PIN(FVoxelDoubleVectorBuffer, SecondNeighborPosition, DisplayName("Position"), Category("Neighbor 2::Collapsed"));
	// Third closest neighbor position. Will be NaN if no neighbor found within MaxDistance.
	VOXEL_OUTPUT_PIN(FVoxelDoubleVectorBuffer, ThirdNeighborPosition, DisplayName("Position"), Category("Neighbor 3::Collapsed"));

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};
