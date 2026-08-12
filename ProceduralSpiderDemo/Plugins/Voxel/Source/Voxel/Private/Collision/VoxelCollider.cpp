// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Collision/VoxelCollider.h"
#include "VoxelMesh.h"
#include "VoxelChaosTriangleMeshCooker.h"
#include "Chaos/TriangleMeshImplicitObject.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelColliderMemory);

FVoxelCollider::~FVoxelCollider()
{
}

int64 FVoxelCollider::GetAllocatedSize() const
{
	int64 AllocatedSize = sizeof(*this);
	AllocatedSize += FVoxelChaosTriangleMeshCooker::GetAllocatedSize(*TriangleMesh);
	AllocatedSize += SurfaceTypes.GetAllocatedSize();
	return AllocatedSize;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelCollider> FVoxelCollider::Create(const FVoxelMesh& Mesh)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<uint16> FaceMaterials;
	TVoxelArray<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>> SurfaceTypes;
	BuildSurfaceTypes(
		Mesh,
		FaceMaterials,
		SurfaceTypes);

	const TRefCountPtr<Chaos::FTriangleMeshImplicitObject> TriangleMesh = FVoxelChaosTriangleMeshCooker::Create(
		Mesh.Indices,
		Mesh.Vertices,
		FaceMaterials);

	if (!TriangleMesh)
	{
		return nullptr;
	}

	return MakeShared<FVoxelCollider>(
		TriangleMesh,
		SurfaceTypes);
}

void FVoxelCollider::BuildSurfaceTypes(
	const FVoxelMesh& Mesh,
	TVoxelArray<uint16>& OutFaceMaterials,
	TVoxelArray<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>>& OutSurfaceTypes)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(Mesh.Indices.Num() % 3 == 0);
	const int32 NumTriangles = Mesh.Indices.Num() / 3;

	FVoxelUtilities::SetNumFast(OutFaceMaterials, NumTriangles);

	TVoxelMap<FVoxelSurfaceType, uint16> MaterialIndexToArrayIndex;
	MaterialIndexToArrayIndex.Reserve(32);

	for (int32 TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		const int32 IndexA = Mesh.Indices[3 * TriangleIndex + 0];
		const int32 IndexB = Mesh.Indices[3 * TriangleIndex + 1];
		const int32 IndexC = Mesh.Indices[3 * TriangleIndex + 2];

		const FVoxelSurfaceTypeBlend& MaterialA = Mesh.SurfaceTypes[IndexA];
		const FVoxelSurfaceTypeBlend& MaterialB = Mesh.SurfaceTypes[IndexB];
		const FVoxelSurfaceTypeBlend& MaterialC = Mesh.SurfaceTypes[IndexC];

		const FVoxelSurfaceTypeBlendLayer TopLayerA = MaterialA.GetTopLayer();
		const FVoxelSurfaceTypeBlendLayer TopLayerB = MaterialB.GetTopLayer();
		const FVoxelSurfaceTypeBlendLayer TopLayerC = MaterialC.GetTopLayer();

		const FVoxelSurfaceType SurfaceType =
			TopLayerA.Weight > TopLayerB.Weight && TopLayerA.Weight > TopLayerC.Weight
			? TopLayerA.Type
			: TopLayerB.Weight > TopLayerC.Weight
			? TopLayerB.Type
			: TopLayerC.Type;

		if (const uint16* PhysicalMaterialIndex = MaterialIndexToArrayIndex.Find(SurfaceType))
		{
			OutFaceMaterials[TriangleIndex] = *PhysicalMaterialIndex;
			continue;
		}

		const int32 ArrayIndex = OutSurfaceTypes.Add(SurfaceType.GetSurfaceTypeAsset());
		checkVoxelSlow(FVoxelUtilities::IsValidUINT16(ArrayIndex));

		MaterialIndexToArrayIndex.Add_CheckNew(SurfaceType, uint16(ArrayIndex));
		OutFaceMaterials[TriangleIndex] = uint16(ArrayIndex);
	}

	OutFaceMaterials.Shrink();
	OutSurfaceTypes.Shrink();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelCollider::FVoxelCollider(
	const TRefCountPtr<Chaos::FTriangleMeshImplicitObject>& TriangleMesh,
	const TVoxelArray<TVoxelObjectPtr<UVoxelSurfaceTypeAsset>>& SurfaceTypes)
	: TriangleMesh(TriangleMesh)
	, SurfaceTypes(SurfaceTypes)
{
	check(TriangleMesh);
}