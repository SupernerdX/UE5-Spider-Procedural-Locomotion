// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelPointSet.h"
#include "VoxelNode_PruneByDistance.generated.h"

// Will prune any points closer to each others than Distance
USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_PruneByDistance : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	VOXEL_INPUT_PIN(float, Distance, 100.f);
	// Extends the requested chunk bounds, to get more cohesive pruning results on chunk boundaries
	// For example: Distance 200, BoundsExtension 2, we'll extend the bounds by 400
	// NOTE! Bounds Extension is Scatter only
	VOXEL_INPUT_PIN(float, BoundsExtension, 1.5f, AdvancedDisplay);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};