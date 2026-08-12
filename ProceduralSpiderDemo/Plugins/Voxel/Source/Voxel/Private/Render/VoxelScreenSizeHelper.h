// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelChunkKey.h"

struct FVoxelConfig;

struct FVoxelScreenSizeHelper
{
	double MinQuality;
	double MaxQuality;
	double ChunkToWorld;
	double QualityExponent;

	explicit FVoxelScreenSizeHelper(const FVoxelConfig& Config);

	double GetChunkQuality(const TVoxelArray<FVector>& Invokers, FVoxelChunkKey ChunkKey) const;
};