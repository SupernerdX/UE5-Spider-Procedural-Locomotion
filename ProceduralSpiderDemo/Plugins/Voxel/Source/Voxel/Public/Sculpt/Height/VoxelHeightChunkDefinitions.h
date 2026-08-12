// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelHeightChunkDefinitions
{
	static constexpr int32 ChunkSizeLog2 = 4;
	static constexpr int32 ChunkSize = 1 << ChunkSizeLog2;
	static constexpr int32 ChunkCount = FMath::Square(ChunkSize);
};