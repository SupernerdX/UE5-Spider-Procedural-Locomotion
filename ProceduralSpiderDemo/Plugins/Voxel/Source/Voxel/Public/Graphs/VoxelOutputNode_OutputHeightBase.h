// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightBlendMode.h"
#include "VoxelHeightLayerPinType.h"
#include "VoxelOutputNode_MetadataBase.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "VoxelOutputNode_OutputHeightBase.generated.h"

USTRUCT()
struct VOXEL_API FVoxelOutputNode_OutputHeightBase : public FVoxelOutputNode_MetadataBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Height, nullptr);
	VOXEL_INPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType, nullptr);

	// Height will be clamped between Min and Max
	VOXEL_INPUT_PIN(FVoxelFloatRange, HeightRange, FVoxelFloatRange(-1000.f, 1000.f));
	// If true and the blend mode is set to Override, HeightRange will be set relative to the previous stamps heights
	// Use this if your override graph is offsetting the previous height
	VOXEL_INPUT_PIN(bool, RelativeHeightRange, false, AdvancedDisplay);

	// The weight with which to overlay this graph surface and metadatas, over top of the previous state.
	// 0 means entirely the previous state, 1 means entirely this graph's state.
	// Used only with Override blend mode.
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Alpha, 1.f, AdvancedDisplay);

	VOXEL_INPUT_PIN(bool, EnableLayerOverride, false, AdvancedDisplay);
	VOXEL_INPUT_PIN(FVoxelHeightLayerObject, LayerOverride, nullptr, AdvancedDisplay);
	VOXEL_INPUT_PIN(bool, EnableBlendModeOverride, false, AdvancedDisplay);
	VOXEL_INPUT_PIN(EVoxelHeightBlendMode, BlendModeOverride, EVoxelHeightBlendMode::Override, AdvancedDisplay);
};