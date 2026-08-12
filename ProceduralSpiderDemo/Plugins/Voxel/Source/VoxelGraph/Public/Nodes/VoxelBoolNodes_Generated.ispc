// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_BooleanAND(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const bool A = LoadBool(InputBuffers[0], Index);
		const bool B = LoadBool(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A && B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_BooleanNAND(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const bool A = LoadBool(InputBuffers[0], Index);
		const bool B = LoadBool(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = !(A && B);

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_BooleanOR(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const bool A = LoadBool(InputBuffers[0], Index);
		const bool B = LoadBool(InputBuffers[1], Index);
		bool ReturnValue;

		ReturnValue = A || B;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}