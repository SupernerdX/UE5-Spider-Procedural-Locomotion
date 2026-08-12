// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Nodes/VoxelOutputNode.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelOutputNode_OutputSculptHeight.generated.h"

USTRUCT()
struct VOXEL_API FVoxelOutputNode_OutputSculptHeight : public FVoxelOutputNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(float, Radius, 1000.f);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Height, nullptr);
};