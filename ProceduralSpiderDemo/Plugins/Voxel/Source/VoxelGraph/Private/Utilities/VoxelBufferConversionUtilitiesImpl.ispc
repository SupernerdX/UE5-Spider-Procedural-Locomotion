// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

// ReSharper disable CppCStyleCast

export void VoxelBufferConversionUtilities_BooleanToInt32(
	const uniform bool Bool[],
	uniform int32 Int32[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Int32[Index] = select(Bool[Index], 1, 0);
	}
}

export void VoxelBufferConversionUtilities_ByteToInt32(
	const uniform uint8 Byte[],
	uniform int32 Int32[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Int32[Index] = Byte[Index];
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferConversionUtilities_Int32ToFloat(
	const uniform int32 Int32[],
	uniform float Float[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Float[Index] = (float)Int32[Index];
	}
}

export void VoxelBufferConversionUtilities_Int32ToDouble(
	const uniform int32 Int32[],
	uniform double Double[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Double[Index] = (double)Int32[Index];
	}
}

export void VoxelBufferConversionUtilities_Int32ToInt64(
	const uniform int32 Int32[],
	uniform int64 Int64[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Int64[Index] = Int32[Index];
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferConversionUtilities_Int64ToFloat(
	const uniform int64 Int64[],
	uniform float Float[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Float[Index] = (float)Int64[Index];
	}
}

export void VoxelBufferConversionUtilities_Int64ToDouble(
	const uniform int64 Int64[],
	uniform double Double[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Double[Index] = (double)Int64[Index];
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferConversionUtilities_FloatToDouble(
	const uniform float Float[],
	uniform double Double[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Double[Index] = (double)Float[Index];
	}
}

export void VoxelBufferConversionUtilities_DoubleToFloat(
	const uniform double Double[],
	uniform float Float[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Float[Index] = (float)Double[Index];
	}
}