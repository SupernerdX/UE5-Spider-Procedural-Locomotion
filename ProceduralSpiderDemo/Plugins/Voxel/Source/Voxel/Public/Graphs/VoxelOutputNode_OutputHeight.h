// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Graphs/VoxelOutputNode_OutputHeightBase.h"
#include "VoxelOutputNode_OutputHeight.generated.h"

USTRUCT()
struct VOXEL_API FVoxelOutputNode_OutputHeight : public FVoxelOutputNode_OutputHeightBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelBox2D, Bounds, nullptr);
};