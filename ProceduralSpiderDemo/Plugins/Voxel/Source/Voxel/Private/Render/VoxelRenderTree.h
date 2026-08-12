// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelChunkKey.h"

class FVoxelRenderChunk;
struct FVoxelRenderSubsystem;
struct FVoxelRenderInvokersContainer;

class FVoxelRenderTree
{
public:
	const int32 VoxelSize;
	const int32 Depth;
	const TVoxelStaticArray<TSharedRef<FVoxelRenderChunk>, 8> RootChunks;
	bool bHasReachedMaxChunks = false;

	FVoxelRenderTree(
		const FVoxelRenderSubsystem& Subsystem,
		int32 Depth);
	~FVoxelRenderTree();

public:
	void Update(
		FVoxelRenderSubsystem& Subsystem,
		const FVoxelRenderInvokersContainer& InvokersContainer);
	void DestroyAllRenderDatas(FVoxelRenderSubsystem& Subsystem);

	TConstVoxelArrayView<FVoxelChunkKey> GetChunkKeysToRender() const;

	FVoxelRenderChunk* FindNeighbor(
		const FVoxelChunkKey& ChunkKey,
		int32 DirectionX,
		int32 DirectionY,
		int32 DirectionZ) const;

	template<typename LambdaType>
	requires LambdaHasSignature_V<LambdaType, void(FVoxelRenderChunk& Chunk)>
	void ForeachChunk(LambdaType Lambda)
	{
		VOXEL_FUNCTION_COUNTER();

		for (auto& It : ChunkKeyToChunk)
		{
			Lambda(*It.Value);
		}
	}

	FORCEINLINE TSharedRef<FVoxelRenderChunk> GetChunk(const FVoxelChunkKey& ChunkKey) const
	{
		return ChunkKeyToChunk[ChunkKey].ToSharedRef();
	}

private:
	void CollapseChunk(FVoxelRenderChunk& Chunk);
	void SubdivideChunk(FVoxelRenderChunk& Chunk);

	void DestroyChunk(
		FVoxelRenderSubsystem& Subsystem,
		FVoxelRenderChunk& Chunk);

private:
	void Traverse(
		const FVoxelRenderSubsystem& Subsystem,
		const TVoxelArray<FVector>& CameraInvokers);
	void Collapse(
		const FVoxelRenderSubsystem& Subsystem,
		const TVoxelArray<FVector>& CameraInvokers);
	void Subdivide(
		const FVoxelRenderSubsystem& Subsystem,
		const TVoxelArray<FVector>& CameraInvokers);
	void TraverseLODInvokers(const FVoxelRenderInvokersContainer& LODInvokers);
	void SubdivideNeighbors(FVoxelRenderSubsystem& Subsystem);
	void FinalizeTraversal(FVoxelRenderSubsystem& Subsystem);

private:
	// TODO Add array for LOD then store vector to chunk inside
	// TODO Don't use shared ptrs, use linear allocator
	TVoxelMap<FVoxelChunkKey, TSharedPtr<FVoxelRenderChunk>> ChunkKeyToChunk;
	TVoxelOptional<TVoxelArray<FVoxelChunkKey>> CachedChunkKeysToRender;

	friend class FVoxelRenderNeighborSubdivider;
};