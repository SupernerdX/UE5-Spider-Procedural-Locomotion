// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

// ReSharper disable CppCStyleCast

export void VoxelPointUtilities_PointIdToSeed(
	const uniform uint64 PointId[],
	const uniform uint32 Seed,
	uniform uint32 OutSeed[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const uint64 Value = PointId[Index];
		OutSeed[Index] = MurmurHash32(MurmurHash64(Value), Seed);
	}
}