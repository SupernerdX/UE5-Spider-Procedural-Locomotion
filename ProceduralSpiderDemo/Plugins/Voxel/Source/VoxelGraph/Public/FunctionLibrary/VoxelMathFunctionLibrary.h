// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFunctionLibrary.h"
#include "Buffer/VoxelRangeBuffers.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelNormalBuffer.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Buffer/VoxelIntegerBuffers.h"
#include "VoxelMathFunctionLibrary.generated.h"

UCLASS()
class VOXELGRAPH_API UVoxelMathFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Math|Transform", meta = (NativeMakeFunc))
	FVoxelTransformBuffer MakeTransform(
		const FVoxelVectorBuffer& Translation,
		const FVoxelQuaternionBuffer& Rotation,
		const FVoxelVectorBuffer& Scale) const;

	UFUNCTION(Category = "Math|Transform", meta = (NativeBreakFunc))
	void BreakTransform(
		const FVoxelTransformBuffer& Value,
		FVoxelVectorBuffer& Translation,
		FVoxelQuaternionBuffer& Rotation,
		FVoxelVectorBuffer& Scale) const;

public:
	UFUNCTION(Category = "Math|Vector2D", meta = (NativeMakeFunc))
	FVoxelVector2DBuffer MakeVector2D(
		const FVoxelFloatBuffer& X,
		const FVoxelFloatBuffer& Y) const;

	UFUNCTION(Category = "Math|Vector2D", meta = (NativeBreakFunc))
	void BreakVector2D(
		const FVoxelVector2DBuffer& Value,
		FVoxelFloatBuffer& X,
		FVoxelFloatBuffer& Y) const;

	UFUNCTION(Category = "Math|Vector", meta = (NativeMakeFunc))
	FVoxelVectorBuffer MakeVector(
		const FVoxelFloatBuffer& X,
		const FVoxelFloatBuffer& Y,
		const FVoxelFloatBuffer& Z) const;

	UFUNCTION(Category = "Math|Vector", meta = (NativeBreakFunc))
	void BreakVector(
		const FVoxelVectorBuffer& Value,
		FVoxelFloatBuffer& X,
		FVoxelFloatBuffer& Y,
		FVoxelFloatBuffer& Z) const;

	UFUNCTION(Category = "Math|Color", meta = (NativeMakeFunc))
	FVoxelLinearColorBuffer MakeLinearColor(
		const FVoxelFloatBuffer& R,
		const FVoxelFloatBuffer& G,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& A) const;

	UFUNCTION(Category = "Math|Color", meta = (NativeBreakFunc))
	void BreakLinearColor(
		const FVoxelLinearColorBuffer& Value,
		FVoxelFloatBuffer& R,
		FVoxelFloatBuffer& G,
		FVoxelFloatBuffer& B,
		FVoxelFloatBuffer& A) const;

public:
	UFUNCTION(Category = "Math|Int Point", meta = (NativeMakeFunc))
	FVoxelIntPointBuffer MakeIntPoint(
		const FVoxelInt32Buffer& X,
		const FVoxelInt32Buffer& Y) const;

	UFUNCTION(Category = "Math|Int Point", meta = (NativeBreakFunc))
	void BreakIntPoint(
		const FVoxelIntPointBuffer& Value,
		FVoxelInt32Buffer& X,
		FVoxelInt32Buffer& Y) const;

	UFUNCTION(Category = "Math|Int Vector", meta = (NativeMakeFunc))
	FVoxelIntVectorBuffer MakeIntVector(
		const FVoxelInt32Buffer& X,
		const FVoxelInt32Buffer& Y,
		const FVoxelInt32Buffer& Z) const;

	UFUNCTION(Category = "Math|Int Vector", meta = (NativeBreakFunc))
	void BreakIntVector(
		const FVoxelIntVectorBuffer& Value,
		FVoxelInt32Buffer& X,
		FVoxelInt32Buffer& Y,
		FVoxelInt32Buffer& Z) const;

	UFUNCTION(Category = "Math|Int Vector 4", meta = (NativeMakeFunc))
	FVoxelIntVector4Buffer MakeIntVector4(
		const FVoxelInt32Buffer& X,
		const FVoxelInt32Buffer& Y,
		const FVoxelInt32Buffer& Z,
		const FVoxelInt32Buffer& W) const;

	UFUNCTION(Category = "Math|Int Vector 4", meta = (NativeBreakFunc))
	void BreakIntVector4(
		const FVoxelIntVector4Buffer& Value,
		FVoxelInt32Buffer& X,
		FVoxelInt32Buffer& Y,
		FVoxelInt32Buffer& Z,
		FVoxelInt32Buffer& W) const;

