// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Graphs/VoxelOutputNode_OutputVolumeBase.h"
#include "VoxelOutputNode_OutputVolume.generated.h"

USTRUCT()
struct VOXEL_API FVoxelOutputNode_OutputVolume : public FVoxelOutputNode_OutputVolumeBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelBox, Bounds, nullptr);
};