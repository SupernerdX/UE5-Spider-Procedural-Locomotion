// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/VoxelVolumeChunks.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"

class FVoxelAABBTree;

class FVoxelSculptVolumeInvalidationBuilder : public FVoxelVolumeChunkDefinitions
{
public:
	static TSharedRef<const FVoxelAABBTree> Create(
		const TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>>& KeyToFarChunkA,
		const TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>>& KeyToFarChunkB);

private:
	TVoxelChunkedArray<FVoxelIntBox> BoundsToInvalidate;

	void Traverse(
		const FIntVector& ChunkKey,
		const TVoxelBulkPtr<FVoxelVolumeFarChunk>& ChunkA,
		const TVoxelBulkPtr<FVoxelVolumeFarChunk>& ChunkB);

	void Traverse(
		const FIntVector& ChunkKey,
		const TVoxelBulkPtr<FVoxelVolumeMidChunk>& ChunkA,
		const TVoxelBulkPtr<FVoxelVolumeMidChunk>& ChunkB);

	void Traverse(
		const FIntVector& ChunkKey,
		const TVoxelBulkPtr<FVoxelVolumeNearChunk>& ChunkA,
		const TVoxelBulkPtr<FVoxelVolumeNearChunk>& ChunkB);
};