// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"

struct VOXELGRAPH_API FVoxelBufferTransformUtilities
{
public:
	static FVoxelVectorBuffer ApplyTransform(const FVoxelVectorBuffer& Buffer, const FTransform& Transform);
	static FVoxelVectorBuffer ApplyInverseTransform(const FVoxelVectorBuffer& Buffer, const FTransform& Transform);
	
public:
	static FVoxelVectorBuffer ApplyTransform(const FVoxelVectorBuffer& Buffer, const FVoxelTransformBuffer& Transform);
	static FVoxelVectorBuffer ApplyInverseTransform(const FVoxelVectorBuffer& Buffer, const FVoxelTransformBuffer& Transform);

public:
	static FVoxelDoubleVectorBuffer ApplyTransform(const FVoxelDoubleVectorBuffer& Buffer, const FTransform& Transform);
	static FVoxelDoubleVectorBuffer ApplyInverseTransform(const FVoxelDoubleVectorBuffer& Buffer, const FTransform& Transform);

public:
	static FVoxelVector2DBuffer ApplyTransform(const FVoxelVector2DBuffer& Buffer, const FTransform2d& Transform);
	static FVoxelDoubleVector2DBuffer ApplyTransform(const FVoxelDoubleVector2DBuffer& Buffer, const FTransform2d& Transform);

	static FVoxelVector2DBuffer ApplyInverseTransform(const FVoxelVector2DBuffer& Buffer, const FTransform2d& Transform);
	static FVoxelDoubleVector2DBuffer ApplyInverseTransform(const FVoxelDoubleVector2DBuffer& Buffer, const FTransform2d& Transform);

public:
	static FVoxelVectorBuffer ApplyTransform(
		const FVoxelVectorBuffer& Buffer,
		const FVoxelVectorBuffer& Translation,
		const FVoxelQuaternionBuffer& Rotation,
		const FVoxelVectorBuffer& Scale);

public:
	static FVoxelVectorBuffer ApplyTransform(const FVoxelVectorBuffer& Buffer, const FMatrix& Transform);
	static FVoxelDoubleVectorBuffer ApplyTransform(const FVoxelDoubleVectorBuffer& Buffer, const FMatrix& Transform);

public:
	static FVoxelFloatBuffer TransformDistance(const FVoxelFloatBuffer& Distance, const FMatrix& Transform);
};