// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelHeightStamp.h"
#include "VoxelVersion.h"
#include "Buffer/VoxelDoubleBuffers.h"

FVoxelHeightStamp::FVoxelHeightStamp()
{
	Layer = UVoxelHeightLayer::Default();
}

void FVoxelHeightStamp::FixupProperties()
{
	Super::FixupProperties();

	if (VoxelLayers.Num() > 0 &&
		ensure(Layer == UVoxelHeightLayer::Default()) &&
		ensure(AdditionalLayers.Num() == 0))
	{
		Layer = VoxelLayers[0];
		VoxelLayers.RemoveAt(0);

		AdditionalLayers = MoveTemp(VoxelLayers);
	}
}

void FVoxelHeightStamp::PostSerialize(const FArchive& Ar)
{
	Super::PostSerialize(Ar);

	if (Ar.CustomVer(GVoxelCustomVersionGUID) < FVoxelVersion::ChangeBoundsExtension)
	{
		HeightPaddingMultiplier = BoundsExtension_DEPRECATED;
	}
}

#if WITH_EDITOR
void FVoxelHeightStamp::GetPropertyInfo(FPropertyInfo& Info) const
{
	Info.bIsSmoothnessVisible = BlendMode != EVoxelHeightBlendMode::Override;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightStampRuntime::Apply(
	const FVoxelHeightBulkQuery& Query,
	const FVoxelHeightTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER("Slow Path");

	FVoxelDoubleVector2DBuffer Positions;
	FVoxelFloatBuffer LocalHeights;
	{
		VOXEL_SCOPE_COUNTER("Expand positions");

		Positions.Allocate(Query.Num());
		LocalHeights.Allocate(Query.Num());

		int32 WriteIndex = 0;
		for (int32 IndexY = Query.Indices.Min.Y; IndexY < Query.Indices.Max.Y; IndexY++)
		{
			for (int32 IndexX = Query.Indices.Min.X; IndexX < Query.Indices.Max.X; IndexX++)
			{
				const int32 ReadIndex = Query.GetIndex(IndexX, IndexY);

				Positions.Set(WriteIndex, Query.GetPosition(IndexX, IndexY));
				LocalHeights.Set(WriteIndex, Query.Heights[ReadIndex]);

				WriteIndex++;
			}
		}
		check(WriteIndex == Query.Num());
	}

	FVoxelHeightSparseQuery SparseQuery = FVoxelHeightSparseQuery::Create(
		Query.Query,
		LocalHeights.View(),
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
		VOXEL_SCOPE_COUNTER("Apply heights");

		int32 ReadIndex = 0;
		for (int32 IndexY = Query.Indices.Min.Y; IndexY < Query.Indices.Max.Y; IndexY++)
		{
			for (int32 IndexX = Query.Indices.Min.X; IndexX < Query.Indices.Max.X; IndexX++)
			{
				const int32 WriteIndex = Query.GetIndex(IndexX, IndexY);

				Query.Heights[WriteIndex] = LocalHeights[ReadIndex];

				ReadIndex++;
			}
		}
		check(ReadIndex == Query.Num());
	}
}