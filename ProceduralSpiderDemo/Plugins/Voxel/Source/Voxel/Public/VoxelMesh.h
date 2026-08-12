// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "Surface/VoxelSurfaceTypeBlend.h"

class FDistanceFieldVolumeData;
struct FVoxelSubsystem;
struct FVoxelChunkNeighborInfo;

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelMeshMemory, "Voxel Mesh Memory");

class VOXEL_API FVoxelMesh : public TSharedFromThis<FVoxelMesh>
{
public:
	const int32 ChunkLOD;
	const FInt64Vector ChunkOffset;
	const int32 ChunkSize;
	const FVoxelBox Bounds;
	const TVoxelArray<FVoxelSurfaceType> UsedSurfaceTypes;
	const TVoxelMap<FVoxelMetadataRef, TSharedRef<const FVoxelBuffer>> MetadataToBuffer;

	const TVoxelArray<int32> Indices;
	const TVoxelArray<FVector3f> Vertices;
	const TVoxelArray<FVoxelOctahedron> Normals;
	const TVoxelArray<FVoxelSurfaceTypeBlend> SurfaceTypes;

	// For cubic mesher: indicates this mesh contains translucent surfaces
	// and requires separate render pass with depth sorting
	const bool bIsTranslucent = false;

	TSharedPtr<FDistanceFieldVolumeData> DistanceFieldsData;

	struct FCell
	{
		int16 X = 0;
		int16 Y = 0;
		int16 Z = 0;
	};
	const TVoxelArray<FCell> Cells;

	struct FLOD
	{
		TVoxelArray<int32> VertexIndexToDisplacedVertexIndex;
		TVoxelArray<FVector3f> DisplacedVertices;
	};
	const TVoxelArray<FLOD> LODs;

	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelMeshMemory);

	FVoxelMesh(
		int32 ChunkLOD,
		const FInt64Vector& ChunkOffset,
		int32 ChunkSize,
		TVoxelArray<FVoxelSurfaceType>&& UsedSurfaceTypes,
		TVoxelMap<FVoxelMetadataRef, TSharedRef<const FVoxelBuffer>>&& MetadataToBuffer,
		TVoxelArray<int32>&& Indices,
		TVoxelArray<FVector3f>&& Vertices,
		TVoxelArray<FVoxelOctahedron>&& Normals,
		TVoxelArray<FVoxelSurfaceTypeBlend>&& SurfaceTypes,
		TVoxelArray<FCell>&& Cells,
		TVoxelArray<FLOD>&& LODs,
		bool bIsTranslucent = false);

	int64 GetAllocatedSize() const;

	TVoxelArray<FVector3f> GetDisplacedVertices(const FVoxelChunkNeighborInfo& NeighborInfo) const;
};