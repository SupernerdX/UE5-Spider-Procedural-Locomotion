// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeBlendMode.h"
#include "VoxelVolumeLayerPinType.h"
#include "VoxelOutputNode_MetadataBase.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "VoxelOutputNode_OutputVolumeBase.generated.h"

USTRUCT()
struct VOXEL_API FVoxelOutputNode_OutputVolumeBase : public FVoxelOutputNode_MetadataBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Distance, nullptr);
	VOXEL_INPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType, nullptr);
	// The weight with which to overlay this graph surface and metadatas, over top of the previous state.
	// 0 means entirely the previous state, 1 means entirely this graph's state.
	// Used only with Override blend mode.
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Alpha, 1.f, AdvancedDisplay);

	VOXEL_INPUT_PIN(bool, EnableLayerOverride, false, AdvancedDisplay);
	VOXEL_INPUT_PIN(FVoxelVolumeLayerObject, LayerOverride, nullptr, AdvancedDisplay);
	VOXEL_INPUT_PIN(bool, EnableBlendModeOverride, false, AdvancedDisplay);
	VOXEL_INPUT_PIN(EVoxelVolumeBlendMode, BlendModeOverride, EVoxelVolumeBlendMode::Override, AdvancedDisplay);
};