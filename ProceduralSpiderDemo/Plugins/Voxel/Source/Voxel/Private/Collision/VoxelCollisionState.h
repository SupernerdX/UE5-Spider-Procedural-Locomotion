// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"

class FVoxelLayers;
class FVoxelCollider;
class FVoxelSurfaceTypeTable;
class FVoxelMegaMaterialProxy;
class UVoxelSurfaceTypeAsset;

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelCollisionChunkMemory, "Voxel Collision Chunk Memory");

struct FVoxelCollisionChunk
{
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;

	TVoxelArray<int32> Indices;
	TVoxelArray<FVector3f> Vertices;
	TVoxelArray<uint16> FaceMaterials;
	TVoxelArray<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>> SurfaceTypes;

	bool bNeedsLoad = false;
	TSharedPtr<FVoxelCollider> Collider;

	VOXEL_COUNT_INSTANCES();
	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelCollisionChunkMemory);

	int64 GetAllocatedSize() const
	{
		return int64(
			Indices.GetAllocatedSize() +
			Vertices.GetAllocatedSize() +
			FaceMaterials.GetAllocatedSize() +
			SurfaceTypes.GetAllocatedSize());
	}
};

class FVoxelCollisionState : public TSharedFromThis<FVoxelCollisionState>
{
public:
	const int32 VoxelSize;
	const int32 ChunkSize;

	TSharedPtr<FVoxelLayers> Layers;
	TSharedPtr<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	TSharedPtr<FVoxelMegaMaterialProxy> MegaMaterialProxy;
	FVector InvokerPosition = FVector(ForceInit);
	double InvokerRadius = 0;
	FVoxelFuture UpdateFuture;

	bool bIsRendered = false;
	TVoxelAtomic<bool> ShouldExit;
	TVoxelAtomic<bool> Invalidated;
	TVoxelAtomic<int32> NumTasks;
	TVoxelAtomic<int32> TotalNumTasks;

	TVoxelMap<FIntVector, TSharedPtr<FVoxelCollisionChunk>> ChunkKeyToChunk;

	FVoxelCollisionState(
		const int32 VoxelSize,
		const int32 ChunkSize)
		: VoxelSize(VoxelSize)
		, ChunkSize(ChunkSize)
	{
	}
	~FVoxelCollisionState();

public:
	FVoxelFuture Update(
		const FVoxelWeakStackLayer& WeakLayer,
		bool bCanLoad);

	void Serialize(FArchive& Ar);

private:
	FVoxelWeakStackLayer LastWeakLayer;

	TSharedRef<FVoxelCollisionChunk> CreateChunk(
		const FVoxelWeakStackLayer& WeakLayer,
		const FIntVector& ChunkKey);
};