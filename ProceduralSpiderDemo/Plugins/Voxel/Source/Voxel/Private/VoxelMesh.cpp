// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMesh.h"
#include "VoxelBuffer.h"
#include "VoxelChunkKey.h"
#include "VoxelDistanceFieldWrapper.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelMeshMemory);

FVoxelMesh::FVoxelMesh(
	const int32 ChunkLOD,
	const FInt64Vector& ChunkOffset,
	const int32 ChunkSize,
	TVoxelArray<FVoxelSurfaceType>&& UsedSurfaceTypes,
	TVoxelMap<FVoxelMetadataRef, TSharedRef<const FVoxelBuffer>>&& MetadataToBuffer,
	TVoxelArray<int32>&& Indices,
	TVoxelArray<FVector3f>&& Vertices,
	TVoxelArray<FVoxelOctahedron>&& Normals,
	TVoxelArray<FVoxelSurfaceTypeBlend>&& SurfaceTypes,
	TVoxelArray<FCell>&& Cells,
	TVoxelArray<FLOD>&& LODs,
	const bool bIsTranslucent)
	: ChunkLOD(ChunkLOD)
	, ChunkOffset(ChunkOffset)
	, ChunkSize(ChunkSize)
	, Bounds(FVoxelBox::FromPositions(Vertices))
	, UsedSurfaceTypes(MoveTemp(UsedSurfaceTypes))
	, MetadataToBuffer(MoveTemp(MetadataToBuffer))
	, Indices(MoveTemp(Indices))
	, Vertices(MoveTemp(Vertices))
	, Normals(MoveTemp(Normals))
	, SurfaceTypes(MoveTemp(SurfaceTypes))
	, bIsTranslucent(bIsTranslucent)
	, Cells(MoveTemp(Cells))
	, LODs(MoveTemp(LODs))
{
	VOXEL_FUNCTION_COUNTER();
	ensure(this->Indices.Num() > 0);

	ConstCast(this->UsedSurfaceTypes).Shrink();
	ConstCast(this->MetadataToBuffer).Shrink();
	ConstCast(this->Indices).Shrink();
	ConstCast(this->Vertices).Shrink();
	ConstCast(this->Normals).Shrink();
	ConstCast(this->SurfaceTypes).Shrink();
	ConstCast(this->Cells).Shrink();
	ConstCast(this->LODs).Shrink();

	for (const FLOD& LOD : this->LODs)
	{
		ConstCast(LOD.VertexIndexToDisplacedVertexIndex).Shrink();
		ConstCast(LOD.DisplacedVertices).Shrink();
	}

	UpdateStats();
}

int64 FVoxelMesh::GetAllocatedSize() const
{
	VOXEL_FUNCTION_COUNTER();

	int64 AllocatedSize = 0;
	AllocatedSize += UsedSurfaceTypes.GetAllocatedSize();
	AllocatedSize += MetadataToBuffer.GetAllocatedSize();
	AllocatedSize += Indices.GetAllocatedSize();
	AllocatedSize += Vertices.GetAllocatedSize();
	AllocatedSize += Normals.GetAllocatedSize();
	AllocatedSize += SurfaceTypes.GetAllocatedSize();
	AllocatedSize += Cells.GetAllocatedSize();
	AllocatedSize += LODs.GetAllocatedSize();

	for (const auto& It : MetadataToBuffer)
	{
		AllocatedSize += It.Value->GetAllocatedSize();
	}

	for (const FLOD& LOD : LODs)
	{
		AllocatedSize += LOD.VertexIndexToDisplacedVertexIndex.GetAllocatedSize();
		AllocatedSize += LOD.DisplacedVertices.GetAllocatedSize();
	}

	if (DistanceFieldsData)
	{
		AllocatedSize += DistanceFieldsData->GetResourceSizeBytes();
	}

	return AllocatedSize;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelArray<FVector3f> FVoxelMesh::GetDisplacedVertices(const FVoxelChunkNeighborInfo& NeighborInfo) const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<FVector3f> Result = Vertices;

	for (int32 VertexIndex = 0; VertexIndex < Vertices.Num(); VertexIndex++)
	{
		const FCell Cell = Cells[VertexIndex];
		ensureVoxelSlow(0 <= Cell.X && Cell.X <= ChunkSize);
		ensureVoxelSlow(0 <= Cell.Y && Cell.Y <= ChunkSize);
		ensureVoxelSlow(0 <= Cell.Z && Cell.Z <= ChunkSize);

		const int32 VertexLOD = NeighborInfo.GetVertexLOD(ChunkLOD, ChunkSize, Cell.X, Cell.Y, Cell.Z);
		if (VertexLOD == ChunkLOD)
		{
			continue;
		}

		const int32 LODIndex = VertexLOD - ChunkLOD - 1;
		if (!ensureVoxelSlow(LODs.IsValidIndex(LODIndex)))
		{
			continue;
		}

		const FLOD& LOD = LODs[LODIndex];
		if (LOD.VertexIndexToDisplacedVertexIndex.Num() == 0)
		{
			// No parent cells
			continue;
		}

		const int32 DisplacedVertexIndex = LOD.VertexIndexToDisplacedVertexIndex[VertexIndex];
		if (DisplacedVertexIndex == -1)
		{
			continue;
		}

		Result[VertexIndex] = LOD.DisplacedVertices[DisplacedVertexIndex];
	}

	return Result;
}