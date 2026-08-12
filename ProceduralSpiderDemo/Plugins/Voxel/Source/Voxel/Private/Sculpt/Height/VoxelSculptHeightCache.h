// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Height/VoxelHeightChunkDefinitions.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;
class AVoxelSculptHeight;
struct FVoxelWeakStackLayer;

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelHeightSculptCache_Memory, "Voxel Height Sculpt Cache Chunk Memory");

struct FVoxelHeightSculptPreviousChunk : FVoxelHeightChunkDefinitions
{
public:
	TVoxelStaticArray<float, ChunkCount> Heights{ NoInit };

	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelHeightSculptCache_Memory);

	int64 GetAllocatedSize() const;

private:
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;

	FVoxelHeightSculptPreviousChunk();

	friend class FVoxelSculptHeightCache;
};

class FVoxelSculptHeightCache : FVoxelHeightChunkDefinitions
{
public:
	const FTransform2d SculptToWorld;
	const float ScaleZ;
	const float OffsetZ;
	const TVoxelObjectPtr<const AVoxelSculptHeight> WeakSculptActor;

	FVoxelSculptHeightCache(
		const FTransform2d& SculptToWorld,
		float ScaleZ,
		float OffsetZ,
		TVoxelObjectPtr<const AVoxelSculptHeight> SculptActor);
	~FVoxelSculptHeightCache();

public:
	TSharedRef<const FVoxelHeightSculptPreviousChunk> ComputeChunk(
		const FVoxelLayers& Layers,
		const FVoxelSurfaceTypeTable& SurfaceTypeTable,
		const FVoxelWeakStackLayer& WeakLayer,
		const FIntPoint& ChunkKey);

private:
	FVoxelSharedCriticalSection CriticalSection;
	TVoxelMap<FIntPoint, TSharedPtr<const FVoxelHeightSculptPreviousChunk>> ChunkKeyToChunk_RequiresLock;
};