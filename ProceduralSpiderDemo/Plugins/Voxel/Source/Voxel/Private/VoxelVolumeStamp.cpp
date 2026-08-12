// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelVolumeStamp.h"
#include "VoxelVersion.h"
#include "Buffer/VoxelDoubleBuffers.h"

FVoxelVolumeStamp::FVoxelVolumeStamp()
{
	Layer = UVoxelVolumeLayer::Default();
}

void FVoxelVolumeStamp::FixupProperties()
{
	Super::FixupProperties();

	if (VoxelLayers.Num() > 0 &&
		ensure(Layer == UVoxelVolumeLayer::Default()) &&
		ensure(AdditionalLayers.Num() == 0))
	{
		Layer = VoxelLayers[0];
		VoxelLayers.RemoveAt(0);

		AdditionalLayers = MoveTemp(VoxelLayers);
	}
}

void FVoxelVolumeStamp::PostSerialize(const FArchive& Ar)
{
	Super::PostSerialize(Ar);

	if (Ar.CustomVer(GVoxelCustomVersionGUID) < FVoxelVersion::ChangeBoundsExtension)
	{
		BoundsExtensionMultiplier = BoundsExtension_DEPRECATED;
		MaximumBoundsExtension = 1e9;
	}
}

#if WITH_EDITOR
void FVoxelVolumeStamp::GetPropertyInfo(FPropertyInfo& Info) const
{
	Info.bIsSmoothnessVisible = BlendMode != EVoxelVolumeBlendMode::Override;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeStampRuntime::Apply(
	const FVoxelVolumeBulkQuery& Query,
	const FVoxelVolumeTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER("Slow Path");

	FVoxelDoubleVectorBuffer Positions;
	FVoxelFloatBuffer LocalDistances;
	{
		VOXEL_SCOPE_COUNTER("Expand positions");

		Positions.Allocate(Query.Num());
		LocalDistances.Allocate(Query.Num());

		int32 WriteIndex = 0;
		for (int32 IndexZ = Query.Indices.Min.Z; IndexZ < Query.Indices.Max.Z; IndexZ++)
		{
			for (int32 IndexY = Query.Indices.Min.Y; IndexY < Query.Indices.Max.Y; IndexY++)
			{
				for (int32 IndexX = Query.Indices.Min.X; IndexX < Query.Indices.Max.X; IndexX++)
				{
					const int32 ReadIndex = Query.GetIndex(IndexX, IndexY, IndexZ);

					Positions.Set(WriteIndex, Query.GetPosition(IndexX, IndexY, IndexZ));
					LocalDistances.Set(WriteIndex, Query.Distances[ReadIndex]);

					WriteIndex++;
				}
			}
		}
		check(WriteIndex == Query.Num());
	}

	FVoxelVolumeSparseQuery SparseQuery = FVoxelVolumeSparseQuery::Create(
		Query.Query,
		LocalDistances.View(),
		{},
		{},
		Positions,
		false,
		{});

	SparseQuery.QueryPrevious = Query.QueryPrevious;

	Apply(
		SparseQuery,
		StampToQuery);

	{
		VOXEL_SCOPE_COUNTER("Apply distances");

		int32 ReadIndex = 0;
		for (int32 IndexZ = Query.Indices.Min.Z; IndexZ < Query.Indices.Max.Z; IndexZ++)
		{
			for (int32 IndexY = Query.Indices.Min.Y; IndexY < Query.Indices.Max.Y; IndexY++)
			{
				for (int32 IndexX = Query.Indices.Min.X; IndexX < Query.Indices.Max.X; IndexX++)
				{
					const int32 WriteIndex = Query.GetIndex(IndexX, IndexY, IndexZ);

					Query.Distances[WriteIndex] = LocalDistances[ReadIndex];

					ReadIndex++;
				}
			}
		}
		check(ReadIndex == Query.Num());
	}
}