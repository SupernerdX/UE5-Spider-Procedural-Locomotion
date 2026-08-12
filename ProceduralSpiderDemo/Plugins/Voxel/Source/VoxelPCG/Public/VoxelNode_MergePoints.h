// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelPointSet.h"
#include "VoxelNode_MergePoints.generated.h"

USTRUCT(Category = "Point", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_MergePoints : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_VARIADIC_INPUT_PIN(FVoxelPointSet, Input, nullptr, 2);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};