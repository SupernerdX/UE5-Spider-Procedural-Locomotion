// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelNavigationMeshMemory, "Voxel Navigation Mesh Memory");

class VOXEL_API FVoxelNavigationMesh
{
public:
	const FVoxelBox LocalBounds;
	const TVoxelArray<int32> Indices;
	const TVoxelArray<FVector3f> Vertices;
	const TSharedRef<FVoxelDependencyTracker> DependencyTracker;

	FVoxelNavigationMesh(
		TVoxelArray<int32>&& Indices,
		TVoxelArray<FVector3f>&& Vertices,
		const TSharedRef<FVoxelDependencyTracker>& DependencyTracker)
		: LocalBounds(FVoxelBox::FromPositions(Vertices))
		, Indices(Indices)
		, Vertices(Vertices)
		, DependencyTracker(DependencyTracker)
	{
	}

	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelNavigationMeshMemory);

	bool IsEmpty() const
	{
		return Indices.Num() == 0;
	}
	int64 GetAllocatedSize() const
	{
		return Indices.GetAllocatedSize() + Vertices.GetAllocatedSize();
	}
};