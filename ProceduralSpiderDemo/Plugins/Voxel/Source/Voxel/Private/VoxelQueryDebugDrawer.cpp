// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelQueryDebugDrawer.h"
#include "VoxelStampQuery.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelShowAllQueries, false,
	"voxel.ShowAllQueries",
	"If true will draw a debug point per queried voxel");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, float, GVoxelShowAllQueriesMaxTimeInNanoseconds, 1000,
	"voxel.ShowAllQueries.MaxTimeInNanoseconds",
	"Max time per voxel to use for debug colors");

void FVoxelQueryDebugDrawer::OnHeightLayerQuery(const FVoxelHeightBulkQuery& Query, const double Time)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDebugDrawer Drawer;
	Drawer.Color(GetColor(Time, Query.Indices.Count_int32()));

	for (int32 X = Query.Indices.Min.X; X < Query.Indices.Max.X; X++)
	{
		for (int32 Y = Query.Indices.Min.Y; Y < Query.Indices.Max.Y; Y++)
		{
			float Height = Query.Heights[Query.GetIndex(X, Y)];
			if (FVoxelUtilities::IsNaN(Height))
			{
				Height = 0;
			}

			Drawer.DrawPoint(FVector(Query.GetPosition(X, Y), Height));
		}
	}
}

void FVoxelQueryDebugDrawer::OnHeightLayerQuery(const FVoxelHeightSparseQuery& Query, const double Time)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDebugDrawer Drawer;
	Drawer.Color(GetColor(Time, Query.Num()));

	for (int32 Index = 0; Index < Query.Num(); Index++)
	{
		float Height = Query.IndirectHeights[Query.GetIndirectIndex(Index)];
		if (FVoxelUtilities::IsNaN(Height))
		{
			Height = 0;
		}

		Drawer.DrawPoint(FVector(Query.Positions[Index], Height));
	}
}

void FVoxelQueryDebugDrawer::OnVolumeLayerQuery(const FVoxelVolumeBulkQuery& Query, const double Time)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDebugDrawer Drawer;
	Drawer.Color(GetColor(Time, Query.Indices.Count_int32()));

	for (int32 X = Query.Indices.Min.X; X < Query.Indices.Max.X; X++)
	{
		for (int32 Y = Query.Indices.Min.Y; Y < Query.Indices.Max.Y; Y++)
		{
			for (int32 Z = Query.Indices.Min.Z; Z < Query.Indices.Max.Z; Z++)
			{
				Drawer.DrawPoint(Query.GetPosition(X, Y, Z));
			}
		}
	}
}

void FVoxelQueryDebugDrawer::OnVolumeLayerQuery(const FVoxelVolumeSparseQuery& Query, const double Time)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDebugDrawer Drawer;
	Drawer.Color(GetColor(Time, Query.Num()));

	for (int32 Index = 0; Index < Query.Num(); Index++)
	{
		Drawer.DrawPoint(Query.Positions[Index]);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FLinearColor FVoxelQueryDebugDrawer::GetColor(
	const double Time,
	const int32 NumVoxels)
{
	const double TimePerVoxel = Time * (1e9 / GVoxelShowAllQueriesMaxTimeInNanoseconds) / NumVoxels;
	const float Alpha = FMath::Clamp(TimePerVoxel, 0.f, 1.f);

	if (Alpha < 0.15f)
	{
		return FMath::Lerp(
			FLinearColor(0.0f, 1.0f, 0.2f, 1.0f),
			FLinearColor(0.5f, 1.0f, 0.0f, 1.0f),
			Alpha / 0.15f);
	}

	if (Alpha < 0.3f)
	{
		return FMath::Lerp(
			FLinearColor(0.5f, 1.0f, 0.0f, 1.0f),
			FLinearColor(1.0f, 1.0f, 0.0f, 1.0f),
			(Alpha - 0.15f) / 0.15f);
	}

	if (Alpha < 0.45f)
	{
		return FMath::Lerp(
			FLinearColor(1.0f, 1.0f, 0.0f, 1.0f),
			FLinearColor(1.0f, 0.7f, 0.0f, 1.0f),
			(Alpha - 0.3f) / 0.15f);
	}

	if (Alpha < 0.6f)
	{
		return FMath::Lerp(
			FLinearColor(1.0f, 0.7f, 0.0f, 1.0f),
			FLinearColor(1.0f, 0.3f, 0.0f, 1.0f),
			(Alpha - 0.45f) / 0.15f);
	}

	if (Alpha < 0.75f)
	{
		return FMath::Lerp(
			FLinearColor(1.0f, 0.3f, 0.0f, 1.0f),
			FLinearColor(1.0f, 0.0f, 0.0f, 1.0f),
			(Alpha - 0.6f) / 0.15f);
	}

	if (Alpha < 0.9f)
	{
		return FMath::Lerp(
			FLinearColor(1.0f, 0.0f, 0.0f, 1.0f),
			FLinearColor(0.5f, 0.0f, 0.8f, 1.0f),
			(Alpha - 0.75f) / 0.15f);
	}

	return FMath::Lerp(
		FLinearColor(0.5f, 0.0f, 0.8f, 1.0f),
		FLinearColor(0.0f, 0.0f, 0.0f, 1.0f),
		(Alpha - 0.9f) / 0.1f);
}
