// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelTerminalBuffer.h"
#include "VoxelBaseBuffers.generated.h"

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelBoolBuffer, bool);

USTRUCT(DisplayName = "Boolean Buffer")
struct VOXELGRAPH_API FVoxelBoolBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelBoolBuffer, bool);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelByteBuffer, uint8);

template<typename T>
requires std::is_enum_v<T>
struct TVoxelBufferTypeImpl<T>
{
	using Type = FVoxelByteBuffer;
};

USTRUCT()
struct VOXELGRAPH_API FVoxelByteBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelByteBuffer, uint8);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace ispc
{
#ifndef __ISPC_STRUCT_FVoxelFloatBuffer__
#define __ISPC_STRUCT_FVoxelFloatBuffer__
struct FVoxelFloatBuffer
{
	bool bIsConstant;
	const float* Values;
};
#endif
}

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelFloatBuffer, float);

USTRUCT()
struct VOXELGRAPH_API FVoxelFloatBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelFloatBuffer, float);

	FORCEINLINE ispc::FVoxelFloatBuffer ISPC() const
	{
		return ispc::FVoxelFloatBuffer
		{
			IsConstant(),
			GetData()
		};
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace ispc
{
#ifndef __ISPC_STRUCT_FVoxelDoubleBuffer__
#define __ISPC_STRUCT_FVoxelDoubleBuffer__
struct FVoxelDoubleBuffer
{
	bool bIsConstant;
	const double* Values;
};
#endif
}

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelDoubleBuffer, double);

USTRUCT()
struct VOXELGRAPH_API FVoxelDoubleBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelDoubleBuffer, double);

	FORCEINLINE ispc::FVoxelDoubleBuffer ISPC() const
	{
		return ispc::FVoxelDoubleBuffer
		{
			IsConstant(),
			GetData()
		};
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelInt32Buffer, int32);

USTRUCT(DisplayName = "Integer Buffer")
struct VOXELGRAPH_API FVoxelInt32Buffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelInt32Buffer, int32);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelInt64Buffer, int64);

USTRUCT(DisplayName = "Integer64 Buffer")
struct VOXELGRAPH_API FVoxelInt64Buffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelInt64Buffer, int64);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelUInt16Buffer, uint16);

USTRUCT(DisplayName = "uint16 Buffer")
struct VOXELGRAPH_API FVoxelUInt16Buffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelUInt16Buffer, uint16);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct VOXELGRAPH_API FVoxelSeed
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Seed = 0;

	FVoxelSeed() = default;
	FORCEINLINE FVoxelSeed(const int32 Seed)
		: Seed(Seed)
	{
	}
	FORCEINLINE operator int32() const
	{
		return Seed;
	}

	FORCEINLINE bool operator==(const FVoxelSeed& Other) const
	{
		return Seed == Other.Seed;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelSeed InSeed)
	{
		return InSeed.Seed;
	}
};
checkStatic(sizeof(FVoxelSeed) == sizeof(int32));

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelSeedBuffer, FVoxelSeed);

USTRUCT()
struct VOXELGRAPH_API FVoxelSeedBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelSeedBuffer, FVoxelSeed);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelColorBuffer, FColor);

USTRUCT()
struct VOXELGRAPH_API FVoxelColorBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelColorBuffer, FColor);
};