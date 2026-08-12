// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"

struct VOXEL_API FVoxelSurfaceTypeBlendUtilities
{
	static FVoxelSurfaceTypeBlendBuffer Lerp(
		const FVoxelSurfaceTypeBlendBuffer& A,
		const FVoxelSurfaceTypeBlendBuffer& B,
		const FVoxelFloatBuffer& Alpha);
};