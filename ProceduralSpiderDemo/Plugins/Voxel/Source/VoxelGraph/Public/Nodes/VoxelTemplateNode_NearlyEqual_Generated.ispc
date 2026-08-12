// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelISPCNodeHelpers.isph"

export void VoxelNode_VoxelComputeNode_NearlyEqual_Double(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const double A = LoadDouble(InputBuffers[0], Index);
		const double B = LoadDouble(InputBuffers[1], Index);
		const double ErrorTolerance = LoadDouble(InputBuffers[2], Index);
		bool ReturnValue;

		ReturnValue = abs(A - B) <= ErrorTolerance;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}

export void VoxelNode_VoxelComputeNode_NearlyEqual_Float(const FVoxelInputBuffer* uniform InputBuffers, void* uniform const* uniform OutputBuffers, const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const float A = LoadFloat(InputBuffers[0], Index);
		const float B = LoadFloat(InputBuffers[1], Index);
		const float ErrorTolerance = LoadFloat(InputBuffers[2], Index);
		bool ReturnValue;

		ReturnValue = abs(A - B) <= ErrorTolerance;

		StoreBool(OutputBuffers[0], Index, ReturnValue);
	}
}