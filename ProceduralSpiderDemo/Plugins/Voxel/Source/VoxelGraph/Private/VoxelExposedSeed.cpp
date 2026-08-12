// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelExposedSeed.h"

int32 FVoxelExposedSeed::GetSeed() const
{
	return FCrc::StrCrc32(*Seed);
}

void FVoxelExposedSeed::Randomize(const int32 RandSeed)
{
	const FRandomStream Stream(RandSeed);

	Seed.Reset(8);
	for (int32 Index = 0; Index < 8; Index++)
	{
		Seed += TCHAR(Stream.RandRange(TEXT('A'), TEXT('Z')));
	}
}