// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Buffer/VoxelIntegerBuffers.h"

struct VOXELGRAPH_API FVoxelBufferMathUtilities
{
public:
	static FVoxelFloatBuffer Min(
		const FVoxelFloatBuffer& Buffer0,
		const FVoxelFloatBuffer& Buffer1);

	static FVoxelFloatBuffer Max(
		const FVoxelFloatBuffer& Buffer0,
		const FVoxelFloatBuffer& Buffer1);

public:
	static FVoxelFloatBuffer Min8(
		const FVoxelFloatBuffer& Buffer0,
		const FVoxelFloatBuffer& Buffer1,
		const FVoxelFloatBuffer& Buffer2,
		const FVoxelFloatBuffer& Buffer3,
		const FVoxelFloatBuffer& Buffer4,
		const FVoxelFloatBuffer& Buffer5,
		const FVoxelFloatBuffer& Buffer6,
		const FVoxelFloatBuffer& Buffer7);

	static FVoxelFloatBuffer Max8(
		const FVoxelFloatBuffer& Buffer0,
		const FVoxelFloatBuffer& Buffer1,
		const FVoxelFloatBuffer& Buffer2,
		const FVoxelFloatBuffer& Buffer3,
		const FVoxelFloatBuffer& Buffer4,
		const FVoxelFloatBuffer& Buffer5,
		const FVoxelFloatBuffer& Buffer6,
		const FVoxelFloatBuffer& Buffer7);

	static FVoxelVectorBuffer Min8(
		const FVoxelVectorBuffer& Buffer0,
		const FVoxelVectorBuffer& Buffer1,
		const FVoxelVectorBuffer& Buffer2,
		const FVoxelVectorBuffer& Buffer3,
		const FVoxelVectorBuffer& Buffer4,
		const FVoxelVectorBuffer& Buffer5,
		const FVoxelVectorBuffer& Buffer6,
		const FVoxelVectorBuffer& Buffer7);

	static FVoxelVectorBuffer Max8(
		const FVoxelVectorBuffer& Buffer0,
		const FVoxelVectorBuffer& Buffer1,
		const FVoxelVectorBuffer& Buffer2,
		const FVoxelVectorBuffer& Buffer3,
		const FVoxelVectorBuffer& Buffer4,
		const FVoxelVectorBuffer& Buffer5,
		const FVoxelVectorBuffer& Buffer6,
		const FVoxelVectorBuffer& Buffer7);

public:
	static FVoxelFloatBuffer Add(const FVoxelFloatBuffer& A, const FVoxelFloatBuffer& B);
	static FVoxelDoubleBuffer Add(const FVoxelDoubleBuffer& A, const FVoxelDoubleBuffer& B);
	static FVoxelFloatBuffer Multiply(const FVoxelFloatBuffer& A, const FVoxelFloatBuffer& B);
	static FVoxelBoolBuffer Less(const FVoxelFloatBuffer& A, const FVoxelFloatBuffer& B);
	static FVoxelBoolBuffer IsFinite(const FVoxelFloatBuffer& Values);

public:
	static FVoxelVectorBuffer Add(const FVoxelVectorBuffer& A, const FVoxelVectorBuffer& B);
	static FVoxelDoubleVectorBuffer Add(const FVoxelDoubleVectorBuffer& A, const FVoxelDoubleVectorBuffer& B);
	static FVoxelVectorBuffer Multiply(const FVoxelVectorBuffer& A, const FVoxelVectorBuffer& B);

public:
	static FVoxelFloatBuffer Lerp(const FVoxelFloatBuffer& A, const FVoxelFloatBuffer& B, const FVoxelFloatBuffer& Alpha);
	static FVoxelVector2DBuffer Lerp(const FVoxelVector2DBuffer& A, const FVoxelVector2DBuffer& B, const FVoxelFloatBuffer& Alpha);
	static FVoxelVectorBuffer Lerp(const FVoxelVectorBuffer& A, const FVoxelVectorBuffer& B, const FVoxelFloatBuffer& Alpha);
	static FVoxelLinearColorBuffer Lerp(const FVoxelLinearColorBuffer& A, const FVoxelLinearColorBuffer& B, const FVoxelFloatBuffer& Alpha);
	static FVoxelQuaternionBuffer Lerp(const FVoxelQuaternionBuffer& A, const FVoxelQuaternionBuffer& B, const FVoxelFloatBuffer& Alpha);

public:
	static FVoxelQuaternionBuffer Combine(const FVoxelQuaternionBuffer& A, const FVoxelQuaternionBuffer& B);
	static FVoxelVectorBuffer RotateVector(const FVoxelQuaternionBuffer& Quaternion, const FVoxelVectorBuffer& Vector);
	static FVoxelVectorBuffer UnrotateVector(const FVoxelQuaternionBuffer& Quaternion, const FVoxelVectorBuffer& Vector);
	static FVoxelVectorBuffer MakeEulerFromQuaternion(const FVoxelQuaternionBuffer& Quaternion);
	static FVoxelQuaternionBuffer MakeQuaternionFromEuler(const FVoxelVectorBuffer& Euler);

public:
	static FVoxelVector2DBuffer RotateVector2D(const FVoxelVector2DBuffer& Vector, const FVoxelFloatBuffer& AngleDeg);

public:
	static FVoxelFloatBuffer Length(const FVoxelVector2DBuffer& Vector);
	static FVoxelFloatBuffer Length(const FVoxelVectorBuffer& Vector);
	static FVoxelFloatBuffer Length(const FVoxelIntPointBuffer& Vector);
	static FVoxelFloatBuffer Length(const FVoxelIntVectorBuffer& Vector);

public:
	static FVoxelFloatBuffer SquaredLength(const FVoxelVector2DBuffer& Vector);
	static FVoxelFloatBuffer SquaredLength(const FVoxelVectorBuffer& Vector);
	static FVoxelFloatBuffer SquaredLength(const FVoxelIntPointBuffer& Vector);
	static FVoxelFloatBuffer SquaredLength(const FVoxelIntVectorBuffer& Vector);

public:
	static FVoxelVector2DBuffer Normalize(const FVoxelVector2DBuffer& Vector);
	static FVoxelVectorBuffer Normalize(const FVoxelVectorBuffer& Vector);
	static FVoxelVector2DBuffer Normalize(const FVoxelIntPointBuffer& Vector);
	static FVoxelVectorBuffer Normalize(const FVoxelIntVectorBuffer& Vector);

public:
	static FVoxelQuaternionBuffer MakeQuaternionFromZ(const FVoxelVectorBuffer& Vector);
};