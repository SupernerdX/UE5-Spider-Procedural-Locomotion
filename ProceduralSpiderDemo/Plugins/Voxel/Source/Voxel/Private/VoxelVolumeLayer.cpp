// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelVolumeLayer.h"
#include "VoxelVolumeStampWrapper.h"
#include "VoxelHeightLayer.h"
#include "VoxelQueryDebugDrawer.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelVolumeLayer);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelOptionalBox FVoxelVolumeLayer::GetBoundsToGenerate(FVoxelDependencyCollector& DependencyCollector) const
{
	FVoxelOptionalBox Bounds;
	if (PreviousHeightLayer)
	{
		check(!PreviousVolumeLayer);
		Bounds = PreviousHeightLayer->GetBoundsToGenerate(DependencyCollector);
	}
	else if (PreviousVolumeLayer)
	{
		Bounds = PreviousVolumeLayer->GetBoundsToGenerate(DependencyCollector);
	}

	if (Bounds.IsValid())
	{
		Bounds = Bounds->IntersectWith(IntersectBounds);
	}

	for (int32 LOD = 0; LOD < LODToTree.Num(); LOD++)
	{
		const FVoxelStampTree& Tree = GetTree(LOD);

		Tree.ForeachElement_Unsorted(
			DependencyCollector,
			FVoxelBox::Infinite,
			EVoxelStampBehavior::AffectShape,
			[&](const FVoxelStampTreeElement& Element)
			{
				Bounds += Element.Bounds;
			});
	}

	return Bounds;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelVolumeLayer::HasStamps(
	const FVoxelQuery& Query,
	const FVoxelBox& Bounds,
	const EVoxelStampBehavior BehaviorMask) const
{
	if (!IntersectBounds.Intersects(Bounds))
	{
		return false;
	}

	if (PreviousHeightLayer &&
		PreviousHeightLayer->HasStamps(
			Query,
			Bounds.IntersectWith(IntersectBounds),
			BehaviorMask,
			false))
	{
		return true;
	}

	if (PreviousVolumeLayer &&
		PreviousVolumeLayer->HasStamps(
			Query,
			Bounds.IntersectWith(IntersectBounds),
			BehaviorMask))
	{
		return true;
	}

	bool bIntersects = false;
	GetTree(Query.LOD).ForeachElement_Unsorted(
		Query.DependencyCollector,
		Bounds,
		BehaviorMask,
		[&](const FVoxelStampTreeElement& Element)
		{
			if (Element.Bounds.Intersects(Bounds))
			{
				bIntersects = true;
				return EVoxelIterate::Stop;
			}

			return EVoxelIterate::Continue;
		});

	return bIntersects;
}

bool FVoxelVolumeLayer::HasVolumeStamps(
	const FVoxelQuery& Query,
	const FVoxelBox& Bounds,
	const EVoxelStampBehavior BehaviorMask) const
{
	if (!IntersectBounds.Intersects(Bounds))
	{
		return false;
	}

	if (PreviousVolumeLayer &&
		PreviousVolumeLayer->HasVolumeStamps(
			Query,
			Bounds.IntersectWith(IntersectBounds),
			BehaviorMask))
	{
		return true;
	}

	bool bIntersects = false;
	GetTree(Query.LOD).ForeachElement_Unsorted(
		Query.DependencyCollector,
		Bounds,
		BehaviorMask,
		[&](const FVoxelStampTreeElement& Element)
		{
			if (Element.Bounds.Intersects(Bounds))
			{
				bIntersects = true;
				return EVoxelIterate::Stop;
			}

			return EVoxelIterate::Continue;
		});

	return bIntersects;
}

bool FVoxelVolumeLayer::HasIntersectStamps() const
{
	if (PreviousVolumeLayer &&
		PreviousVolumeLayer->HasIntersectStamps())
	{
		return true;
	}

	return !IntersectBounds.IsInfinite();
}

FVoxelOptionalBox FVoxelVolumeLayer::GetVolumeStampBounds(
	const FVoxelQuery& Query,
	const FVoxelBox& Bounds,
	const EVoxelStampBehavior BehaviorMask) const
{
	if (!IntersectBounds.Intersects(Bounds))
	{
		return {};
	}

	FVoxelOptionalBox Result;
	if (PreviousVolumeLayer)
	{
		Result += PreviousVolumeLayer->GetVolumeStampBounds(
			Query,
			Bounds.IntersectWith(IntersectBounds),
			BehaviorMask);
	}

	GetTree(Query.LOD).ForeachElement_Unsorted(
		Query.DependencyCollector,
		Bounds,
		BehaviorMask,
		[&](const FVoxelStampTreeElement& Element)
		{
			ensureVoxelSlow(Element.Bounds.IsValidAndNotEmpty());
			Result += Element.Bounds;
		});

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeLayer::Sample(const FVoxelVolumeBulkQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const double StartTime = FPlatformTime::Seconds();

	const TUniquePtr<FVoxelStampTree::FIterator> Iterator = GetTree(Query.Query.LOD).CreateIterator(
		Query.Query,
		Query.GetBounds());

	SampleStamps(
		Query.Query,
		Query.WithQuery(Query.Query.MakeChild_Layer(WeakLayer)),
		Iterator->Stamps);

	if (GVoxelShowAllQueries)
	{
		FVoxelQueryDebugDrawer::OnVolumeLayerQuery(Query, FPlatformTime::Seconds() - StartTime);
	}
}

void FVoxelVolumeLayer::Sample(const FVoxelVolumeSparseQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const double StartTime = FPlatformTime::Seconds();

	const TUniquePtr<FVoxelStampTree::FIterator> Iterator = GetTree(Query.Query.LOD).CreateIterator(
		Query.Query,
		Query.PositionBounds);

	SampleStamps(
		Query.Query,
		Query.WithQuery(Query.Query.MakeChild_Layer(WeakLayer)),
		Iterator->Stamps);

	if (GVoxelShowAllQueries)
	{
		FVoxelQueryDebugDrawer::OnVolumeLayerQuery(Query, FPlatformTime::Seconds() - StartTime);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename QueryType>
void FVoxelVolumeLayer::SampleStamps(
	const FVoxelQuery& PreviousQuery,
	const QueryType& Query,
	const TConstVoxelArrayView<FVoxelStampTree::FStamp> Stamps) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	for (int32 StampIndex = Stamps.Num() - 1; StampIndex >= 0; StampIndex--)
	{
		const FVoxelStampTree::FStamp& Stamp = Stamps[StampIndex];
		const FVoxelVolumeStampRuntime& TypedStamp = Stamp.GetStamp<FVoxelVolumeStampRuntime>();

		if (!TypedStamp.ShouldUseQueryPrevious())
		{
			continue;
		}

		TVoxelInlineArray<QueryType, 1> LocalQueries;
		LocalQueries.Add(Query);

		for (const FVoxelStampTreeElement* Element : Stamp.GetElements())
		{
			TVoxelInlineArray<QueryType, 1> NextQueries;
			for (const QueryType& LocalQuery : LocalQueries)
			{
				TVoxelOptional<QueryType> Inside;
				LocalQuery.Split(
					Element->Bounds,
					Inside,
					NextQueries);

				if (!Inside)
				{
					continue;
				}

				const FVoxelVolumeQueryPrevious QueryPrevious([&](const auto& InQuery)
				{
					VOXEL_SCOPE_COUNTER_NUM("Query previous", InQuery.Num());

					this->SampleStamps(
						InQuery.Query,
						InQuery,
						Stamps.LeftOf(StampIndex));
				});

				Inside->QueryPrevious = &QueryPrevious;

				FVoxelVolumeStampWrapper::Apply(
					WeakLayer,
					TypedStamp,
					*Inside,
					Element->VolumeStampToQuery);
			}
			LocalQueries = MoveTemp(NextQueries);
		}

		// Sample all the stamps before us, but only outside the override bounds
		for (const QueryType& LocalQuery : LocalQueries)
		{
			this->SampleStamps(
				PreviousQuery,
				LocalQuery,
				Stamps.LeftOf(StampIndex));
		}

		// Sample all the stamps after us everywhere
		if (StampIndex + 1 < Stamps.Num())
		{
			this->SampleStampsImpl(
				Query,
				Stamps.RightOf(StampIndex + 1));
		}

		return;
	}

	if (const TVoxelOptional<QueryType> ShrunkQuery = Query.ShrinkTo(IntersectBounds))
	{
		if (PreviousHeightLayer)
		{
			check(!PreviousVolumeLayer);

			PreviousHeightLayer->SampleAsVolume(ShrunkQuery->WithQuery(PreviousQuery));
		}
		else if (PreviousVolumeLayer)
		{
			PreviousVolumeLayer->Sample(ShrunkQuery->WithQuery(PreviousQuery));
		}
	}

	this->SampleStampsImpl(Query, Stamps);
}

template<typename QueryType>
void FVoxelVolumeLayer::SampleStampsImpl(
	const QueryType& Query,
	const TConstVoxelArrayView<FVoxelStampTree::FStamp> Stamps) const
{
	for (const FVoxelStampTree::FStamp& Stamp : Stamps)
	{
		checkVoxelSlow(!Stamp.GetStamp<FVoxelVolumeStampRuntime>().ShouldUseQueryPrevious());

		if (const FVoxelStampTreeElement* Element = Stamp.GetUniqueElement())
		{
			if (const TVoxelOptional<QueryType> ElementQuery = Query.ShrinkTo(Element->Bounds))
			{
				FVoxelVolumeStampWrapper::Apply(
					WeakLayer,
					Stamp.GetStamp<FVoxelVolumeStampRuntime>(),
					*ElementQuery,
					Element->VolumeStampToQuery);
			}

			continue;
		}

		TVoxelInlineArray<QueryType, 1> LocalQueries;
		LocalQueries.Add(Query);

		for (const FVoxelStampTreeElement* Element : Stamp.GetElements())
		{
			TVoxelInlineArray<QueryType, 1> NextQueries;
			for (const QueryType& LocalQuery : LocalQueries)
			{
				TVoxelOptional<QueryType> Inside;
				LocalQuery.Split(
					Element->Bounds,
					Inside,
					NextQueries);

				if (!Inside)
				{
					continue;
				}

				FVoxelVolumeStampWrapper::Apply(
					WeakLayer,
					Stamp.GetStamp<FVoxelVolumeStampRuntime>(),
					*Inside,
					Element->VolumeStampToQuery);
			}

			if (NextQueries.Num() == 0)
			{
				break;
			}

			LocalQueries = MoveTemp(NextQueries);
		}
	}
}