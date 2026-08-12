// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Buffer/VoxelIntegerBuffers.h"

// Windows include
#undef Int64ToInt32

struct FVoxelPointIdBuffer;

struct VOXELGRAPH_API FVoxelBufferConversionUtilities
{
public:
	static FVoxelInt32Buffer ByteToInt32(const FVoxelByteBuffer& Buffer);
	static FVoxelInt32Buffer BooleanToInt32(const FVoxelBoolBuffer& Buffer);

public:
	static FVoxelFloatBuffer Int32ToFloat(const FVoxelInt32Buffer& Buffer);
	static FVoxelDoubleBuffer Int32ToDouble(const FVoxelInt32Buffer& Buffer);
	static FVoxelInt64Buffer Int32ToInt64(const FVoxelInt32Buffer& Buffer);

	static FVoxelInt64PointBuffer Int32ToInt64(const FVoxelIntPointBuffer& Buffer);
	static FVoxelInt64VectorBuffer Int32ToInt64(const FVoxelIntVectorBuffer& Buffer);
	static FVoxelInt64Vector4Buffer Int32ToInt64(const FVoxelIntVector4Buffer& Buffer);

public:
	static FVoxelFloatBuffer Int64ToFloat(const FVoxelInt64Buffer& Buffer);
	static FVoxelDoubleBuffer Int64ToDouble(const FVoxelInt64Buffer& Buffer);
	static FVoxelInt32Buffer Int64ToInt32(const FVoxelInt64Buffer& Buffer);

public:
	static FVoxelDoubleBuffer FloatToDouble(const FVoxelFloatBuffer& Buffer);
	static FVoxelFloatBuffer DoubleToFloat(const FVoxelDoubleBuffer& Buffer);

	static FVoxelDoubleVector2DBuffer FloatToDouble(const FVoxelVector2DBuffer& Buffer);
	static FVoxelDoubleVectorBuffer FloatToDouble(const FVoxelVectorBuffer& Buffer);

	static FVoxelVector2DBuffer DoubleToFloat(const FVoxelDoubleVector2DBuffer& Buffer);
	static FVoxelVectorBuffer DoubleToFloat(const FVoxelDoubleVectorBuffer& Buffer);
};