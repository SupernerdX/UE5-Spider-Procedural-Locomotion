// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLevelToolType.generated.h"

UENUM(BlueprintType, meta = (VoxelSegmentedEnum))
enum class EVoxelLevelToolType : uint8
{
	Additive UMETA(ToolTip = "Additive mode: will only grow the surface", StyleSet = "VoxelStyle", Icon = "Voxel.Sculpt.LevelTool.Additive"),
	Subtractive UMETA(ToolTip = "Substractive mode: will only shrink the surface", StyleSet = "VoxelStyle", Icon = "Voxel.Sculpt.LevelTool.Subtractive"),
	Both UMETA(ToolTip = "Will grow and shrink the surface at the same time", StyleSet = "VoxelStyle", Icon = "Voxel.Sculpt.LevelTool.Both")
};