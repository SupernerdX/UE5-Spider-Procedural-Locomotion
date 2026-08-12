// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"
#include "VoxelBufferImpl.isph"

export void VoxelShapeFunctionLibrary_MakeSphere(
	const uniform FVoxelDoubleVectorBuffer& Position,
	const uniform FVoxelDoubleBuffer& Radius,
	uniform float Distance[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Distance[Index] = length(Get(Position, Index)) - Get(Radius, Index);
	}
}

export void VoxelShapeFunctionLibrary_MakeCube(
	const uniform FVoxelDoubleVectorBuffer& Position,
	const uniform FVoxelDoubleVectorBuffer& Size,
	const uniform FVoxelDoubleBuffer& Roundness,
	uniform float Distance[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying double3 HalfSize = max(Get(Size, Index), double3_zero) / 2.f;
		const varying double Radius = min3(HalfSize.x, HalfSize.y, HalfSize.z) * Get(Roundness, Index);

		const varying double3 Q = abs(Get(Position, Index)) - HalfSize + Radius;
		const varying double Length = length(max(Q, double3_zero));

		Distance[Index] = Length + min(0.f, max3(Q.x, Q.y, Q.z)) - Radius;
	}
}