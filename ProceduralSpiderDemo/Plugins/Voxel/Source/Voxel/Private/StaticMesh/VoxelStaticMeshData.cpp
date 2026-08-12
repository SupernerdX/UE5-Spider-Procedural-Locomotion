// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "StaticMesh/VoxelStaticMeshData.h"
#include "StaticMesh/VoxelMeshVoxelizer.h"
#include "StaticMeshAttributes.h"
#include "Engine/StaticMesh.h"
#include "Misc/ScopedSlowTask.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelVoxelizedMeshData);

int64 FVoxelStaticMeshData::GetAllocatedSize() const
{
	int64 AllocatedSize = sizeof(*this);
	AllocatedSize += DistanceField.GetAllocatedSize();
	AllocatedSize += PointIndices.GetAllocatedSize();
	AllocatedSize += Points.GetAllocatedSize();
	return AllocatedSize;
}

void FVoxelStaticMeshData::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion,
		RemoveMaxSmoothness,
		AddRanges,
		AddRangeMips,
		RemoveRangeMips,
		RemoveSerializedRangeChunkSize,
		AddMaxSmoothness,
		AddNormals,
		SwitchToFVoxelBox,
		AddPoints
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	if (Version < FVersion::AddPoints)
	{
		return;
	}

	Ar << MeshBounds;
	Ar << VoxelSize;
	Ar << BoundsExtension;
	Ar << VoxelizerSettings;

	Ar << Origin;
	Ar << Size;
	DistanceField.BulkSerialize(Ar);
	PointIndices.BulkSerialize(Ar);
	Points.BulkSerialize(Ar);

	UpdateStats();
}

