// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelBaseBuffers.h"
#include "VoxelBufferStruct.h"
#include "VoxelRangeBuffers.generated.h"

DECLARE_VOXEL_BUFFER(FVoxelFloatRangeBuffer, FVoxelFloatRange);

USTRUCT()
struct VOXELGRAPH_API FVoxelFloatRangeBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelFloatRangeBuffer, FVoxelFloatRange);

	UPROPERTY()
	FVoxelFloatBuffer Min;

	UPROPERTY()
	FVoxelFloatBuffer Max;

	FORCEINLINE const FVoxelFloatRange operator[](const int32 Index) const
	{
		return FVoxelFloatRange(Min[Index], Max[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FVoxelFloatRange& Value)
	{
		Min.Set(Index, Value.Min);
		Max.Set(Index, Value.Max);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_BUFFER(FVoxelInt32RangeBuffer, FVoxelInt32Range);

USTRUCT(DisplayName = "Integer Range Buffer")
struct VOXELGRAPH_API FVoxelInt32RangeBuffer final : public FVoxelBufferStruct
{
	GENERATED_BODY()
	GENERATED_VOXEL_BUFFER_STRUCT_BODY(FVoxelInt32RangeBuffer, FVoxelInt32Range);

	UPROPERTY()
	FVoxelInt32Buffer Min;

	UPROPERTY()
	FVoxelInt32Buffer Max;

	FORCEINLINE const FVoxelInt32Range operator[](const int32 Index) const
	{
		return FVoxelInt32Range(Min[Index], Max[Index]);
	}
	FORCEINLINE void Set(const int32 Index, const FVoxelInt32Range& Value)
	{
		Min.Set(Index, Value.Min);
		Max.Set(Index, Value.Max);
	}
};