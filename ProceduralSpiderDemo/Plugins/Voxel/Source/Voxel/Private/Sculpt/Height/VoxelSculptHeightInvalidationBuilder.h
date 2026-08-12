// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Height/VoxelHeightChunks.h"
#include "Sculpt/Height/VoxelHeightChunkDefinitions.h"

class FVoxelAABBTree2D;

class FVoxelSculptHeightInvalidationBuilder : public FVoxelHeightChunkDefinitions
{
public:
	static TSharedRef<const FVoxelAABBTree2D> Create(
		const TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>>& KeyToFarChunkA,
		const TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>>& KeyToFarChunkB);

private:
	TVoxelChunkedArray<FVoxelIntBox2D> BoundsToInvalidate;

	void Traverse(
		const FIntPoint& ChunkKey,
		const TVoxelBulkPtr<FVoxelHeightFarChunk>& ChunkA,
		const TVoxelBulkPtr<FVoxelHeightFarChunk>& ChunkB);

	void Traverse(
		const FIntPoint& ChunkKey,
		const TVoxelBulkPtr<FVoxelHeightMidChunk>& ChunkA,
		const TVoxelBulkPtr<FVoxelHeightMidChunk>& ChunkB);

	void Traverse(
		const FIntPoint& ChunkKey,
		const TVoxelBulkPtr<FVoxelHeightNearChunk>& ChunkA,
		const TVoxelBulkPtr<FVoxelHeightNearChunk>& ChunkB);
};