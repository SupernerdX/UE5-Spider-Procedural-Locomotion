// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class FVoxelState;

class FVoxelStampDebug
{
public:
	static void Open(
		const FVoxelState& State,
		const FVector& Position);
};