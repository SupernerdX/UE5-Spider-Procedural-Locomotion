// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodeInterface.h"

struct VOXELPCG_API FVoxelPointFilterStats
{
#if WITH_EDITOR
	static void RecordNodeStats(const IVoxelNodeInterface& Node, int64 Count, int64 RemainingCount);
#else
	static void RecordNodeStats(const IVoxelNodeInterface& Node, int64 Count, int64 RemainingCount) {}
#endif
};