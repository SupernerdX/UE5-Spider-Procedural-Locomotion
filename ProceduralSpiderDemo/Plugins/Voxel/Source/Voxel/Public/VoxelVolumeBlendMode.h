// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeBlendMode.generated.h"

UENUM(BlueprintType, meta = (VoxelSegmentedEnum))
enum class EVoxelVolumeBlendMode : uint8
{
	Additive UMETA(ToolTip = "Additive: will add even if there's no stamps before us", StyleSet = "VoxelStyle", Icon = "Voxel.Stamps.BlendMode.Add"),
	Subtractive UMETA(ToolTip = "Subtractive: will carve out even if there's no stamps before us", StyleSet = "VoxelStyle", Icon = "Voxel.Stamps.BlendMode.Subtract"),
	Intersect UMETA(ToolTip = "Intersect", StyleSet = "VoxelStyle", Icon = "Voxel.Stamps.BlendMode.Intersect"),
	Override UMETA(ToolTip = "Override", StyleSet = "VoxelStyle", Icon = "Voxel.Stamps.BlendMode.Override")
};