public:
	UFUNCTION(Category = "Math|Int64 Point", meta = (NativeMakeFunc))
	FVoxelInt64PointBuffer MakeInt64Point(
		const FVoxelInt64Buffer& X,
		const FVoxelInt64Buffer& Y) const;

	UFUNCTION(Category = "Math|Int64 Point", meta = (NativeBreakFunc))
	void BreakInt64Point(
		const FVoxelInt64PointBuffer& Value,
		FVoxelInt64Buffer& X,
		FVoxelInt64Buffer& Y) const;

	UFUNCTION(Category = "Math|Int64 Vector", meta = (NativeMakeFunc))
	FVoxelInt64VectorBuffer MakeInt64Vector(
		const FVoxelInt64Buffer& X,
		const FVoxelInt64Buffer& Y,
		const FVoxelInt64Buffer& Z) const;

	UFUNCTION(Category = "Math|Int64 Vector", meta = (NativeBreakFunc))
	void BreakInt64Vector(
		const FVoxelInt64VectorBuffer& Value,
		FVoxelInt64Buffer& X,
		FVoxelInt64Buffer& Y,
		FVoxelInt64Buffer& Z) const;

	UFUNCTION(Category = "Math|Int64 Vector 4", meta = (NativeMakeFunc))
	FVoxelInt64Vector4Buffer MakeInt64Vector4(
		const FVoxelInt64Buffer& X,
		const FVoxelInt64Buffer& Y,
		const FVoxelInt64Buffer& Z,
		const FVoxelInt64Buffer& W) const;

	UFUNCTION(Category = "Math|Int64 Vector 4", meta = (NativeBreakFunc))
	void BreakInt64Vector4(
		const FVoxelInt64Vector4Buffer& Value,
		FVoxelInt64Buffer& X,
		FVoxelInt64Buffer& Y,
		FVoxelInt64Buffer& Z,
		FVoxelInt64Buffer& W) const;

public:
	UFUNCTION(Category = "Math|Vector2D", meta = (NativeMakeFunc))
	FVoxelDoubleVector2DBuffer MakeDoubleVector2D(
		const FVoxelDoubleBuffer& X,
		const FVoxelDoubleBuffer& Y) const;

	UFUNCTION(Category = "Math|Vector2D", meta = (NativeBreakFunc))
	void BreakDoubleVector2D(
		const FVoxelDoubleVector2DBuffer& Value,
		FVoxelDoubleBuffer& X,
		FVoxelDoubleBuffer& Y) const;

	UFUNCTION(Category = "Math|Vector", meta = (NativeMakeFunc))
	FVoxelDoubleVectorBuffer MakeDoubleVector(
		const FVoxelDoubleBuffer& X,
		const FVoxelDoubleBuffer& Y,
		const FVoxelDoubleBuffer& Z) const;

	UFUNCTION(Category = "Math|Vector", meta = (NativeBreakFunc))
	void BreakDoubleVector(
		const FVoxelDoubleVectorBuffer& Value,
		FVoxelDoubleBuffer& X,
		FVoxelDoubleBuffer& Y,
		FVoxelDoubleBuffer& Z) const;

	UFUNCTION(Category = "Math|Color", meta = (NativeMakeFunc))
	FVoxelDoubleLinearColorBuffer MakeDoubleLinearColor(
		const FVoxelDoubleBuffer& R,
		const FVoxelDoubleBuffer& G,
		const FVoxelDoubleBuffer& B,
		const FVoxelDoubleBuffer& A) const;

	UFUNCTION(Category = "Math|Color", meta = (NativeBreakFunc))
	void BreakDoubleLinearColor(
		const FVoxelDoubleLinearColorBuffer& Value,
		FVoxelDoubleBuffer& R,
		FVoxelDoubleBuffer& G,
		FVoxelDoubleBuffer& B,
		FVoxelDoubleBuffer& A) const;

public:
	UFUNCTION(Category = "Math|Interval", meta = (NativeMakeFunc))
	FVoxelFloatRangeBuffer MakeFloatRange(
		const FVoxelFloatBuffer& Min,
		const FVoxelFloatBuffer& Max) const;

	UFUNCTION(Category = "Math|Interval", meta = (NativeBreakFunc))
	void BreakFloatRange(
		const FVoxelFloatRangeBuffer& Value,
		FVoxelFloatBuffer& Min,
		FVoxelFloatBuffer& Max) const;

	UFUNCTION(Category = "Math|Interval", meta = (NativeMakeFunc, DisplayName = "Make Integer Range"))
	FVoxelInt32RangeBuffer MakeInt32Range(
		const FVoxelInt32Buffer& Min,
		const FVoxelInt32Buffer& Max) const;

	UFUNCTION(Category = "Math|Interval", meta = (NativeBreakFunc, DisplayName = "Break Integer Range"))
	void BreakInt32Range(
		const FVoxelInt32RangeBuffer& Value,
		FVoxelInt32Buffer& Min,
		FVoxelInt32Buffer& Max) const;

