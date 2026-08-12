// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelPointSet.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelPointNodes.generated.h"

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter", Keywords = "filter"))
struct VOXELPCG_API FVoxelNode_SplitPoints : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	VOXEL_INPUT_PIN(FVoxelBoolBuffer, Condition, true);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, True);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, False);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_DensityFilter : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Density, 1.f);
	VOXEL_INPUT_PIN(FVoxelSeed, Seed, nullptr, AdvancedDisplay);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, A);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, B);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};