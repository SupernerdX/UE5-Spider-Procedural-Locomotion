// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "StaticMesh/VoxelStaticMeshPoint.h"
#include "StaticMesh/VoxelStaticMeshSettings.h"

class UVoxelStaticMesh;

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelVoxelizedMeshData, "Voxel Voxelized Mesh Data Memory");

class VOXEL_API FVoxelStaticMeshData
{
public:
	FVoxelBox MeshBounds;
	int32 VoxelSize = 0;
	float BoundsExtension = 0;
	FVoxelStaticMeshSettings VoxelizerSettings;

	FVector3f Origin = FVector3f::ZeroVector;
	FIntVector Size = FIntVector::ZeroValue;

	TVoxelArray<float> DistanceField;
	TVoxelArray<int32> PointIndices;
	TVoxelArray<FVoxelStaticMeshPoint> Points;

	FVoxelStaticMeshData() = default;

	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelVoxelizedMeshData);

	int64 GetAllocatedSize() const;
	void Serialize(FArchive& Ar);

#if WITH_EDITOR
	static TSharedPtr<FVoxelStaticMeshData> VoxelizeMesh(
		const UStaticMesh& StaticMesh,
		int32 VoxelSize,
		float BoundsExtension,
		const FVoxelStaticMeshSettings& VoxelizerSettings);
#endif
};