// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_Lerp(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		const float Alpha = LoadFloat(InputBuffers[2], Index);
		const bool Clamp = LoadBool(InputBuffers[3], Index);
		float ReturnValue;

		ReturnValue = lerp_NaN(A, B, Clamp ? clamp(Alpha, 0.f, 1.f) : Alpha);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_SmoothStep(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		const float Alpha = LoadFloat(InputBuffers[2], Index);
		const bool Clamp = LoadBool(InputBuffers[3], Index);
		float ReturnValue;

		ReturnValue = SmoothStep_NaN(A, B, Clamp ? clamp(Alpha, 0.f, 1.f) : Alpha);

		StoreFloat(OutputBuffers[0], Index, ReturnValue);
	}
}