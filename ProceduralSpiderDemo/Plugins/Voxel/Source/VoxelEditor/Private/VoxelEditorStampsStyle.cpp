// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"

VOXEL_INITIALIZE_STYLE(VoxelStampsStyle)
{
	Set("Voxel.Stamps.BlendMode.Add", new IMAGE_BRUSH("Stamps/BlendMode_Add", CoreStyleConstants::Icon16x16));
	Set("Voxel.Stamps.BlendMode.Subtract", new IMAGE_BRUSH("Stamps/BlendMode_Subtract", CoreStyleConstants::Icon16x16));
	Set("Voxel.Stamps.BlendMode.Min", new IMAGE_BRUSH("Stamps/BlendMode_Min", CoreStyleConstants::Icon16x16));
	Set("Voxel.Stamps.BlendMode.Max", new IMAGE_BRUSH("Stamps/BlendMode_Max", CoreStyleConstants::Icon16x16));
	Set("Voxel.Stamps.BlendMode.Override", new IMAGE_BRUSH("Stamps/BlendMode_Override", CoreStyleConstants::Icon16x16));
	Set("Voxel.Stamps.BlendMode.Intersect", new IMAGE_BRUSH("Stamps/BlendMode_Intersect", CoreStyleConstants::Icon16x16));
}