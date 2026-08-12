// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

namespace ispc
{
#define __ISPC_STRUCT_FVoxelBuffer__
	struct FVoxelInputBuffer
	{
		const void* Data = nullptr;
		bool bIsConstant = false;
	};
}

using FVoxelNodeISPCPtr = void (*)(const ispc::FVoxelInputBuffer* InputBuffers, void* const* OutputBuffers, int32 Num);

extern VOXELGRAPH_API TVoxelMap<FName, FVoxelNodeISPCPtr> GVoxelNodeISPCPtrs;

#define REGISTER_VOXEL_NODE_ISPC(HeaderName, Name) \
	namespace ispc \
	{ \
		extern "C" \
		{ \
			extern void VoxelNode_ ## Name(const FVoxelInputBuffer* InputBuffers, void* const* OutputBuffers, const int32 Num); \
		} \
	} \
	VOXEL_RUN_ON_STARTUP_GAME() \
	{ \
		GVoxelNodeISPCPtrs.Add_EnsureNew(#Name, ispc::VoxelNode_ ## Name); \
	}