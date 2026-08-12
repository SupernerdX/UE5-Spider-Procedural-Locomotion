// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBaseBuffers.h"
#include "VoxelBufferStruct.h"
#include "VoxelFloatBuffers.generated.h"

DECLARE_VOXEL_BUFFER(FVoxelVector2DBuffer, FVector2D);

USTRUCT(DisplayName = "Vector2D Buffer")
struct VOXELGRAPH_API FVoxelVector2DBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(FVoxelVector2DBuffer, FVector2D, FVector2f);

	UPROPERTY()
	FVoxelFloatBuffer X;

	UPROPERTY()
	FVoxelFloatBuffer Y;

	FORCEINLINE const FVector2f operator[](const int32 Index) const
	{
		return FVector2f(X[Index], Y[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FVector2f& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace ispc
{
#ifndef __ISPC_STRUCT_FVoxelVectorBuffer__
#define __ISPC_STRUCT_FVoxelVectorBuffer__
struct FVoxelVectorBuffer
{
	FVoxelFloatBuffer X;
	FVoxelFloatBuffer Y;
	FVoxelFloatBuffer Z;
};
#endif
}

DECLARE_VOXEL_BUFFER(FVoxelVectorBuffer, FVector);

USTRUCT()
struct VOXELGRAPH_API FVoxelVectorBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(FVoxelVectorBuffer, FVector, FVector3f);

	UPROPERTY()
	FVoxelFloatBuffer X;

	UPROPERTY()
	FVoxelFloatBuffer Y;

	UPROPERTY()
	FVoxelFloatBuffer Z;

	FORCEINLINE FVoxelVectorBuffer(const float Value)
		: FVoxelVectorBuffer(FVector3f(Value))
	{
	}

	FORCEINLINE const FVector3f operator[](const int32 Index) const
	{
		return FVector3f(X[Index], Y[Index], Z[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FVector3f& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
		Z.Set(Index, Value.Z);
	}

	FORCEINLINE ispc::FVoxelVectorBuffer ISPC() const
	{
		return ispc::FVoxelVectorBuffer
		{
			X.ISPC(),
			Y.ISPC(),
			Z.ISPC()
		};
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_BUFFER(FVoxelLinearColorBuffer, FLinearColor);

USTRUCT()
struct VOXELGRAPH_API FVoxelLinearColorBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelLinearColorBuffer, FLinearColor);

	UPROPERTY()
	FVoxelFloatBuffer R;

	UPROPERTY()
	FVoxelFloatBuffer G;

	UPROPERTY()
	FVoxelFloatBuffer B;

	UPROPERTY()
	FVoxelFloatBuffer A;

	FORCEINLINE const FLinearColor operator[](const int32 Index) const
	{
		return FLinearColor(R[Index], G[Index], B[Index], A[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FLinearColor& Value)
	{
		R.Set(Index, Value.R);
		G.Set(Index, Value.G);
		B.Set(Index, Value.B);
		A.Set(Index, Value.A);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_BUFFER(FVoxelQuaternionBuffer, FQuat);

USTRUCT()
struct VOXELGRAPH_API FVoxelQuaternionBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(FVoxelQuaternionBuffer, FQuat, FQuat4f);

	UPROPERTY()
	FVoxelFloatBuffer X;

	UPROPERTY()
	FVoxelFloatBuffer Y;

	UPROPERTY()
	FVoxelFloatBuffer Z;

	UPROPERTY()
	FVoxelFloatBuffer W;

	FORCEINLINE const FQuat4f operator[](const int32 Index) const
	{
		return FQuat4f(X[Index], Y[Index], Z[Index], W[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FQuat4f& Value)
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

DECLARE_VOXEL_BUFFER(FVoxelTransformBuffer, FTransform);

USTRUCT()
struct VOXELGRAPH_API FVoxelTransformBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(FVoxelTransformBuffer, FTransform, FTransform3f);

	UPROPERTY()
	FVoxelQuaternionBuffer Rotation;

	UPROPERTY()
	FVoxelVectorBuffer Translation;

	UPROPERTY()
	FVoxelVectorBuffer Scale;

	FORCEINLINE const FTransform3f operator[](const int32 Index) const
	{
		return FTransform3f(Rotation[Index], Translation[Index], Scale[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FTransform3f& Value)
	{
		Rotation.Set(Index, Value.GetRotation());
		Translation.Set(Index, Value.GetTranslation());
		Scale.Set(Index, Value.GetScale3D());
	}
};