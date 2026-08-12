// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"

VOXEL_INITIALIZE_STYLE(VoxelSculptStyle)
{
	Set("Voxel.Sculpt.LevelTool.Additive", new IMAGE_BRUSH_SVG("Sculpt/LevelTool_Additive", CoreStyleConstants::Icon16x16));
	Set("Voxel.Sculpt.LevelTool.Subtractive", new IMAGE_BRUSH_SVG("Sculpt/LevelTool_Subtractive", CoreStyleConstants::Icon16x16));
	Set("Voxel.Sculpt.LevelTool.Both", new IMAGE_BRUSH_SVG("Sculpt/LevelTool_Both", CoreStyleConstants::Icon16x16));
}