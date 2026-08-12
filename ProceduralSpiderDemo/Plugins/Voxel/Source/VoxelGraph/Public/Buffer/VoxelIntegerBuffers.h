// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBaseBuffers.h"
#include "VoxelBufferStruct.h"
#include "VoxelIntegerBuffers.generated.h"

DECLARE_VOXEL_BUFFER(FVoxelIntPointBuffer, FIntPoint);

USTRUCT()
struct VOXELGRAPH_API FVoxelIntPointBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelIntPointBuffer, FIntPoint);

	UPROPERTY()
	FVoxelInt32Buffer X;

	UPROPERTY()
	FVoxelInt32Buffer Y;

	FORCEINLINE const FIntPoint operator[](const int32 Index) const
	{
		return FIntPoint(X[Index], Y[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FIntPoint& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_BUFFER(FVoxelIntVectorBuffer, FIntVector);

USTRUCT()
struct VOXELGRAPH_API FVoxelIntVectorBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelIntVectorBuffer, FIntVector);

	UPROPERTY()
	FVoxelInt32Buffer X;

	UPROPERTY()
	FVoxelInt32Buffer Y;

	UPROPERTY()
	FVoxelInt32Buffer Z;

	FORCEINLINE const FIntVector operator[](const int32 Index) const
	{
		return FIntVector(X[Index], Y[Index], Z[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FIntVector& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
		Z.Set(Index, Value.Z);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_BUFFER(FVoxelIntVector4Buffer, FIntVector4);

USTRUCT()
struct VOXELGRAPH_API FVoxelIntVector4Buffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelIntVector4Buffer, FIntVector4);

	UPROPERTY()
	FVoxelInt32Buffer X;

	UPROPERTY()
	FVoxelInt32Buffer Y;

	UPROPERTY()
	FVoxelInt32Buffer Z;

	UPROPERTY()
	FVoxelInt32Buffer W;

	FORCEINLINE const FIntVector4 operator[](const int32 Index) const
	{
		return FIntVector4(X[Index], Y[Index], Z[Index], W[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FIntVector4& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
		Z.Set(Index, Value.Z);
		W.Set(Index, Value.W);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_BUFFER(FVoxelInt64PointBuffer, FInt64Point);

USTRUCT()
struct VOXELGRAPH_API FVoxelInt64PointBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelInt64PointBuffer, FInt64Point);

	UPROPERTY()
	FVoxelInt64Buffer X;

	UPROPERTY()
	FVoxelInt64Buffer Y;

	FORCEINLINE const FInt64Point operator[](const int32 Index) const
	{
		return FInt64Point(X[Index], Y[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FInt64Point& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_BUFFER(FVoxelInt64VectorBuffer, FInt64Vector);

USTRUCT()
struct VOXELGRAPH_API FVoxelInt64VectorBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelInt64VectorBuffer, FInt64Vector);

	UPROPERTY()
	FVoxelInt64Buffer X;

	UPROPERTY()
	FVoxelInt64Buffer Y;

	UPROPERTY()
	FVoxelInt64Buffer Z;

	FORCEINLINE const FInt64Vector operator[](const int32 Index) const
	{
		return FInt64Vector(X[Index], Y[Index], Z[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FInt64Vector& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
		Z.Set(Index, Value.Z);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_BUFFER(FVoxelInt64Vector4Buffer, FInt64Vector4);

USTRUCT()
struct VOXELGRAPH_API FVoxelInt64Vector4Buffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelInt64Vector4Buffer, FInt64Vector4);

	UPROPERTY()
	FVoxelInt64Buffer X;

	UPROPERTY()
	FVoxelInt64Buffer Y;

	UPROPERTY()
	FVoxelInt64Buffer Z;

	UPROPERTY()
	FVoxelInt64Buffer W;

	FORCEINLINE const FInt64Vector4 operator[](const int32 Index) const
	{
		return FInt64Vector4(X[Index], Y[Index], Z[Index], W[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FInt64Vector4& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
		Z.Set(Index, Value.Z);
		W.Set(Index, Value.W);
	}
};