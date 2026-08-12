// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelPointSet.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelNode_ScatterPoints.generated.h"

// Scatter points around parent points
// Output is only child points, not parents
USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_ScatterPoints : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);

	VOXEL_INPUT_PIN(FVoxelFloatRange, Radius, FVoxelFloatRange(0, 200));
	// In degrees
	VOXEL_INPUT_PIN(float, RadialOffset, 10.f);
	VOXEL_INPUT_PIN(FVoxelInt32Range, NumPoints, FVoxelInt32Range(5, 10));
	VOXEL_INPUT_PIN(FVoxelSeed, Seed, nullptr, AdvancedDisplay);

	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};