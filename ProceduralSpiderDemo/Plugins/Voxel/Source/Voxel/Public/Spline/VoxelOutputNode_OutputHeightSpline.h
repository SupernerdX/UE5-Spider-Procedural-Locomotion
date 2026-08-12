// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Graphs/VoxelOutputNode_OutputHeightBase.h"
#include "VoxelOutputNode_OutputHeightSpline.generated.h"

USTRUCT()
struct VOXEL_API FVoxelOutputNode_OutputHeightSpline : public FVoxelOutputNode_OutputHeightBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Max width of the spline
	VOXEL_INPUT_PIN(float, MaxWidth, 1000.f);
};