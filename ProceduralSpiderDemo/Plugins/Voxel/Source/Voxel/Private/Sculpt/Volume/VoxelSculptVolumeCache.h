// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"

class FVoxelLayers;
class AVoxelSculptVolume;
class FVoxelSurfaceTypeTable;
struct FVoxelWeakStackLayer;

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelVolumeSculptCache_Memory, "Voxel Volume Sculpt Cache Chunk Memory");

struct FVoxelVolumeSculptPreviousChunk : FVoxelVolumeChunkDefinitions
{
public:
	TVoxelStaticArray<float, ChunkCount> Distances{ NoInit };

	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelVolumeSculptCache_Memory);

	int64 GetAllocatedSize() const;

private:
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;

	FVoxelVolumeSculptPreviousChunk();

	friend class FVoxelSculptVolumeCache;
};

class FVoxelSculptVolumeCache : FVoxelVolumeChunkDefinitions
{
public:
	const FMatrix SculptToWorld;
	const TVoxelObjectPtr<const AVoxelSculptVolume> WeakSculptActor;

	FVoxelSculptVolumeCache(
		const FMatrix& SculptToWorld,
		TVoxelObjectPtr<const AVoxelSculptVolume> SculptActor);
	~FVoxelSculptVolumeCache();

public:
	TSharedRef<const FVoxelVolumeSculptPreviousChunk> ComputeChunk(
		const FVoxelLayers& Layers,
		const FVoxelSurfaceTypeTable& SurfaceTypeTable,
		const FVoxelWeakStackLayer& WeakLayer,
		const FIntVector& ChunkKey);

private:
	FVoxelSharedCriticalSection CriticalSection;
	TVoxelMap<FIntVector, TSharedPtr<const FVoxelVolumeSculptPreviousChunk>> ChunkKeyToChunk_RequiresLock;
};