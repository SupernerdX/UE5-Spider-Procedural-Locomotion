// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class FDistanceFieldVolumeData;
struct FVoxelCellGeneratorOutput;

struct FVoxelMesherDistanceFieldBuilder
{
	static TSharedPtr<FDistanceFieldVolumeData> Build(
		int32 ChunkLOD,
		const FInt64Vector& ChunkOffset,
		int32 VoxelSize,
		int32 ChunkSize,
		float MeshDistanceFieldBias,
		int32 MeshDistanceFieldBricksPerChunk,
		int32 DataSize,
		FVoxelCellGeneratorOutput& CellGeneratorOutput);
};