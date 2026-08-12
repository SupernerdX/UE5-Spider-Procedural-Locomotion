// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Utilities/VoxelBufferConversionUtilities.h"
#include "VoxelBufferConversionUtilitiesImpl.ispc.generated.h"

FVoxelInt32Buffer FVoxelBufferConversionUtilities::BooleanToInt32(const FVoxelBoolBuffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelInt32Buffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferConversionUtilities_BooleanToInt32(
		Buffer.GetData(),
		Result.GetData(),
		Buffer.Num());

	return Result;
}

FVoxelInt32Buffer FVoxelBufferConversionUtilities::ByteToInt32(const FVoxelByteBuffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelInt32Buffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferConversionUtilities_ByteToInt32(
		Buffer.GetData(),
		Result.GetData(),
		Buffer.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferConversionUtilities::Int32ToFloat(const FVoxelInt32Buffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelFloatBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferConversionUtilities_Int32ToFloat(
		Buffer.GetData(),
		Result.GetData(),
		Buffer.Num());

	return Result;
}

FVoxelDoubleBuffer FVoxelBufferConversionUtilities::Int32ToDouble(const FVoxelInt32Buffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelDoubleBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferConversionUtilities_Int32ToDouble(
		Buffer.GetData(),
		Result.GetData(),
		Buffer.Num());

	return Result;
}

FVoxelInt64Buffer FVoxelBufferConversionUtilities::Int32ToInt64(const FVoxelInt32Buffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelInt64Buffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferConversionUtilities_Int32ToInt64(
		Buffer.GetData(),
		Result.GetData(),
		Buffer.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelInt64PointBuffer FVoxelBufferConversionUtilities::Int32ToInt64(const FVoxelIntPointBuffer& Buffer)
{
	FVoxelInt64PointBuffer Result;
	Result.X = Int32ToInt64(Buffer.X);
	Result.Y = Int32ToInt64(Buffer.Y);
	return Result;
}

FVoxelInt64VectorBuffer FVoxelBufferConversionUtilities::Int32ToInt64(const FVoxelIntVectorBuffer& Buffer)
{
	FVoxelInt64VectorBuffer Result;
	Result.X = Int32ToInt64(Buffer.X);
	Result.Y = Int32ToInt64(Buffer.Y);
	Result.Z = Int32ToInt64(Buffer.Z);
	return Result;
}

FVoxelInt64Vector4Buffer FVoxelBufferConversionUtilities::Int32ToInt64(const FVoxelIntVector4Buffer& Buffer)
{
	FVoxelInt64Vector4Buffer Result;
	Result.X = Int32ToInt64(Buffer.X);
	Result.Y = Int32ToInt64(Buffer.Y);
	Result.Z = Int32ToInt64(Buffer.Z);
	Result.W = Int32ToInt64(Buffer.W);
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer FVoxelBufferConversionUtilities::Int64ToFloat(const FVoxelInt64Buffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelFloatBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferConversionUtilities_Int64ToFloat(
		Buffer.GetData(),
		Result.GetData(),
		Buffer.Num());

	return Result;
}

FVoxelDoubleBuffer FVoxelBufferConversionUtilities::Int64ToDouble(const FVoxelInt64Buffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelDoubleBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferConversionUtilities_Int64ToDouble(
		Buffer.GetData(),
		Result.GetData(),
		Buffer.Num());

	return Result;
}

FVoxelInt32Buffer FVoxelBufferConversionUtilities::Int64ToInt32(const FVoxelInt64Buffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelInt32Buffer Result;
	Result.Allocate(Buffer.Num());

	// Not in ISPC because of a compiler bug

	for (int32 Index = 0; Index < Buffer.Num(); Index++)
	{
		Result.Set(Index, FMath::Clamp<int64>(Buffer[Index], MIN_int32, MAX_int32));
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDoubleBuffer FVoxelBufferConversionUtilities::FloatToDouble(const FVoxelFloatBuffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelDoubleBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferConversionUtilities_FloatToDouble(
		Buffer.GetData(),
		Result.GetData(),
		Buffer.Num());

	return Result;
}

FVoxelFloatBuffer FVoxelBufferConversionUtilities::DoubleToFloat(const FVoxelDoubleBuffer& Buffer)
{
	VOXEL_FUNCTION_COUNTER_NUM(Buffer.Num(), 1024);

	FVoxelFloatBuffer Result;
	Result.Allocate(Buffer.Num());

	ispc::VoxelBufferConversionUtilities_DoubleToFloat(
		Buffer.GetData(),
		Result.GetData(),
		Buffer.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDoubleVector2DBuffer FVoxelBufferConversionUtilities::FloatToDouble(const FVoxelVector2DBuffer& Buffer)
{
	FVoxelDoubleVector2DBuffer Result;
	Result.X = FloatToDouble(Buffer.X);
	Result.Y = FloatToDouble(Buffer.Y);
	return Result;
}

FVoxelDoubleVectorBuffer FVoxelBufferConversionUtilities::FloatToDouble(const FVoxelVectorBuffer& Buffer)
{
	FVoxelDoubleVectorBuffer Result;
	Result.X = FloatToDouble(Buffer.X);
	Result.Y = FloatToDouble(Buffer.Y);
	Result.Z = FloatToDouble(Buffer.Z);
	return Result;
}

FVoxelVector2DBuffer FVoxelBufferConversionUtilities::DoubleToFloat(const FVoxelDoubleVector2DBuffer& Buffer)
{
	FVoxelVector2DBuffer Result;
	Result.X = DoubleToFloat(Buffer.X);
	Result.Y = DoubleToFloat(Buffer.Y);
	return Result;
}

FVoxelVectorBuffer FVoxelBufferConversionUtilities::DoubleToFloat(const FVoxelDoubleVectorBuffer& Buffer)
{
	FVoxelVectorBuffer Result;
	Result.X = DoubleToFloat(Buffer.X);
	Result.Y = DoubleToFloat(Buffer.Y);
	Result.Z = DoubleToFloat(Buffer.Z);
	return Result;
}