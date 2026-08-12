// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "FunctionLibrary/VoxelMathFunctionLibrary.h"
#include "VoxelGraphMigration.h"
#include "Utilities/VoxelBufferMathUtilities.h"
#include "Utilities/VoxelBufferTransformUtilities.h"
#include "VoxelMathFunctionLibraryImpl.ispc.generated.h"

FVoxelTransformBuffer UVoxelMathFunctionLibrary::MakeTransform(
	const FVoxelVectorBuffer& Translation,
	const FVoxelQuaternionBuffer& Rotation,
	const FVoxelVectorBuffer& Scale) const
{
	CheckVoxelBuffersNum_Return(Translation, Rotation, Scale);

	FVoxelTransformBuffer Result;
	Result.Translation = Translation;
	Result.Rotation = Rotation;
	Result.Scale = Scale;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakTransform(
	const FVoxelTransformBuffer& Value,
	FVoxelVectorBuffer& Translation,
	FVoxelQuaternionBuffer& Rotation,
	FVoxelVectorBuffer& Scale) const
{
	Translation = Value.Translation;
	Rotation = Value.Rotation;
	Scale = Value.Scale;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVector2DBuffer UVoxelMathFunctionLibrary::MakeVector2D(
	const FVoxelFloatBuffer& X,
	const FVoxelFloatBuffer& Y) const
{
	CheckVoxelBuffersNum_Return(X, Y);

	FVoxelVector2DBuffer Result;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakVector2D(
	const FVoxelVector2DBuffer& Value,
	FVoxelFloatBuffer& X,
	FVoxelFloatBuffer& Y) const
{
	X = Value.X;
	Y = Value.Y;
}

FVoxelVectorBuffer UVoxelMathFunctionLibrary::MakeVector(
	const FVoxelFloatBuffer& X,
	const FVoxelFloatBuffer& Y,
	const FVoxelFloatBuffer& Z) const
{
	CheckVoxelBuffersNum_Return(X, Y, Z);

	FVoxelVectorBuffer Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakVector(
	const FVoxelVectorBuffer& Value,
	FVoxelFloatBuffer& X,
	FVoxelFloatBuffer& Y,
	FVoxelFloatBuffer& Z) const
{
	X = Value.X;
	Y = Value.Y;
	Z = Value.Z;
}

FVoxelLinearColorBuffer UVoxelMathFunctionLibrary::MakeLinearColor(
	const FVoxelFloatBuffer& R,
	const FVoxelFloatBuffer& G,
	const FVoxelFloatBuffer& B,
	const FVoxelFloatBuffer& A) const
{
	CheckVoxelBuffersNum_Return(R, G, B, A);

	FVoxelLinearColorBuffer Result;
	Result.R = R;
	Result.G = G;
	Result.B = B;
	Result.A = A;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakLinearColor(
	const FVoxelLinearColorBuffer& Value,
	FVoxelFloatBuffer& R,
	FVoxelFloatBuffer& G,
	FVoxelFloatBuffer& B,
	FVoxelFloatBuffer& A) const
{
	R = Value.R;
	G = Value.G;
	B = Value.B;
	A = Value.A;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelIntPointBuffer UVoxelMathFunctionLibrary::MakeIntPoint(
	const FVoxelInt32Buffer& X,
	const FVoxelInt32Buffer& Y) const
{
	CheckVoxelBuffersNum_Return(X, Y);

	FVoxelIntPointBuffer Result;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakIntPoint(
	const FVoxelIntPointBuffer& Value,
	FVoxelInt32Buffer& X,
	FVoxelInt32Buffer& Y) const
{
	X = Value.X;
	Y = Value.Y;
}

FVoxelIntVectorBuffer UVoxelMathFunctionLibrary::MakeIntVector(
	const FVoxelInt32Buffer& X,
	const FVoxelInt32Buffer& Y,
	const FVoxelInt32Buffer& Z) const
{
	CheckVoxelBuffersNum_Return(X, Y, Z);

	FVoxelIntVectorBuffer Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakIntVector(
	const FVoxelIntVectorBuffer& Value,
	FVoxelInt32Buffer& X,
	FVoxelInt32Buffer& Y,
	FVoxelInt32Buffer& Z) const
{
	X = Value.X;
	Y = Value.Y;
	Z = Value.Z;
}

FVoxelIntVector4Buffer UVoxelMathFunctionLibrary::MakeIntVector4(
	const FVoxelInt32Buffer& X,
	const FVoxelInt32Buffer& Y,
	const FVoxelInt32Buffer& Z,
	const FVoxelInt32Buffer& W) const
{
	CheckVoxelBuffersNum_Return(X, Y, Z, W);

	FVoxelIntVector4Buffer Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	Result.W = W;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakIntVector4(
	const FVoxelIntVector4Buffer& Value,
	FVoxelInt32Buffer& X,
	FVoxelInt32Buffer& Y,
	FVoxelInt32Buffer& Z,
	FVoxelInt32Buffer& W) const
{
	X = Value.X;
	Y = Value.Y;
	Z = Value.Z;
	W = Value.W;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelInt64PointBuffer UVoxelMathFunctionLibrary::MakeInt64Point(
	const FVoxelInt64Buffer& X,
	const FVoxelInt64Buffer& Y) const
{
	CheckVoxelBuffersNum_Return(X, Y);

	FVoxelInt64PointBuffer Result;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakInt64Point(
	const FVoxelInt64PointBuffer& Value,
	FVoxelInt64Buffer& X,
	FVoxelInt64Buffer& Y) const
{
	X = Value.X;
	Y = Value.Y;
}

FVoxelInt64VectorBuffer UVoxelMathFunctionLibrary::MakeInt64Vector(
	const FVoxelInt64Buffer& X,
	const FVoxelInt64Buffer& Y,
	const FVoxelInt64Buffer& Z) const
{
	CheckVoxelBuffersNum_Return(X, Y, Z);

	FVoxelInt64VectorBuffer Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakInt64Vector(
	const FVoxelInt64VectorBuffer& Value,
	FVoxelInt64Buffer& X,
	FVoxelInt64Buffer& Y,
	FVoxelInt64Buffer& Z) const
{
	X = Value.X;
	Y = Value.Y;
	Z = Value.Z;
}

FVoxelInt64Vector4Buffer UVoxelMathFunctionLibrary::MakeInt64Vector4(
	const FVoxelInt64Buffer& X,
	const FVoxelInt64Buffer& Y,
	const FVoxelInt64Buffer& Z,
	const FVoxelInt64Buffer& W) const
{
	CheckVoxelBuffersNum_Return(X, Y, Z, W);

	FVoxelInt64Vector4Buffer Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	Result.W = W;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakInt64Vector4(
	const FVoxelInt64Vector4Buffer& Value,
	FVoxelInt64Buffer& X,
	FVoxelInt64Buffer& Y,
	FVoxelInt64Buffer& Z,
	FVoxelInt64Buffer& W) const
{
	X = Value.X;
	Y = Value.Y;
	Z = Value.Z;
	W = Value.W;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDoubleVector2DBuffer UVoxelMathFunctionLibrary::MakeDoubleVector2D(
	const FVoxelDoubleBuffer& X,
	const FVoxelDoubleBuffer& Y) const
{
	CheckVoxelBuffersNum_Return(X, Y);

	FVoxelDoubleVector2DBuffer Result;
	Result.X = X;
	Result.Y = Y;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakDoubleVector2D(
	const FVoxelDoubleVector2DBuffer& Value,
	FVoxelDoubleBuffer& X,
	FVoxelDoubleBuffer& Y) const
{
	X = Value.X;
	Y = Value.Y;
}

FVoxelDoubleVectorBuffer UVoxelMathFunctionLibrary::MakeDoubleVector(
	const FVoxelDoubleBuffer& X,
	const FVoxelDoubleBuffer& Y,
	const FVoxelDoubleBuffer& Z) const
{
	CheckVoxelBuffersNum_Return(X, Y, Z);

	FVoxelDoubleVectorBuffer Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakDoubleVector(
	const FVoxelDoubleVectorBuffer& Value,
	FVoxelDoubleBuffer& X,
	FVoxelDoubleBuffer& Y,
	FVoxelDoubleBuffer& Z) const
{
	X = Value.X;
	Y = Value.Y;
	Z = Value.Z;
}

FVoxelDoubleLinearColorBuffer UVoxelMathFunctionLibrary::MakeDoubleLinearColor(
	const FVoxelDoubleBuffer& R,
	const FVoxelDoubleBuffer& G,
	const FVoxelDoubleBuffer& B,
	const FVoxelDoubleBuffer& A) const
{
	CheckVoxelBuffersNum_Return(R, G, B, A);

	FVoxelDoubleLinearColorBuffer Result;
	Result.R = R;
	Result.G = G;
	Result.B = B;
	Result.A = A;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakDoubleLinearColor(
	const FVoxelDoubleLinearColorBuffer& Value,
	FVoxelDoubleBuffer& R,
	FVoxelDoubleBuffer& G,
	FVoxelDoubleBuffer& B,
	FVoxelDoubleBuffer& A) const
{
	R = Value.R;
	G = Value.G;
	B = Value.B;
	A = Value.A;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatRangeBuffer UVoxelMathFunctionLibrary::MakeFloatRange(
	const FVoxelFloatBuffer& Min,
	const FVoxelFloatBuffer& Max) const
{
	CheckVoxelBuffersNum_Return(Min, Max);

	FVoxelFloatRangeBuffer Result;
	Result.Min = Min;
	Result.Max = Max;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakFloatRange(
	const FVoxelFloatRangeBuffer& Value,
	FVoxelFloatBuffer& Min,
	FVoxelFloatBuffer& Max) const
{
	Min = Value.Min;
	Max = Value.Max;
}

FVoxelInt32RangeBuffer UVoxelMathFunctionLibrary::MakeInt32Range(
	const FVoxelInt32Buffer& Min,
	const FVoxelInt32Buffer& Max) const
{
	CheckVoxelBuffersNum_Return(Min, Max);

	FVoxelInt32RangeBuffer Result;
	Result.Min = Min;
	Result.Max = Max;
	return Result;
}

void UVoxelMathFunctionLibrary::BreakInt32Range(
	const FVoxelInt32RangeBuffer& Value,
	FVoxelInt32Buffer& Min,
	FVoxelInt32Buffer& Max) const
{
	Min = Value.Min;
	Max = Value.Max;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelNormalBuffer UVoxelMathFunctionLibrary::MakeNormal(
	const FVoxelFloatBuffer& X,
	const FVoxelFloatBuffer& Y,
	const FVoxelFloatBuffer& Z) const
{
	const int32 Num = ComputeVoxelBuffersNum_Return(X, Y, Z);

	FVoxelNormalBuffer Result;
	Result.Allocate(Num);

	ispc::VoxelMathFunctionLibrary_MakeNormal(
		X.GetData(),
		X.IsConstant(),
		Y.GetData(),
		Y.IsConstant(),
		Z.GetData(),
		Z.IsConstant(),
		Result.GetData(),
		Num);

	return Result;
}

void UVoxelMathFunctionLibrary::BreakNormal(
	const FVoxelNormalBuffer& Value,
	FVoxelFloatBuffer& X,
	FVoxelFloatBuffer& Y,
	FVoxelFloatBuffer& Z) const
{
	X.Allocate(Value.Num());
	Y.Allocate(Value.Num());
	Z.Allocate(Value.Num());

	ispc::VoxelMathFunctionLibrary_BreakNormal(
		Value.GetData(),
		X.GetData(),
		Y.GetData(),
		Z.GetData(),
		Value.Num());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelMathFunctionLibrary::SmoothMin(
	FVoxelFloatBuffer& OutDistance,
	FVoxelFloatBuffer& OutAlpha,
	const FVoxelFloatBuffer& A,
	const FVoxelFloatBuffer& B,
	const FVoxelFloatBuffer& Smoothness) const
{
	const int32 Num = ComputeVoxelBuffersNum(A, B, Smoothness);

	OutDistance.Allocate(Num);
	OutAlpha.Allocate(Num);

	ispc::VoxelMathFunctionLibrary_SmoothMin(
		A.GetData(),
		A.IsConstant(),
		B.GetData(),
		B.IsConstant(),
		Smoothness.GetData(),
		Smoothness.IsConstant(),
		Num,
		OutAlpha.GetData(),
		OutDistance.GetData());
}

void UVoxelMathFunctionLibrary::SmoothMax(
	FVoxelFloatBuffer& OutDistance,
	FVoxelFloatBuffer& OutAlpha,
	const FVoxelFloatBuffer& A,
	const FVoxelFloatBuffer& B,
	const FVoxelFloatBuffer& Smoothness) const
{
	const int32 Num = ComputeVoxelBuffersNum(A, B, Smoothness);

	OutDistance.Allocate(Num);
	OutAlpha.Allocate(Num);

	ispc::VoxelMathFunctionLibrary_SmoothMax(
		A.GetData(),
		A.IsConstant(),
		B.GetData(),
		B.IsConstant(),
		Smoothness.GetData(),
		Smoothness.IsConstant(),
		Num,
		OutAlpha.GetData(),
		OutDistance.GetData());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelMathFunctionLibrary::SmoothUnion(
	FVoxelFloatBuffer& OutDistance,
	FVoxelFloatBuffer& OutAlpha,
	const FVoxelFloatBuffer& A,
	const FVoxelFloatBuffer& B,
	const FVoxelFloatBuffer& Smoothness) const
{
	return SmoothMin(OutDistance, OutAlpha, A, B, Smoothness);
}

void UVoxelMathFunctionLibrary::SmoothIntersection(
	FVoxelFloatBuffer& OutDistance,
	FVoxelFloatBuffer& OutAlpha,
	const FVoxelFloatBuffer& A,
	const FVoxelFloatBuffer& B,
	const FVoxelFloatBuffer& Smoothness) const
{
	return SmoothMax(OutDistance, OutAlpha, A, B, Smoothness);
}

void UVoxelMathFunctionLibrary::SmoothSubtraction(
	FVoxelFloatBuffer& OutDistance,
	FVoxelFloatBuffer& OutAlpha,
	const FVoxelFloatBuffer& A,
	const FVoxelFloatBuffer& B,
	const FVoxelFloatBuffer& Smoothness) const
{
	const int32 Num = ComputeVoxelBuffersNum(A, B, Smoothness);

	OutDistance.Allocate(Num);
	OutAlpha.Allocate(Num);

	ispc::VoxelMathFunctionLibrary_SmoothSubtraction(
		A.GetData(),
		A.IsConstant(),
		B.GetData(),
		B.IsConstant(),
		Smoothness.GetData(),
		Smoothness.IsConstant(),
		Num,
		OutAlpha.GetData(),
		OutDistance.GetData());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelQuaternionBuffer UVoxelMathFunctionLibrary::CombineRotation(
	const FVoxelQuaternionBuffer& A,
	const FVoxelQuaternionBuffer& B) const
{
	CheckVoxelBuffersNum_Return(A, B);
	return FVoxelBufferMathUtilities::Combine(A, B);
}

FVoxelQuaternionBuffer UVoxelMathFunctionLibrary::LerpRotation(
	const FVoxelQuaternionBuffer& A,
	const FVoxelQuaternionBuffer& B,
	const FVoxelFloatBuffer& Alpha) const
{
	CheckVoxelBuffersNum_Return(A, B, Alpha);
	return FVoxelBufferMathUtilities::Lerp(A, B, Alpha);
}

FVoxelVectorBuffer UVoxelMathFunctionLibrary::RotateVector(
	const FVoxelVectorBuffer& A,
	const FVoxelQuaternionBuffer& B) const
{
	CheckVoxelBuffersNum_Return(B, A);
	return FVoxelBufferMathUtilities::RotateVector(B, A);
}

FVoxelVectorBuffer UVoxelMathFunctionLibrary::UnrotateVector(
	const FVoxelVectorBuffer& A,
	const FVoxelQuaternionBuffer& B) const
{
	CheckVoxelBuffersNum_Return(B, A);
	return FVoxelBufferMathUtilities::UnrotateVector(B, A);
}

FVoxelVectorBuffer UVoxelMathFunctionLibrary::GetUpVector(const FVoxelQuaternionBuffer& Q) const
{
	return FVoxelBufferMathUtilities::RotateVector(Q, FVector3f::UpVector);
}

FVoxelVectorBuffer UVoxelMathFunctionLibrary::GetForwardVector(const FVoxelQuaternionBuffer& Q) const
{
	return FVoxelBufferMathUtilities::RotateVector(Q, FVector3f::ForwardVector);
}

FVoxelVectorBuffer UVoxelMathFunctionLibrary::GetRightVector(const FVoxelQuaternionBuffer& Q) const
{
	return FVoxelBufferMathUtilities::RotateVector(Q, FVector3f::RightVector);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVector2DBuffer UVoxelMathFunctionLibrary::RotateVector2D(
	const FVoxelVector2DBuffer& A,
	const FVoxelFloatBuffer& AngleDeg) const
{
	return FVoxelBufferMathUtilities::RotateVector2D(A, AngleDeg);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FTransform UVoxelMathFunctionLibrary::CombineTransform(
	const FTransform& A,
	const FTransform& B) const
{
	return A * B;
}

FVoxelVectorBuffer UVoxelMathFunctionLibrary::TransformLocation(
	const FVoxelVectorBuffer& Location,
	const FVoxelTransformBuffer& Transform) const
{
	return FVoxelBufferTransformUtilities::ApplyTransform(Location, Transform);
}

FVoxelVectorBuffer UVoxelMathFunctionLibrary::InverseTransformLocation(
	const FVoxelVectorBuffer& Location,
	const FVoxelTransformBuffer& Transform) const
{
	return FVoxelBufferTransformUtilities::ApplyInverseTransform(Location, Transform);
}