// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelChunkKey.h"

class FVoxelRenderTree;
class FVoxelRenderChunk;
struct FVoxelRenderSubsystem;

class FVoxelRenderNeighborSubdivider
{
public:
	FVoxelRenderSubsystem& Subsystem;
	FVoxelRenderTree& Tree;
	const TVoxelStaticArray<TSharedRef<FVoxelRenderChunk>, 8> RootChunks;
	const int32 MaxLOD;
	const FVoxelShouldCancel ShouldCancel;
	TVoxelAtomic<bool> HasReachedMaxChunks;

	explicit FVoxelRenderNeighborSubdivider(
		FVoxelRenderSubsystem& Subsystem,
		FVoxelRenderTree& Tree,
		const TVoxelStaticArray<TSharedRef<FVoxelRenderChunk>, 8>& RootChunks);

	void Traverse();
	void ProcessNewChunks();

private:
	static constexpr int32 MaxChunksToCheck = 1024;

	FVoxelParallelTaskScope Scope;
	FVoxelSharedCriticalSection CriticalSection;
	TVoxelInlineArray<FVoxelRenderChunk*, MaxChunksToCheck> ChunksToCheck;

	TVoxelSet<FVoxelChunkKey> Chunks;
	FVoxelBitArray IsLeaf;

	TVoxelChunkedArray<const FVoxelRenderChunk*> NewChunks;

	void InitializeChunks();
	void FlushChunksToCheck();
	void CheckChunk(const FVoxelRenderChunk& Chunk);
	void SubdivideChunk(const FVoxelChunkKey& ChunkKey);
};