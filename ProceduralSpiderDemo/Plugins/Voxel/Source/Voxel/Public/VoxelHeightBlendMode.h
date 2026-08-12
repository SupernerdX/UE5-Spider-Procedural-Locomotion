// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightBlendMode.generated.h"

UENUM(BlueprintType, meta = (VoxelSegmentedEnum))
enum class EVoxelHeightBlendMode : uint8
{
	Max UMETA(ToolTip = "Max", StyleSet = "VoxelStyle", Icon = "Voxel.Stamps.BlendMode.Max"),
	Min UMETA(ToolTip = "Min", StyleSet = "VoxelStyle", Icon = "Voxel.Stamps.BlendMode.Min"),
	Override UMETA(ToolTip = "Override", StyleSet = "VoxelStyle", Icon = "Voxel.Stamps.BlendMode.Override"),
};