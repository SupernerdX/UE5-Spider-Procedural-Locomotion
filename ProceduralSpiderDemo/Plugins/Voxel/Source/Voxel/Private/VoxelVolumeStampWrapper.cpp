// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelVolumeStampWrapper.h"
#include "VoxelQuery.h"
#include "VoxelBreadcrumbs.h"

extern VOXEL_API bool GVoxelEnableStampValidation;

void FVoxelVolumeStampWrapper::Apply(
	const FVoxelWeakStackLayer& Layer,
	const FVoxelVolumeStampRuntime& Stamp,
	const FVoxelVolumeBulkQuery& Query,
	const FVoxelVolumeTransform& StampToQuery)
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

	VOXEL_FUNCTION_COUNTER();

	const TVoxelArray<float> OldDistances = TVoxelArray<float>(Query.Distances);

	ApplyImpl(
		Layer,
		Stamp,
		Query,
		StampToQuery);

	for (int32 Index = 0; Index < OldDistances.Num(); Index++)
	{
		const float OldDistance = OldDistances[Index];
		const float NewDistance = Query.Distances[Index];

		if (FVoxelUtilities::IsNaN(NewDistance))
		{
			continue;
		}

		if (!Stamp.AffectShape() &&
			!Stamp.ShouldUseQueryPrevious())
		{
			ensure(OldDistance == NewDistance);
		}

		ensure(FVoxelUtilities::IsFinite(NewDistance));

		if (FVoxelUtilities::IsNaN(OldDistance))
		{
			// Allowed to go beyond MaxDistance if negative
			ensure(NewDistance <= StampToQuery.MaxDistance || Query.QueryPrevious);
		}
	}
}

void FVoxelVolumeStampWrapper::Apply(
	const FVoxelWeakStackLayer& Layer,
	const FVoxelVolumeStampRuntime& Stamp,
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery)
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

	const TVoxelArray<float> OldIndirectDistances = TVoxelArray<float>(Query.IndirectDistances);
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

	for (int32 Index = 0; Index < OldIndirectDistances.Num(); Index++)
	{
		const float OldDistance = OldIndirectDistances[Index];
		const float NewDistance = Query.IndirectDistances[Index];

		if (FVoxelUtilities::IsNaN(NewDistance))
		{
			continue;
		}

		if (!Stamp.AffectShape() &&
			!Stamp.ShouldUseQueryPrevious())
		{
			ensure(OldDistance == NewDistance);
		}

		ensure(FVoxelUtilities::IsFinite(NewDistance));

		if (FVoxelUtilities::IsNaN(OldDistance))
		{
			// Allowed to go beyond MaxDistance if negative
			ensure(NewDistance <= StampToQuery.MaxDistance || Query.QueryPrevious);
		}
	}

	if (!Query.bQuerySurfaceTypes)
	{
		ensure(Query.IndirectSurfaceTypes == OldIndirectSurfaceTypes);
	}

	if (!Stamp.ShouldUseQueryPrevious())
	{
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
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeStampWrapper::ApplyImpl(
	const FVoxelWeakStackLayer& Layer,
	const FVoxelVolumeStampRuntime& Stamp,
	const FVoxelVolumeBulkQuery& Query,
	const FVoxelVolumeTransform& StampToQuery)
{
	VOXEL_SCOPE_COUNTER_FORMAT("Apply x%d", Query.Num());

	if (FVoxelShouldCancel())
	{
		return;
	}

	if (Query.Query.Breadcrumbs &&
		Query.Query.Breadcrumbs->PreApplyStamp.BulkVolume)
	{
		Query.Query.Breadcrumbs->PreApplyStamp.BulkVolume(
			Layer,
			Stamp,
			Query);
	}

	Stamp.Apply(
		Query,
		StampToQuery);

	if (Query.Query.Breadcrumbs &&
		Query.Query.Breadcrumbs->PostApplyStamp.BulkVolume)
	{
		Query.Query.Breadcrumbs->PostApplyStamp.BulkVolume(
			Layer,
			Stamp,
			Query);
	}
}

void FVoxelVolumeStampWrapper::ApplyImpl(
	const FVoxelWeakStackLayer& Layer,
	const FVoxelVolumeStampRuntime& Stamp,
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery)
{
	VOXEL_SCOPE_COUNTER_FORMAT("Apply x%d", Query.Num());

	if (FVoxelShouldCancel())
	{
		return;
	}

	if (Query.Query.Breadcrumbs &&
		Query.Query.Breadcrumbs->PreApplyStamp.SparseVolume)
	{
		Query.Query.Breadcrumbs->PreApplyStamp.SparseVolume(
			Layer,
			Stamp,
			Query);
	}

	Stamp.Apply(
		Query,
		StampToQuery);

	if (Query.Query.Breadcrumbs &&
		Query.Query.Breadcrumbs->PostApplyStamp.SparseVolume)
	{
		Query.Query.Breadcrumbs->PostApplyStamp.SparseVolume(
			Layer,
			Stamp,
			Query);
	}
}