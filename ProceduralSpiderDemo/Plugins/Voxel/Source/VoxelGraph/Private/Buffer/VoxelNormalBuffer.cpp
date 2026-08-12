// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Buffer/VoxelNormalBuffer.h"

FVoxelVectorBuffer FVoxelNormalBuffer::GetUnitVector() const
{
	VOXEL_FUNCTION_COUNTER_NUM(Num());

	FVoxelVectorBuffer Result;
	Result.Allocate(Num());

	for (int32 Index = 0; Index < Num(); Index++)
	{
		Result.Set(Index, (*this)[Index].GetUnitVector());
	}

	return Result;
}