#if WITH_EDITOR
TSharedPtr<FVoxelStaticMeshData> FVoxelStaticMeshData::VoxelizeMesh(
	const UStaticMesh& StaticMesh,
	const int32 VoxelSize,
	const float BoundsExtension,
	const FVoxelStaticMeshSettings& VoxelizerSettings)
{
	VOXEL_FUNCTION_COUNTER();

	FScopedSlowTask SlowTask(4.f);

	const FMeshDescription* MeshDescription = StaticMesh.GetMeshDescription(0);
	if (!ensure(MeshDescription))
	{
		VOXEL_MESSAGE(Error, "Failed to read vertex data from {0}", StaticMesh);
		return {};
	}

	SlowTask.EnterProgressFrame();

	TVoxelArray<FVector3f> Vertices;
	TVoxelArray<int32> Indices;
	TVoxelArray<FTriangleID> TriangleIds;
	{
		VOXEL_SCOPE_COUNTER("Copy mesh data");

		const FTriangleArray Triangles = MeshDescription->Triangles();
		const FStaticMeshConstAttributes Attributes(*MeshDescription);
		const TVertexAttributesConstRef<FVector3f> VertexPositions = Attributes.GetVertexPositions();

		FVoxelUtilities::SetNumFast(Vertices, VertexPositions.GetNumElements());
		Indices.Reserve(Triangles.Num() * 3);
		TriangleIds.Reserve(Triangles.Num());

		for (int32 Index = 0; Index < VertexPositions.GetNumElements(); Index++)
		{
			Vertices[Index] = VertexPositions[Index];
		}

		for (const FTriangleID TriangleID : Triangles.GetElementIDs())
		{
			const TConstVoxelArrayView<FVertexID> VertexIndices = MeshDescription->GetTriangleVertices(TriangleID);
			checkVoxelSlow(VertexIndices.Num() == 3);

			Indices.Add_EnsureNoGrow(VertexIndices[0]);
			Indices.Add_EnsureNoGrow(VertexIndices[1]);
			Indices.Add_EnsureNoGrow(VertexIndices[2]);

			TriangleIds.Add(TriangleID);
		}
	}

	SlowTask.EnterProgressFrame();

	FBox3f MeshBounds(ForceInit);
	for (FVector3f& Vertex : Vertices)
	{
		Vertex /= VoxelSize;
		MeshBounds += Vertex;
	}

	const FBox3f MeshBoundsWithSmoothness = MeshBounds.ExpandBy(MeshBounds.GetSize().GetMax() * BoundsExtension);

	// Minimum 4 for tricubic
	const FIntVector Size = FVoxelUtilities::ComponentMax(FVoxelUtilities::CeilToInt(MeshBoundsWithSmoothness.GetSize()), FIntVector(4));
	const FVector3f Origin = MeshBoundsWithSmoothness.Min;

	if (int64(Size.X) * int64(Size.Y) * int64(Size.Z) * sizeof(float) >= MAX_int32 / 2)
	{
		VOXEL_MESSAGE(Error, "{0}: Voxelized mesh would have more than 1GB of data", StaticMesh);
		return nullptr;
	}

	TVoxelArray<float> Distances;
	TVoxelArray<float> ClosestX;
	TVoxelArray<float> ClosestY;
	TVoxelArray<float> ClosestZ;
	TVoxelMap<FVector3f, FVoxelStaticMeshPoint> ClosestToPoint;
	int32 NumLeaks = 0;
	FVoxelMeshVoxelizer::Voxelize(
		VoxelizerSettings,
		Vertices,
		Indices,
		TriangleIds,
		Origin,
		Size,
		Distances,
		ClosestX,
		ClosestY,
		ClosestZ,
		ClosestToPoint,
		NumLeaks);

	SlowTask.EnterProgressFrame();

	// Propagate distances
	FVoxelUtilities::JumpFlood_Initialized(
		Size,
		Distances,
		ClosestX,
		ClosestY,
		ClosestZ);

	if (SlowTask.ShouldCancel())
	{
		return nullptr;
	}

	SlowTask.EnterProgressFrame();

	TVoxelArray<FVoxelStaticMeshPoint> Points;
	TVoxelArray<int32> PointIndices;
	{
		FScopedSlowTask InnerSlowTask(Distances.Num(), INVTEXT("Find closest positions"));

		Points.Reserve(Distances.Num());
		FVoxelUtilities::SetNumFast(PointIndices, Distances.Num());

		TVoxelMap<FVector3f, int32> ClosestToPointIndex;
		ClosestToPointIndex.Reserve(Distances.Num());

		for (int32 Index = 0; Index < Distances.Num(); Index++)
		{
			if (Index % 4096 == 4095)
			{
				InnerSlowTask.EnterProgressFrame(4096);

				if (InnerSlowTask.ShouldCancel())
				{
					break;
				}
			}

			const FVector3f Closest = FVector3f(
				ClosestX[Index],
				ClosestY[Index],
				ClosestZ[Index]);

			int32& PointIndex = ClosestToPointIndex.FindOrAdd_WithDefault(Closest, -1);
			if (PointIndex == -1)
			{
				FVoxelStaticMeshPoint Point;

				if (const FVoxelStaticMeshPoint* PointPtr = ClosestToPoint.Find(Closest))
				{
					Point = *PointPtr;
				}
				else
				{
					ensureVoxelSlow(false);
					Point = FVoxelStaticMeshPoint(0, FVector3f(1.f / 3.f));
				}

				PointIndex = Points.Add(Point);
			}
			PointIndices[Index] = PointIndex;
		}
	}

	if (SlowTask.ShouldCancel())
	{
		return nullptr;
	}

	const TSharedRef<FVoxelStaticMeshData> VoxelizedMeshData = MakeShared<FVoxelStaticMeshData>();
	VoxelizedMeshData->MeshBounds = FVoxelBox(MeshBounds);
	VoxelizedMeshData->VoxelSize = VoxelSize;
	VoxelizedMeshData->BoundsExtension = BoundsExtension;
	VoxelizedMeshData->VoxelizerSettings = VoxelizerSettings;

	VoxelizedMeshData->Origin = Origin;
	VoxelizedMeshData->Size = Size;
	VoxelizedMeshData->DistanceField = MoveTemp(Distances);
	VoxelizedMeshData->PointIndices = MoveTemp(PointIndices);
	VoxelizedMeshData->Points = MoveTemp(Points);

	VoxelizedMeshData->DistanceField.Shrink();
	VoxelizedMeshData->PointIndices.Shrink();
	VoxelizedMeshData->Points.Shrink();

	VoxelizedMeshData->UpdateStats();

	LOG_VOXEL(Log, "%s voxelized, %d leaks", *StaticMesh.GetPathName(), NumLeaks);

	if (SlowTask.ShouldCancel())
	{
		return nullptr;
	}

	return VoxelizedMeshData;
}
#endif