// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelHeightStampWrapper.h"
#include "VoxelHeightStamp.h"
#include "VoxelQuery.h"
#include "VoxelBreadcrumbs.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelEnableStampValidation, VOXEL_DEBUG,
	"voxel.stamp.EnableValidation",
	"If true, will run additional checks to ensure stamps aren't writing data outside their bounds. Useful to debug holes in the terrain.");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightStampWrapper::Apply(
	const FVoxelWeakStackLayer& Layer,
	const FVoxelHeightStampRuntime& Stamp,
	const FVoxelHeightBulkQuery& Query,
	const FVoxelHeightTransform& StampToQuery)
{
	if (!GVoxelEnableStampValidation)
	{
		ApplyImpl(
			Layer,
			Stamp,
			Query,
			StampToQuery);

		return;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const TVoxelArray<float> OldHeights = TVoxelArray<float>(Query.Heights);

	ApplyImpl(
		Layer,
		Stamp,
		Query,
		StampToQuery);

	for (int32 Index = 0; Index < Query.Heights.Num(); Index++)
	{
		const float OldHeight = OldHeights[Index];
		const float NewHeight = Query.Heights[Index];

		if (FVoxelUtilities::IsNaN(NewHeight))
		{
			continue;
		}

		ensure(FVoxelUtilities::IsFinite(NewHeight));

		if (OldHeight != NewHeight)
		{
			ensure(StampToQuery.MinHeight <= NewHeight && NewHeight <= StampToQuery.MaxHeight);
		}
	}

	if (!Stamp.ShouldUseQueryPrevious())
	{
		if (!Stamp.AffectShape())
		{
			ensure(Query.Heights == OldHeights);
		}
	}
}

void FVoxelHeightStampWrapper::Apply(
	const FVoxelWeakStackLayer& Layer,
	const FVoxelHeightStampRuntime& Stamp,
	const FVoxelHeightSparseQuery& Query,
	const FVoxelHeightTransform& StampToQuery)
{
	if (!GVoxelEnableStampValidation)
	{
		ApplyImpl(
			Layer,
			Stamp,
			Query,
			StampToQuery);

		return;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const TVoxelArray<float> OldIndirectHeights = TVoxelArray<float>(Query.IndirectHeights);
	const TVoxelArray<FVoxelSurfaceTypeBlend> OldIndirectSurfaceTypes = Query.IndirectSurfaceTypes.Array();

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
	if (!Stamp.AffectMetadata())
	{
		for (const auto& It : Query.IndirectMetadata.MetadataToBuffer)
		{
			MetadataToBuffer.Add_EnsureNew(It.Key, It.Value->MakeDeepCopy());
		}
	}

	ApplyImpl(
		Layer,
		Stamp,
		Query,
		StampToQuery);

	for (const float Height : Query.IndirectHeights)
	{
		if (!FVoxelUtilities::IsNaN(Height))
		{
			ensure(FMath::IsFinite(Height));
		}
	}

	if (!Query.bQuerySurfaceTypes)
	{
		ensure(Query.IndirectSurfaceTypes == OldIndirectSurfaceTypes);
	}

	if (!Stamp.ShouldUseQueryPrevious())
	{
		if (!Stamp.AffectShape())
		{
			ensure(Query.IndirectHeights == OldIndirectHeights);
		}

		if (!Stamp.AffectSurfaceType())
		{
			ensure(Query.IndirectSurfaceTypes == OldIndirectSurfaceTypes);
		}

		if (!Stamp.AffectMetadata())
		{
			for (const auto& It : Query.IndirectMetadata.MetadataToBuffer)
			{
				ensure(It.Value->Equal(*MetadataToBuffer[It.Key]));
			}
		}

		for (const auto& It : Query.IndirectMetadata.MetadataToBuffer)
		{
			if (!Query.MetadatasToQuery.Contains(It.Key))
			{
				ensure(It.Value->Equal(*MetadataToBuffer[It.Key]));
			}
		}
	}

	for (int32 Index = 0; Index < OldIndirectHeights.Num(); Index++)
	{
		const float OldHeight = OldIndirectHeights[Index];
		const float NewHeight = Query.IndirectHeights[Index];

		if (FVoxelUtilities::IsNaN(NewHeight))
		{
			continue;
		}

		if (OldHeight != NewHeight)
		{
			ensure(StampToQuery.MinHeight <= NewHeight && NewHeight <= StampToQuery.MaxHeight);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightStampWrapper::ApplyImpl(
	const FVoxelWeakStackLayer& Layer,
	const FVoxelHeightStampRuntime& Stamp,
	const FVoxelHeightBulkQuery& Query,
	const FVoxelHeightTransform& StampToQuery)
{
	VOXEL_SCOPE_COUNTER_FORMAT("Apply x%d", Query.Num());

	if (FVoxelShouldCancel())
	{
		return;
	}

	if (Query.Query.Breadcrumbs &&
		Query.Query.Breadcrumbs->PreApplyStamp.BulkHeight)
	{
		Query.Query.Breadcrumbs->PreApplyStamp.BulkHeight(
			Layer,
			Stamp,
			Query);
	}

	Stamp.Apply(
		Query,
		StampToQuery);

	if (Query.Query.Breadcrumbs &&
		Query.Query.Breadcrumbs->PostApplyStamp.BulkHeight)
	{
		Query.Query.Breadcrumbs->PostApplyStamp.BulkHeight(
			Layer,
			Stamp,
			Query);
	}
}

void FVoxelHeightStampWrapper::ApplyImpl(
	const FVoxelWeakStackLayer& Layer,
	const FVoxelHeightStampRuntime& Stamp,
	const FVoxelHeightSparseQuery& Query,
	const FVoxelHeightTransform& StampToQuery)
{
	VOXEL_SCOPE_COUNTER_FORMAT("Apply x%d", Query.Num());

	if (FVoxelShouldCancel())
	{
		return;
	}

	if (Query.Query.Breadcrumbs &&
		Query.Query.Breadcrumbs->PreApplyStamp.SparseHeight)
	{
		Query.Query.Breadcrumbs->PreApplyStamp.SparseHeight(
			Layer,
			Stamp,
			Query);
	}

	Stamp.Apply(
		Query,
		StampToQuery);

	if (Query.Query.Breadcrumbs &&
		Query.Query.Breadcrumbs->PostApplyStamp.SparseHeight)
	{
		Query.Query.Breadcrumbs->PostApplyStamp.SparseHeight(
			Layer,
			Stamp,
			Query);
	}
}