// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelCoreMinimal.h"

enum class EVoxelIterate : uint8
{
	Continue,
	Stop
};

enum class EVoxelIterateTree : uint8
{
	Continue,
	SkipChildren,
	Stop
};