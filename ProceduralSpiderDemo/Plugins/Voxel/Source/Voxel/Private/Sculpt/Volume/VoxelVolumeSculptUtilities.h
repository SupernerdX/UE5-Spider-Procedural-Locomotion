// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"

struct FVoxelVolumeSculptUtilities : FVoxelVolumeChunkDefinitions
{
	static void DiffDistances(
		TConstVoxelArrayView<float> Distances,
		TConstVoxelArrayView<float> PreviousDistances,
		const FIntVector& Size,
		TVoxelArray<float>& AdditiveDistances,
		TVoxelArray<float>& SubtractiveDistances);
};