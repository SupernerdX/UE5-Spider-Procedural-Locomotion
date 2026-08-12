// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Buffer/VoxelFloatBuffers.h"

struct VOXELGRAPH_API FVoxelBufferGradientUtilities
{
public:
	static FVoxelVector2DBuffer SplitPositions2D(
		const FVoxelVector2DBuffer& Positions,
		float Step);
	static FVoxelDoubleVector2DBuffer SplitPositions2D(
		const FVoxelDoubleVector2DBuffer& Positions,
		float Step);
	static FVoxelVector2DBuffer SplitPositions2D(
		const FVoxelVectorBuffer& Positions,
		float Step);
	static FVoxelDoubleVector2DBuffer SplitPositions2D(
		const FVoxelDoubleVectorBuffer& Positions,
		float Step);

public:
	static FVoxelVectorBuffer SplitPositions3D(
		const FVoxelVectorBuffer& Positions,
		float Step);
	static FVoxelDoubleVectorBuffer SplitPositions3D(
		const FVoxelDoubleVectorBuffer& Positions,
		float Step);

public:
	static FVoxelVector2DBuffer CollapseGradient2D(
		const FVoxelFloatBuffer& Heights,
		int32 NumPositions,
		float Step);

	static FVoxelVectorBuffer CollapseGradient2DToNormal(
		const FVoxelFloatBuffer& Heights,
		int32 NumPositions,
		float Step);

	static FVoxelVectorBuffer CollapseGradient3D(
		const FVoxelFloatBuffer& Distances,
		int32 NumPositions,
		float Step);

	static FVoxelVectorBuffer CollapseGradient3DToNormal(
		const FVoxelFloatBuffer& Distances,
		int32 NumPositions,
		float Step);
};