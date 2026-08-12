// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBaseBuffers.h"
#include "VoxelBufferStruct.h"
#include "VoxelDoubleBuffers.generated.h"

USTRUCT(BlueprintType)
struct VOXELGRAPH_API FVoxelDoubleVector2D
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vector2D")
	double X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vector2D")
	double Y = 0;

	FVoxelDoubleVector2D() = default;
	FORCEINLINE FVoxelDoubleVector2D(const FVector2D& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
	{
	}
	FORCEINLINE FVoxelDoubleVector2D(
		const double X,
		const double Y)
		: X(X)
		, Y(Y)
	{
	}
	FORCEINLINE operator const FVector2D&() const
	{
		return ReinterpretCastRef<FVector2D>(*this);
	}

	FString ToString() const;
};

DECLARE_VOXEL_DOUBLE_BUFFER(FVoxelDoubleVector2DBuffer, FVoxelDoubleVector2D, FVector2D);

USTRUCT(DisplayName = "Double Vector2D Buffer")
struct VOXELGRAPH_API FVoxelDoubleVector2DBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(FVoxelDoubleVector2DBuffer, FVoxelDoubleVector2D, FVector2D);

	UPROPERTY()
	FVoxelDoubleBuffer X;

	UPROPERTY()
	FVoxelDoubleBuffer Y;

	FORCEINLINE const FVector2D operator[](const int32 Index) const
	{
		return FVector2D(X[Index], Y[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FVector2D& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct VOXELGRAPH_API FVoxelDoubleVector
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vector")
	double X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vector")
	double Y = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vector")
	double Z = 0;

	FVoxelDoubleVector() = default;
	FORCEINLINE FVoxelDoubleVector(const double Value)
		: X(Value)
		, Y(Value)
		, Z(Value)
	{
	}
	FORCEINLINE FVoxelDoubleVector(const FVector& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
		, Z(Vector.Z)
	{
	}
	FORCEINLINE FVoxelDoubleVector(
		const double X,
		const double Y,
		const double Z)
		: X(X)
		, Y(Y)
		, Z(Z)
	{
	}
	FORCEINLINE operator const FVector&() const
	{
		return ReinterpretCastRef<FVector>(*this);
	}

	FString ToString() const;
};

namespace ispc
{
#ifndef __ISPC_STRUCT_FVoxelDoubleVectorBuffer__
#define __ISPC_STRUCT_FVoxelDoubleVectorBuffer__
struct FVoxelDoubleVectorBuffer
{
	FVoxelDoubleBuffer X;
	FVoxelDoubleBuffer Y;
	FVoxelDoubleBuffer Z;
};
#endif
}

DECLARE_VOXEL_DOUBLE_BUFFER(FVoxelDoubleVectorBuffer, FVoxelDoubleVector, FVector);

USTRUCT()
struct VOXELGRAPH_API FVoxelDoubleVectorBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(FVoxelDoubleVectorBuffer, FVoxelDoubleVector, FVector);

	UPROPERTY()
	FVoxelDoubleBuffer X;

	UPROPERTY()
	FVoxelDoubleBuffer Y;

	UPROPERTY()
	FVoxelDoubleBuffer Z;

	FORCEINLINE FVoxelDoubleVectorBuffer(const double Value)
		: FVoxelDoubleVectorBuffer(FVector(Value))
	{
	}

	FORCEINLINE const FVector operator[](const int32 Index) const
	{
		return FVector(X[Index], Y[Index], Z[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FVector& Value)
	{
		X.Set(Index, Value.X);
		Y.Set(Index, Value.Y);
		Z.Set(Index, Value.Z);
	}

	FORCEINLINE ispc::FVoxelDoubleVectorBuffer ISPC() const
	{
		return ispc::FVoxelDoubleVectorBuffer
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

USTRUCT(BlueprintType)
struct alignas(16) VOXELGRAPH_API FVoxelDoubleQuat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quat")
	double X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quat")
	double Y = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quat")
	double Z = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quat")
	double W = 0;

	FVoxelDoubleQuat() = default;
	FORCEINLINE FVoxelDoubleQuat(const FQuat4d& Quat)
		: X(Quat.X)
		, Y(Quat.Y)
		, Z(Quat.Z)
		, W(Quat.W)
	{
	}
	FORCEINLINE FVoxelDoubleQuat(
		const double X,
		const double Y,
		const double Z,
		const double W)
		: X(X)
		, Y(Y)
		, Z(Z)
		, W(W)
	{
	}
	FORCEINLINE operator const FQuat&() const
	{
		return ReinterpretCastRef<FQuat>(*this);
	}
};

DECLARE_VOXEL_DOUBLE_BUFFER(FVoxelDoubleQuaternionBuffer, FVoxelDoubleQuat, FQuat);

USTRUCT()
struct VOXELGRAPH_API FVoxelDoubleQuaternionBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(FVoxelDoubleQuaternionBuffer, FVoxelDoubleQuat, FQuat);

	UPROPERTY()
	FVoxelDoubleBuffer X;

	UPROPERTY()
	FVoxelDoubleBuffer Y;

	UPROPERTY()
	FVoxelDoubleBuffer Z;

	UPROPERTY()
	FVoxelDoubleBuffer W;

	FORCEINLINE const FQuat operator[](const int32 Index) const
	{
		return FQuat(X[Index], Y[Index], Z[Index], W[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FQuat& Value)
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

USTRUCT(BlueprintType)
struct VOXELGRAPH_API FVoxelDoubleTransform
{
	GENERATED_BODY()

	// Rotation of this transformation, as a quaternion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Transform)
	FVoxelDoubleQuat Rotation = FQuat::Identity;

	// Translation of this transformation, as a vector
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Transform)
#if CPP
	alignas(16)
#endif
	FVoxelDoubleVector Translation = FVector::ZeroVector;

	// 3D scale (always applied in local space) as a vector
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Transform)
#if CPP
	alignas(16)
#endif
	FVoxelDoubleVector Scale3D = FVector::ZeroVector;

	FVoxelDoubleTransform() = default;
	FORCEINLINE FVoxelDoubleTransform(const FTransform& Transform)
		: Rotation(Transform.GetRotation())
		, Translation(Transform.GetTranslation())
		, Scale3D(Transform.GetScale3D())
	{
	}
	FORCEINLINE operator const FTransform&() const
	{
		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Rotation.X == Rotation.X);
		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Rotation.Y == Rotation.Y);
		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Rotation.Z == Rotation.Z);
		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Rotation.W == Rotation.W);

		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Translation.X == Translation.X);
		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Translation.Y == Translation.Y);
		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Translation.Z == Translation.Z);

		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Scale3D.X == Scale3D.X);
		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Scale3D.Y == Scale3D.Y);
		checkVoxelSlow(FVoxelDoubleTransform(ReinterpretCastRef<FTransform>(*this)).Scale3D.Z == Scale3D.Z);

		return ReinterpretCastRef<FTransform>(*this);
	}
};

