// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class FVoxelMesh;
class UVoxelSurfaceTypeAsset;

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelColliderMemory, "Voxel Collider Memory");

class VOXEL_API FVoxelCollider
{
public:
	const TRefCountPtr<Chaos::FTriangleMeshImplicitObject> TriangleMesh;
	const TVoxelArray<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>> SurfaceTypes;

	FVoxelCollider(
		const TRefCountPtr<Chaos::FTriangleMeshImplicitObject>& TriangleMesh,
		const TVoxelArray<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>>& SurfaceTypes);
	~FVoxelCollider();
	UE_NONCOPYABLE(FVoxelCollider);

	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelColliderMemory);

	int64 GetAllocatedSize() const;

public:
	static TSharedPtr<FVoxelCollider> Create(const FVoxelMesh& Mesh);

	static void BuildSurfaceTypes(
		const FVoxelMesh& Mesh,
		TVoxelArray<uint16>& OutFaceMaterials,
		TVoxelArray<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>>& OutSurfaceTypes);
};