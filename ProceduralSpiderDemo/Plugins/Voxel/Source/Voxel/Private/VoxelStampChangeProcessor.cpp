// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampChangeProcessor.h"
#include "VoxelAABBTree.h"
#include "VoxelDependency.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelStampShowInvalidations, false,
	"voxel.ShowInvalidations",
	"Show the bounds invalidated by stamp changes");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampChangeProcessor::Process(const TVoxelChunkedArray<FVoxelStampChange>& StampChanges)
{
	checkStatic(sizeof(FVoxelStampChange) == sizeof(TVoxelStampChange<FVoxelHeightStampRuntime>));
	checkStatic(sizeof(FVoxelStampChange) == sizeof(TVoxelStampChange<FVoxelVolumeStampRuntime>));

	if (bIs2D)
	{
		ProcessImpl(ReinterpretCastRef<TVoxelChunkedArray<TVoxelStampChange<FVoxelHeightStampRuntime>>>(StampChanges));
	}
	else
	{
		ProcessImpl(ReinterpretCastRef<TVoxelChunkedArray<TVoxelStampChange<FVoxelVolumeStampRuntime>>>(StampChanges));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename RuntimeType>
void FVoxelStampChangeProcessor::ProcessImpl(const TVoxelChunkedArray<TVoxelStampChange<RuntimeType>>& StampChanges)
{
	VOXEL_FUNCTION_COUNTER_NUM(StampChanges.Num());

	for (const TVoxelStampChange<RuntimeType>& StampChange : StampChanges)
	{
		if (StampChange.ShouldInvalidateEverywhere())
		{
			for (const TSharedRef<FVoxelDependency3D>& Dependency : LODToDependency)
			{
				Dependency->Invalidate(FVoxelBox::Infinite);
			}
			return;
		}

		if (StampChange.TryToPartiallyInvalidate(*this))
		{
			// Used by sculpting & splines: only invalidate the bounds going beyond our original bounds
			// Sculpt dependency will handle invalidation within the sculpt bounds
			continue;
		}

		StampChange.Invalidate(*this);
	}

	Finalize();
}

void FVoxelStampChangeProcessor::Finalize()
{
	VOXEL_FUNCTION_COUNTER_NUM(Changes.Num());

	if (Changes.Num() == 0)
	{
		return;
	}

	if (bIs2D)
	{
		for (FChange& Change : Changes)
		{
			Change.Bounds.Min.Z -= StackMaxDistance;
			Change.Bounds.Max.Z += StackMaxDistance;
		}
	}

	if (GVoxelStampShowInvalidations)
	{
		for (const FChange& Change : Changes)
		{
			FVoxelDebugDrawer()
			.Color(FLinearColor::Blue)
			.LifeTime(1.f)
			.DrawBox(Change.Bounds, QueryToWorld);
		}
	}

	TVoxelInlineArray<bool, 32> ShouldSplit;
	{
		VOXEL_SCOPE_COUNTER("ShouldSplit");

		FVoxelUtilities::SetNumZeroed(ShouldSplit, LODToDependency.Num() + 1);

		for (const FChange& Change : Changes)
		{
			ShouldSplit[Change.LODRange.Min] = true;
			ShouldSplit[Change.LODRange.Max + 1] = true;
		}
	}

	TSharedPtr<FVoxelAABBTree> Tree;
	for (int32 LOD = 0; LOD < LODToDependency.Num(); LOD++)
	{
		if (!Tree ||
			ShouldSplit[LOD])
		{
			Tree = BuildTree(LOD);
		}

		LODToDependency[LOD]->Invalidate(Tree.ToSharedRef());
	}
}

TSharedRef<FVoxelAABBTree> FVoxelStampChangeProcessor::BuildTree(const int32 LOD) const
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER_FORMAT("LOD=%d", LOD);

	FVoxelAABBTree::FElementArray Elements;
	Elements.SetNum(Changes.Num());

	int32 Index = 0;

	for (const FChange& Change : Changes)
	{
		if (!Change.LODRange.Contains(LOD))
		{
			continue;
		}

		Elements.Set(
			Index,
			Change.Bounds,
			Index);

		Index++;
	}

	Elements.SetNum(Index);

	return FVoxelAABBTree::Create(MoveTemp(Elements));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampChangeProcessor::Invalidate(
	const FVoxelHeightStampRuntime& Stamp,
	const TConstVoxelArrayView<FVoxelBox> LocalBoundsToInvalidate)
{
	const FVoxelHeightTransform StampToQuery = FVoxelHeightTransform::Create(
		Stamp.GetLocalToWorld(),
		WorldToQuery,
		Stamp.GetLocalBounds(),
		Stamp.GetStamp().HeightPaddingMultiplier);

	for (const FVoxelBox& LocalBounds : LocalBoundsToInvalidate)
	{
		FVoxelBox Bounds = StampToQuery.GetBounds(LocalBounds);

		// TODO More granular?
		if (Stamp.HasRelativeHeightRange())
		{
			Bounds = FVoxelBox2D(Bounds).ToBox3D_Infinite();
		}

		Changes.Add(FChange
		{
			Bounds,
			Stamp.GetLODRange()
		});
	}
}

void FVoxelStampChangeProcessor::Invalidate(
	const FVoxelVolumeStampRuntime& Stamp,
	const TConstVoxelArrayView<FVoxelBox> LocalBoundsToInvalidate)
{
	const FVoxelVolumeTransform StampToQuery = FVoxelVolumeTransform::Create(
		Stamp.GetLocalToWorld(),
		WorldToQuery,
		Stamp.GetLocalBounds(),
		Stamp.GetStamp().BoundsExtensionMultiplier,
		Stamp.GetStamp().MaximumBoundsExtension);

	for (const FVoxelBox& LocalBounds : LocalBoundsToInvalidate)
	{
		Changes.Add(FChange
		{
			StampToQuery.GetBounds(LocalBounds),
			Stamp.GetLODRange()
		});
	}
}