struct FVoxelDoubleTransformChecks : FTransform
{
	static void Test()
	{
		checkStatic(offsetof(FVoxelDoubleTransformChecks, Rotation) == offsetof(FVoxelDoubleTransform, Rotation));
		checkStatic(offsetof(FVoxelDoubleTransformChecks, Translation) == offsetof(FVoxelDoubleTransform, Translation));
		checkStatic(offsetof(FVoxelDoubleTransformChecks, Scale3D) == offsetof(FVoxelDoubleTransform, Scale3D));
	}
};

DECLARE_VOXEL_DOUBLE_BUFFER(FVoxelDoubleTransformBuffer, FVoxelDoubleTransform, FTransform);

USTRUCT()
struct VOXELGRAPH_API FVoxelDoubleTransformBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY_IMPL(FVoxelDoubleTransformBuffer, FVoxelDoubleTransform, FTransform);

	UPROPERTY()
	FVoxelDoubleQuaternionBuffer Rotation;

	UPROPERTY()
	FVoxelDoubleVectorBuffer Translation;

	UPROPERTY()
	FVoxelDoubleVectorBuffer Scale;

	FORCEINLINE const FTransform operator[](const int32 Index) const
	{
		return FTransform(Rotation[Index], Translation[Index], Scale[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FTransform& Value)
	{
		Rotation.Set(Index, Value.GetRotation());
		Translation.Set(Index, Value.GetTranslation());
		Scale.Set(Index, Value.GetScale3D());
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct alignas(16) VOXELGRAPH_API FVoxelDoubleLinearColor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LinearColor)
	double R = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LinearColor)
	double G = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LinearColor)
	double B = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LinearColor)
	double A = 0;

	FVoxelDoubleLinearColor() = default;
	FORCEINLINE FVoxelDoubleLinearColor(
		const double R,
		const double G,
		const double B,
		const double A)
		: R(R)
		, G(G)
		, B(B)
		, A(A)
	{
	}
	FORCEINLINE FVoxelDoubleLinearColor(const FVector4& Vector)
		: R(Vector.X)
		, G(Vector.Y)
		, B(Vector.Z)
		, A(Vector.W)
	{
	}

	FORCEINLINE operator const FVector4&() const
	{
		return ReinterpretCastRef<FVector4>(*this);
	}

	FString ToString() const;
};

DECLARE_VOXEL_BUFFER(FVoxelDoubleLinearColorBuffer, FVoxelDoubleLinearColor);

USTRUCT()
struct VOXELGRAPH_API FVoxelDoubleLinearColorBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelDoubleLinearColorBuffer, FVoxelDoubleLinearColor);

	UPROPERTY()
	FVoxelDoubleBuffer R;

	UPROPERTY()
	FVoxelDoubleBuffer G;

	UPROPERTY()
	FVoxelDoubleBuffer B;

	UPROPERTY()
	FVoxelDoubleBuffer A;

	FORCEINLINE const FVoxelDoubleLinearColor operator[](const int32 Index) const
	{
		return FVoxelDoubleLinearColor(R[Index], G[Index], B[Index], A[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FVoxelDoubleLinearColor& Value)
	{
		R.Set(Index, Value.R);
		G.Set(Index, Value.G);
		B.Set(Index, Value.B);
		A.Set(Index, Value.A);
	}
};