public:
	UFUNCTION(Category = "Math|Normal", meta = (NativeMakeFunc))
	FVoxelNormalBuffer MakeNormal(
		const FVoxelFloatBuffer& X,
		const FVoxelFloatBuffer& Y,
		const FVoxelFloatBuffer& Z) const;

	UFUNCTION(Category = "Math|Normal", meta = (NativeBreakFunc))
	void BreakNormal(
		const FVoxelNormalBuffer& Value,
		FVoxelFloatBuffer& X,
		FVoxelFloatBuffer& Y,
		FVoxelFloatBuffer& Z) const;

public:
	UFUNCTION(Category = "Math|Distance")
	void SmoothMin(
		FVoxelFloatBuffer& OutDistance,
		FVoxelFloatBuffer& OutAlpha,
		const FVoxelFloatBuffer& A,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& Smoothness = 100.f) const;

	UFUNCTION(Category = "Math|Distance")
	void SmoothMax(
		FVoxelFloatBuffer& OutDistance,
		FVoxelFloatBuffer& OutAlpha,
		const FVoxelFloatBuffer& A,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& Smoothness = 100.f) const;

public:
	UFUNCTION(Category = "Math|Distance")
	void SmoothUnion(
		FVoxelFloatBuffer& OutDistance,
		FVoxelFloatBuffer& OutAlpha,
		const FVoxelFloatBuffer& A,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& Smoothness = 100.f) const;

	UFUNCTION(Category = "Math|Distance")
	void SmoothIntersection(
		FVoxelFloatBuffer& OutDistance,
		FVoxelFloatBuffer& OutAlpha,
		const FVoxelFloatBuffer& A,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& Smoothness = 100.f) const;

	UFUNCTION(Category = "Math|Distance")
	void SmoothSubtraction(
		FVoxelFloatBuffer& OutDistance,
		FVoxelFloatBuffer& OutAlpha,
		const FVoxelFloatBuffer& A,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& Smoothness = 100.f) const;

public:
	UFUNCTION(Category = "Math|Rotation")
	FVoxelQuaternionBuffer CombineRotation(
		const FVoxelQuaternionBuffer& A,
		const FVoxelQuaternionBuffer& B) const;

	// Spherical interpolation between Rotations. Result is normalized
	UFUNCTION(Category = "Math|Rotation")
	FVoxelQuaternionBuffer LerpRotation(
		const FVoxelQuaternionBuffer& A,
		const FVoxelQuaternionBuffer& B,
		const FVoxelFloatBuffer& Alpha) const;

	// Returns result of vector A rotated by Rotator B
	UFUNCTION(Category = "Math|Rotation")
	FVoxelVectorBuffer RotateVector(
		const FVoxelVectorBuffer& A,
		const FVoxelQuaternionBuffer& B) const;

	// Returns result of vector A rotated by the inverse of Rotator B
	UFUNCTION(Category = "Math|Rotation")
	FVoxelVectorBuffer UnrotateVector(
		const FVoxelVectorBuffer& A,
		const FVoxelQuaternionBuffer& B) const;

	UFUNCTION(Category = "Math|Rotation")
	FVoxelVectorBuffer GetUpVector(const FVoxelQuaternionBuffer& Q) const;

	UFUNCTION(Category = "Math|Rotation")
	FVoxelVectorBuffer GetForwardVector(const FVoxelQuaternionBuffer& Q) const;

	UFUNCTION(Category = "Math|Rotation")
	FVoxelVectorBuffer GetRightVector(const FVoxelQuaternionBuffer& Q) const;

public:
	// Rotates vector by angle in degrees
	UFUNCTION(Category = "Math|Rotation")
	FVoxelVector2DBuffer RotateVector2D(
		const FVoxelVector2DBuffer& A,
		const FVoxelFloatBuffer& AngleDeg) const;

public:
	UFUNCTION(Category = "Math|Transform", meta = (Keywords = "* mul"))
	FTransform CombineTransform(
		const FTransform& A,
		const FTransform& B) const;

	// Transform a position by the supplied transform.
	// For example, if Transform was an object's transform, this would transform a position from local space to world space.
	UFUNCTION(Category = "Math|Transform")
	FVoxelVectorBuffer TransformLocation(
		const FVoxelVectorBuffer& Location,
		const FVoxelTransformBuffer& Transform) const;

	// Transform a position by the inverse of the supplied transform.
	// For example, if Transform was an object's transform, this would transform a position from world space to local space.
	UFUNCTION(Category = "Math|Transform")
	FVoxelVectorBuffer InverseTransformLocation(
		const FVoxelVectorBuffer& Location,
		const FVoxelTransformBuffer& Transform) const;

public:
	// Use this as height/distance to disable a stamp output
	UFUNCTION(Category = "Math|Float", DisplayName = "Make Void")
	FVoxelFloatBuffer MakeVoid() const
	{
		return FVoxelUtilities::NaNf();
	}
};