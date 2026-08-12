// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Utilities/VoxelBufferMathUtilities.h"
#include "VoxelBufferAccessor.h"
#include "VoxelBufferMathUtilitiesImpl.ispc.generated.h"

FVoxelFloatBuffer FVoxelBufferMathUtilities::Min(
	const FVoxelFloatBuffer& Buffer0,
	const FVoxelFloatBuffer& Buffer1)
{
	const FVoxelBufferAccessor BufferAccessor(
		Buffer0,
		Buffer1);

	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelFloatBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Min(
		Buffer0.GetData(), Buffer0.IsConstant(),
		Buffer1.GetData(), Buffer1.IsConstant(),
		Result.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferMathUtilities::Max(
	const FVoxelFloatBuffer& Buffer0,
	const FVoxelFloatBuffer& Buffer1)
{
	const FVoxelBufferAccessor BufferAccessor(
		Buffer0,
		Buffer1);

	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelFloatBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Max(
		Buffer0.GetData(), Buffer0.IsConstant(),
		Buffer1.GetData(), Buffer1.IsConstant(),
		Result.GetData(),
		BufferAccessor.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer FVoxelBufferMathUtilities::Min8(
	const FVoxelFloatBuffer& Buffer0,
	const FVoxelFloatBuffer& Buffer1,
	const FVoxelFloatBuffer& Buffer2,
	const FVoxelFloatBuffer& Buffer3,
	const FVoxelFloatBuffer& Buffer4,
	const FVoxelFloatBuffer& Buffer5,
	const FVoxelFloatBuffer& Buffer6,
	const FVoxelFloatBuffer& Buffer7)
{
	const FVoxelBufferAccessor BufferAccessor(
		Buffer0,
		Buffer1,
		Buffer2,
		Buffer3,
		Buffer4,
		Buffer5,
		Buffer6,
		Buffer7);

	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelFloatBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Min8(
		Buffer0.GetData(), Buffer0.IsConstant(),
		Buffer1.GetData(), Buffer1.IsConstant(),
		Buffer2.GetData(), Buffer2.IsConstant(),
		Buffer3.GetData(), Buffer3.IsConstant(),
		Buffer4.GetData(), Buffer4.IsConstant(),
		Buffer5.GetData(), Buffer5.IsConstant(),
		Buffer6.GetData(), Buffer6.IsConstant(),
		Buffer7.GetData(), Buffer7.IsConstant(),
		Result.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferMathUtilities::Max8(
	const FVoxelFloatBuffer& Buffer0,
	const FVoxelFloatBuffer& Buffer1,
	const FVoxelFloatBuffer& Buffer2,
	const FVoxelFloatBuffer& Buffer3,
	const FVoxelFloatBuffer& Buffer4,
	const FVoxelFloatBuffer& Buffer5,
	const FVoxelFloatBuffer& Buffer6,
	const FVoxelFloatBuffer& Buffer7)
{
	const FVoxelBufferAccessor BufferAccessor(
		Buffer0,
		Buffer1,
		Buffer2,
		Buffer3,
		Buffer4,
		Buffer5,
		Buffer6,
		Buffer7);

	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelFloatBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Max8(
		Buffer0.GetData(), Buffer0.IsConstant(),
		Buffer1.GetData(), Buffer1.IsConstant(),
		Buffer2.GetData(), Buffer2.IsConstant(),
		Buffer3.GetData(), Buffer3.IsConstant(),
		Buffer4.GetData(), Buffer4.IsConstant(),
		Buffer5.GetData(), Buffer5.IsConstant(),
		Buffer6.GetData(), Buffer6.IsConstant(),
		Buffer7.GetData(), Buffer7.IsConstant(),
		Result.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelVectorBuffer FVoxelBufferMathUtilities::Min8(
	const FVoxelVectorBuffer& Buffer0,
	const FVoxelVectorBuffer& Buffer1,
	const FVoxelVectorBuffer& Buffer2,
	const FVoxelVectorBuffer& Buffer3,
	const FVoxelVectorBuffer& Buffer4,
	const FVoxelVectorBuffer& Buffer5,
	const FVoxelVectorBuffer& Buffer6,
	const FVoxelVectorBuffer& Buffer7)
{
	FVoxelVectorBuffer Result;
	Result.X = Min8(Buffer0.X, Buffer1.X, Buffer2.X, Buffer3.X, Buffer4.X, Buffer5.X, Buffer6.X, Buffer7.X);
	Result.Y = Min8(Buffer0.Y, Buffer1.Y, Buffer2.Y, Buffer3.Y, Buffer4.Y, Buffer5.Y, Buffer6.Y, Buffer7.Y);
	Result.Z = Min8(Buffer0.Z, Buffer1.Z, Buffer2.Z, Buffer3.Z, Buffer4.Z, Buffer5.Z, Buffer6.Z, Buffer7.Z);
	return Result;
}

FVoxelVectorBuffer FVoxelBufferMathUtilities::Max8(
	const FVoxelVectorBuffer& Buffer0,
	const FVoxelVectorBuffer& Buffer1,
	const FVoxelVectorBuffer& Buffer2,
	const FVoxelVectorBuffer& Buffer3,
	const FVoxelVectorBuffer& Buffer4,
	const FVoxelVectorBuffer& Buffer5,
	const FVoxelVectorBuffer& Buffer6,
	const FVoxelVectorBuffer& Buffer7)
{
	FVoxelVectorBuffer Result;
	Result.X = Max8(Buffer0.X, Buffer1.X, Buffer2.X, Buffer3.X, Buffer4.X, Buffer5.X, Buffer6.X, Buffer7.X);
	Result.Y = Max8(Buffer0.Y, Buffer1.Y, Buffer2.Y, Buffer3.Y, Buffer4.Y, Buffer5.Y, Buffer6.Y, Buffer7.Y);
	Result.Z = Max8(Buffer0.Z, Buffer1.Z, Buffer2.Z, Buffer3.Z, Buffer4.Z, Buffer5.Z, Buffer6.Z, Buffer7.Z);
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer FVoxelBufferMathUtilities::Add(const FVoxelFloatBuffer& A, const FVoxelFloatBuffer& B)
{
	const FVoxelBufferAccessor BufferAccessor(A, B);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelFloatBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Add_Float(
		A.GetData(),
		A.IsConstant(),
		B.GetData(),
		B.IsConstant(),
		Result.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelDoubleBuffer FVoxelBufferMathUtilities::Add(const FVoxelDoubleBuffer& A, const FVoxelDoubleBuffer& B)
{
	const FVoxelBufferAccessor BufferAccessor(A, B);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelDoubleBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelDoubleBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Add_Double(
		A.GetData(),
		A.IsConstant(),
		B.GetData(),
		B.IsConstant(),
		Result.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferMathUtilities::Multiply(const FVoxelFloatBuffer& A, const FVoxelFloatBuffer& B)
{
	const FVoxelBufferAccessor BufferAccessor(A, B);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelFloatBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Multiply(
		A.GetData(),
		A.IsConstant(),
		B.GetData(),
		B.IsConstant(),
		Result.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelBoolBuffer FVoxelBufferMathUtilities::Less(const FVoxelFloatBuffer& A, const FVoxelFloatBuffer& B)
{
	const FVoxelBufferAccessor BufferAccessor(A, B);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelBoolBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelBoolBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Less(
		A.GetData(),
		A.IsConstant(),
		B.GetData(),
		B.IsConstant(),
		ReinterpretCastPtr<uint8>(Result.GetData()),
		BufferAccessor.Num());

	return Result;
}

FVoxelBoolBuffer FVoxelBufferMathUtilities::IsFinite(const FVoxelFloatBuffer& Values)
{
	VOXEL_FUNCTION_COUNTER_NUM(Values.Num());

	FVoxelBoolBuffer Result;
	Result.Allocate(Values.Num());

	ispc::VoxelBufferMathUtilities_IsFinite(
		Values.GetData(),
		ReinterpretCastPtr<uint8>(Result.GetData()),
		Values.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVectorBuffer FVoxelBufferMathUtilities::Add(const FVoxelVectorBuffer& A, const FVoxelVectorBuffer& B)
{
	FVoxelVectorBuffer Result;
	Result.X = Add(A.X, B.X);
	Result.Y = Add(A.Y, B.Y);
	Result.Z = Add(A.Z, B.Z);
	return Result;
}

FVoxelDoubleVectorBuffer FVoxelBufferMathUtilities::Add(const FVoxelDoubleVectorBuffer& A, const FVoxelDoubleVectorBuffer& B)
{
	FVoxelDoubleVectorBuffer Result;
	Result.X = Add(A.X, B.X);
	Result.Y = Add(A.Y, B.Y);
	Result.Z = Add(A.Z, B.Z);
	return Result;
}

FVoxelVectorBuffer FVoxelBufferMathUtilities::Multiply(const FVoxelVectorBuffer& A, const FVoxelVectorBuffer& B)
{
	FVoxelVectorBuffer Result;
	Result.X = Multiply(A.X, B.X);
	Result.Y = Multiply(A.Y, B.Y);
	Result.Z = Multiply(A.Z, B.Z);
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer FVoxelBufferMathUtilities::Lerp(const FVoxelFloatBuffer& A, const FVoxelFloatBuffer& B, const FVoxelFloatBuffer& Alpha)
{
	const FVoxelBufferAccessor BufferAccessor(A, B, Alpha);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelFloatBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Alpha(
		A.GetData(),
		A.IsConstant(),
		B.GetData(),
		B.IsConstant(),
		Alpha.GetData(),
		Alpha.IsConstant(),
		Result.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelVector2DBuffer FVoxelBufferMathUtilities::Lerp(const FVoxelVector2DBuffer& A, const FVoxelVector2DBuffer& B, const FVoxelFloatBuffer& Alpha)
{
	FVoxelVector2DBuffer Result;
	Result.X = Lerp(A.X, B.X, Alpha);
	Result.Y = Lerp(A.Y, B.Y, Alpha);
	return Result;
}

FVoxelVectorBuffer FVoxelBufferMathUtilities::Lerp(const FVoxelVectorBuffer& A, const FVoxelVectorBuffer& B, const FVoxelFloatBuffer& Alpha)
{
	FVoxelVectorBuffer Result;
	Result.X = Lerp(A.X, B.X, Alpha);
	Result.Y = Lerp(A.Y, B.Y, Alpha);
	Result.Z = Lerp(A.Z, B.Z, Alpha);
	return Result;
}

FVoxelLinearColorBuffer FVoxelBufferMathUtilities::Lerp(const FVoxelLinearColorBuffer& A, const FVoxelLinearColorBuffer& B, const FVoxelFloatBuffer& Alpha)
{
	FVoxelLinearColorBuffer Result;
	Result.R = Lerp(A.R, B.R, Alpha);
	Result.G = Lerp(A.G, B.G, Alpha);
	Result.B = Lerp(A.B, B.B, Alpha);
	Result.A = Lerp(A.A, B.A, Alpha);
	return Result;
}

FVoxelQuaternionBuffer FVoxelBufferMathUtilities::Lerp(const FVoxelQuaternionBuffer& A, const FVoxelQuaternionBuffer& B, const FVoxelFloatBuffer& Alpha)
{
	const FVoxelBufferAccessor BufferAccessor(A, B, Alpha);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelQuaternionBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num(), 1024);

	FVoxelQuaternionBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_LerpRotation(
		A.X.GetData(),
		A.X.IsConstant(),
		A.Y.GetData(),
		A.Y.IsConstant(),
		A.Z.GetData(),
		A.Z.IsConstant(),
		A.W.GetData(),
		A.W.IsConstant(),
		B.X.GetData(),
		B.X.IsConstant(),
		B.Y.GetData(),
		B.Y.IsConstant(),
		B.Z.GetData(),
		B.Z.IsConstant(),
		B.W.GetData(),
		B.W.IsConstant(),
		Alpha.GetData(),
		Alpha.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData(),
		Result.W.GetData(),
		BufferAccessor.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelQuaternionBuffer FVoxelBufferMathUtilities::Combine(const FVoxelQuaternionBuffer& A, const FVoxelQuaternionBuffer& B)
{
	const FVoxelBufferAccessor BufferAccessor(A, B);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelQuaternionBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(BufferAccessor.Num());

	FVoxelQuaternionBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_Combine(
		A.X.GetData(),
		A.X.IsConstant(),
		A.Y.GetData(),
		A.Y.IsConstant(),
		A.Z.GetData(),
		A.Z.IsConstant(),
		A.W.GetData(),
		A.W.IsConstant(),
		B.X.GetData(),
		B.X.IsConstant(),
		B.Y.GetData(),
		B.Y.IsConstant(),
		B.Z.GetData(),
		B.Z.IsConstant(),
		B.W.GetData(),
		B.W.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData(),
		Result.W.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelVectorBuffer FVoxelBufferMathUtilities::RotateVector(const FVoxelQuaternionBuffer& Quaternion, const FVoxelVectorBuffer& Vector)
{
	const FVoxelBufferAccessor BufferAccessor(Quaternion, Vector);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelVectorBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(Quaternion.Num());

	FVoxelVectorBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_RotateVector(
		Quaternion.X.GetData(),
		Quaternion.X.IsConstant(),
		Quaternion.Y.GetData(),
		Quaternion.Y.IsConstant(),
		Quaternion.Z.GetData(),
		Quaternion.Z.IsConstant(),
		Quaternion.W.GetData(),
		Quaternion.W.IsConstant(),
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Vector.Z.GetData(),
		Vector.Z.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelVectorBuffer FVoxelBufferMathUtilities::UnrotateVector(const FVoxelQuaternionBuffer& Quaternion, const FVoxelVectorBuffer& Vector)
{
	const FVoxelBufferAccessor BufferAccessor(Quaternion, Vector);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelVectorBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(Quaternion.Num());

	FVoxelVectorBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_UnrotateVector(
		Quaternion.X.GetData(),
		Quaternion.X.IsConstant(),
		Quaternion.Y.GetData(),
		Quaternion.Y.IsConstant(),
		Quaternion.Z.GetData(),
		Quaternion.Z.IsConstant(),
		Quaternion.W.GetData(),
		Quaternion.W.IsConstant(),
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Vector.Z.GetData(),
		Vector.Z.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData(),
		BufferAccessor.Num());

	return Result;
}

FVoxelVectorBuffer FVoxelBufferMathUtilities::MakeEulerFromQuaternion(const FVoxelQuaternionBuffer& Quaternion)
{
	VOXEL_FUNCTION_COUNTER_NUM(Quaternion.Num());

	FVoxelVectorBuffer Result;
	Result.Allocate(Quaternion.Num());

	ispc::VoxelBufferMathUtilities_MakeEulerFromQuaternion(
		Quaternion.X.GetData(),
		Quaternion.X.IsConstant(),
		Quaternion.Y.GetData(),
		Quaternion.Y.IsConstant(),
		Quaternion.Z.GetData(),
		Quaternion.Z.IsConstant(),
		Quaternion.W.GetData(),
		Quaternion.W.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData(),
		Quaternion.Num());

	return Result;
}

FVoxelQuaternionBuffer FVoxelBufferMathUtilities::MakeQuaternionFromEuler(const FVoxelVectorBuffer& Euler)
{
	VOXEL_FUNCTION_COUNTER_NUM(Euler.Num());

	FVoxelQuaternionBuffer Result;
	Result.Allocate(Euler.Num());

	ispc::VoxelBufferMathUtilities_MakeQuaternionFromEuler(
		Euler.X.GetData(),
		Euler.X.IsConstant(),
		Euler.Y.GetData(),
		Euler.Y.IsConstant(),
		Euler.Z.GetData(),
		Euler.Z.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData(),
		Result.W.GetData(),
		Euler.Num());

	return Result;
}

FVoxelVector2DBuffer FVoxelBufferMathUtilities::RotateVector2D(const FVoxelVector2DBuffer& Vector, const FVoxelFloatBuffer& AngleDeg)
{
	const FVoxelBufferAccessor BufferAccessor(Vector, AngleDeg);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelVector2DBuffer::MakeDefault();
	}

	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelVector2DBuffer Result;
	Result.Allocate(BufferAccessor.Num());

	ispc::VoxelBufferMathUtilities_RotateVector2D(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		AngleDeg.GetData(),
		AngleDeg.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		BufferAccessor.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer FVoxelBufferMathUtilities::Length(const FVoxelVector2DBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_Length_float2(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Result.GetData(),
		Vector.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferMathUtilities::Length(const FVoxelVectorBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_Length_float3(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Vector.Z.GetData(),
		Vector.Z.IsConstant(),
		Result.GetData(),
		Vector.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferMathUtilities::Length(const FVoxelIntPointBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_Length_int2(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Result.GetData(),
		Vector.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferMathUtilities::Length(const FVoxelIntVectorBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_Length_int3(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Vector.Z.GetData(),
		Vector.Z.IsConstant(),
		Result.GetData(),
		Vector.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer FVoxelBufferMathUtilities::SquaredLength(const FVoxelVector2DBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_SquaredLength_float2(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Result.GetData(),
		Vector.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferMathUtilities::SquaredLength(const FVoxelVectorBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_SquaredLength_float3(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Vector.Z.GetData(),
		Vector.Z.IsConstant(),
		Result.GetData(),
		Vector.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferMathUtilities::SquaredLength(const FVoxelIntPointBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_SquaredLength_int2(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Result.GetData(),
		Vector.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferMathUtilities::SquaredLength(const FVoxelIntVectorBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelFloatBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_SquaredLength_int3(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Vector.Z.GetData(),
		Vector.Z.IsConstant(),
		Result.GetData(),
		Vector.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVector2DBuffer FVoxelBufferMathUtilities::Normalize(const FVoxelVector2DBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelVector2DBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_Normalize_float2(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Vector.Num());

	return Result;
}

FVoxelVectorBuffer FVoxelBufferMathUtilities::Normalize(const FVoxelVectorBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelVectorBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_Normalize_float3(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Vector.Z.GetData(),
		Vector.Z.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData(),
		Vector.Num());

	return Result;
}

FVoxelVector2DBuffer FVoxelBufferMathUtilities::Normalize(const FVoxelIntPointBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelVector2DBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_Normalize_int2(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Vector.Num());

	return Result;
}

FVoxelVectorBuffer FVoxelBufferMathUtilities::Normalize(const FVoxelIntVectorBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelVectorBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_Normalize_int3(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Vector.Z.GetData(),
		Vector.Z.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData(),
		Vector.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelQuaternionBuffer FVoxelBufferMathUtilities::MakeQuaternionFromZ(const FVoxelVectorBuffer& Vector)
{
	VOXEL_FUNCTION_COUNTER_NUM(Vector.Num());

	FVoxelQuaternionBuffer Result;
	Result.Allocate(Vector.Num());

	ispc::VoxelBufferMathUtilities_MakeQuaternionFromZ(
		Vector.X.GetData(),
		Vector.X.IsConstant(),
		Vector.Y.GetData(),
		Vector.Y.IsConstant(),
		Vector.Z.GetData(),
		Vector.Z.IsConstant(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData(),
		Result.W.GetData(),
		Vector.Num());

	return Result;
}