// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

struct FToolMenuEntry;

class FVoxelGlobalActionsExtender
{
public:
	static void RegisterMenu();
	static void UnregisterMenu();
};