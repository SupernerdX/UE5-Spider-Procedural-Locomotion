// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Graphs/VoxelOutputNode_OutputVolumeBase.h"
#include "VoxelOutputNode_OutputVolumeSpline.generated.h"

USTRUCT()
struct VOXEL_API FVoxelOutputNode_OutputVolumeSpline : public FVoxelOutputNode_OutputVolumeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(float, MaxWidth, 1000.f);
};