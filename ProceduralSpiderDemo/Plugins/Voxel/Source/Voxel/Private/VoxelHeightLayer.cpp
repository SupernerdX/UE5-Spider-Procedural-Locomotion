// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelHeightLayer.h"
#include "VoxelHeightStamp.h"
#include "VoxelHeightStampWrapper.h"
#include "VoxelHeightLayerImpl.ispc.generated.h"
#include "VoxelQueryDebugDrawer.h"
#include "Buffer/VoxelBaseBuffers.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelHeightLayer);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelOptionalBox FVoxelHeightLayer::GetBoundsToGenerate(FVoxelDependencyCollector& DependencyCollector) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelOptionalBox Bounds;
	if (PreviousLayer)
	{
		Bounds = PreviousLayer->GetBoundsToGenerate(DependencyCollector);
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

bool FVoxelHeightLayer::HasStamps(
	const FVoxelQuery& Query,
	const FVoxelBox& Bounds,
	const EVoxelStampBehavior BehaviorMask,
	const bool bExtendByMaxDistance) const
{
	VOXEL_FUNCTION_COUNTER();

	if (PreviousLayer &&
		PreviousLayer->HasStamps(Query, Bounds, BehaviorMask, bExtendByMaxDistance))
	{
		return true;
	}

	FVoxelInterval RangeZ = FVoxelInterval::InvertedInfinite;

	GetTree(Query.LOD).ForeachElement_Unsorted(
		Query.DependencyCollector,
		Bounds.Extend(FVector(0, 0, bExtendByMaxDistance ? MaxDistance : 0.f)),
		BehaviorMask,
		[&](const FVoxelStampTreeElement& Element)
		{
			RangeZ += Element.Bounds.GetZ();
		});

	if (!RangeZ.IsValid())
	{
		return false;
	}

	return Bounds.GetZ().Intersects(RangeZ.Extend(bExtendByMaxDistance ? MaxDistance : 0.f));
}

FVoxelOptionalBox2D FVoxelHeightLayer::GetStampBounds(
	const FVoxelQuery& Query,
	const FVoxelBox2D& Bounds,
	const EVoxelStampBehavior BehaviorMask) const
{
	FVoxelOptionalBox2D Result;
	if (PreviousLayer)
	{
		Result += PreviousLayer->GetStampBounds(Query, Bounds, BehaviorMask);
	}

	GetTree(Query.LOD).ForeachElement_Unsorted(
		Query.DependencyCollector,
		Bounds.ToBox3D_Infinite(),
		BehaviorMask,
		[&](const FVoxelStampTreeElement& Element)
		{
			Result += FVoxelBox2D(Element.Bounds);
		});

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightLayer::Sample(const FVoxelHeightBulkQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const double StartTime = FPlatformTime::Seconds();

	const TUniquePtr<FVoxelStampTree::FIterator> Iterator = GetTree(Query.Query.LOD).CreateIterator(
		Query.Query,
		Query.GetBounds().ToBox3D_Infinite());

	SampleStamps(
		Query.Query,
		Query.WithQuery(Query.Query.MakeChild_Layer(WeakLayer)),
		Iterator->Stamps);

	if (GVoxelShowAllQueries)
	{
		FVoxelQueryDebugDrawer::OnHeightLayerQuery(Query, FPlatformTime::Seconds() - StartTime);
	}
}

void FVoxelHeightLayer::Sample(const FVoxelHeightSparseQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const double StartTime = FPlatformTime::Seconds();

	const TUniquePtr<FVoxelStampTree::FIterator> Iterator = GetTree(Query.Query.LOD).CreateIterator(
		Query.Query,
		Query.PositionBounds.ToBox3D_Infinite());

	SampleStamps(
		Query.Query,
		Query.WithQuery(Query.Query.MakeChild_Layer(WeakLayer)),
		Iterator->Stamps);

	if (GVoxelShowAllQueries)
	{
		FVoxelQueryDebugDrawer::OnHeightLayerQuery(Query, FPlatformTime::Seconds() - StartTime);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename QueryType>
void FVoxelHeightLayer::SampleStamps(
	const FVoxelQuery& PreviousQuery,
	const QueryType& Query,
	const TConstVoxelArrayView<FVoxelStampTree::FStamp> Stamps) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	for (int32 StampIndex = Stamps.Num() - 1; StampIndex >= 0; StampIndex--)
	{
		const FVoxelStampTree::FStamp& Stamp = Stamps[StampIndex];
		const FVoxelHeightStampRuntime& TypedStamp = Stamp.GetStamp<FVoxelHeightStampRuntime>();

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
					FVoxelBox2D(Element->Bounds),
					Inside,
					NextQueries);

				if (!Inside)
				{
					continue;
				}

				const FVoxelHeightQueryPrevious QueryPrevious([&](const auto& InQuery)
				{
					VOXEL_SCOPE_COUNTER_NUM("Query previous", InQuery.Num());

					this->SampleStamps(
						InQuery.Query,
						InQuery,
						Stamps.LeftOf(StampIndex));
				});

				Inside->QueryPrevious = &QueryPrevious;

				FVoxelHeightStampWrapper::Apply(
					WeakLayer,
					TypedStamp,
					*Inside,
					Element->HeightStampToQuery);
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

	if (PreviousLayer)
	{
		PreviousLayer->Sample(Query.WithQuery(PreviousQuery));
	}

	this->SampleStampsImpl(Query, Stamps);
}

template<typename QueryType>
void FVoxelHeightLayer::SampleStampsImpl(
	const QueryType& Query,
	const TConstVoxelArrayView<FVoxelStampTree::FStamp> Stamps) const
{
	for (const FVoxelStampTree::FStamp& Stamp : Stamps)
	{
		checkVoxelSlow(!Stamp.GetStamp<FVoxelHeightStampRuntime>().ShouldUseQueryPrevious());

		if (const FVoxelStampTreeElement* Element = Stamp.GetUniqueElement())
		{
			if (const TVoxelOptional<QueryType> ElementQuery = Query.ShrinkTo(FVoxelBox2D(Element->Bounds)))
			{
				FVoxelHeightStampWrapper::Apply(
					WeakLayer,
					Stamp.GetStamp<FVoxelHeightStampRuntime>(),
					*ElementQuery,
					Element->HeightStampToQuery);
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
					FVoxelBox2D(Element->Bounds),
					Inside,
					NextQueries);

				if (!Inside)
				{
					continue;
				}

				FVoxelHeightStampWrapper::Apply(
					WeakLayer,
					Stamp.GetStamp<FVoxelHeightStampRuntime>(),
					*Inside,
					Element->HeightStampToQuery);
			}

			if (NextQueries.Num() == 0)
			{
				break;
			}

			LocalQueries = MoveTemp(NextQueries);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightLayer::SampleAsVolume(const FVoxelVolumeBulkQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const FIntPoint Size2D = FVoxelIntBox2D(Query.Indices).Size();
	const FVector2d Start2D = FVector2d(Query.Start + FVector(Query.Indices.Min) * Query.Step);

	if (!HasStamps(
		Query.Query,
		Query.GetBounds(),
		// Bulk query has no metadata
		EVoxelStampBehavior::AffectShape,
		true))
	{
		if (VOXEL_DEBUG)
		{
			FVoxelFloatBuffer Heights;
			Heights.Allocate(Size2D.X * Size2D.Y);
			Heights.SetAll(FVoxelUtilities::NaNf());

			Sample(FVoxelHeightBulkQuery::Create(
				Query.Query,
				Heights.View(),
				Start2D,
				Size2D,
				Query.Step));

			const FVoxelBox Bounds = Query.GetBounds();

			for (const float Height : Heights)
			{
				if (FVoxelUtilities::IsNaN(Height))
				{
					continue;
				}

				ensure(FMath::Abs(Bounds.Min.Z - Height) > MaxDistance);
				ensure(FMath::Abs(Bounds.Max.Z - Height) > MaxDistance);
			}
		}

		return;
	}

	FVoxelFloatBuffer Heights;
	Heights.Allocate(Size2D.X * Size2D.Y);

	if (Query.QueryHeights &&
		Query.QueryHeights(Heights.View(), FVoxelIntBox2D(Query.Indices)))
	{
		if (VOXEL_DEBUG)
		{
			FVoxelFloatBuffer LocalHeights;
			LocalHeights.Allocate(Size2D.X * Size2D.Y);
			LocalHeights.SetAll(FVoxelUtilities::NaNf());

			Sample(FVoxelHeightBulkQuery::Create(
				Query.Query,
				LocalHeights.View(),
				Start2D,
				Size2D,
				Query.Step));

			for (int32 Index = 0; Index < Heights.Num(); Index++)
			{
				if (FVoxelUtilities::IntBits(Heights[Index]) == FVoxelUtilities::IntBits(LocalHeights[Index]))
				{
					continue;
				}

				if (FVoxelShouldCancel())
				{
					continue;
				}

				ensure(false);
			}
		}
	}
	else
	{
		Heights.SetAll(FVoxelUtilities::NaNf());

		Sample(FVoxelHeightBulkQuery::Create(
			Query.Query,
			Heights.View(),
			Start2D,
			Size2D,
			Query.Step));
	}

	VOXEL_SCOPE_COUNTER_NUM("HeightToVolume_Dense", Query.Num());

	ispc::VoxelHeightLayer_HeightToVolume_Dense(
		Query.ISPC(),
		Heights.GetData(),
		MaxDistance);
}

void FVoxelHeightLayer::SampleAsVolume(const FVoxelVolumeSparseQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	EVoxelStampBehavior Behavior = EVoxelStampBehavior::AffectShape;
	if (Query.bQuerySurfaceTypes)
	{
		Behavior |= EVoxelStampBehavior::AffectSurfaceType;
	}
	if (Query.MetadatasToQuery.Num() > 0)
	{
		Behavior |= EVoxelStampBehavior::AffectMetadata;
	}

	if (!HasStamps(
		Query.Query,
		Query.PositionBounds,
		Behavior,
		true))
	{
		if (VOXEL_DEBUG)
		{
			FVoxelFloatBuffer Heights;
			Heights.Allocate(Query.Num());
			Heights.SetAll(FVoxelUtilities::NaNf());

			FVoxelDoubleVector2DBuffer Positions2D;
			Positions2D.X = Query.Positions.X;
			Positions2D.Y = Query.Positions.Y;

			Sample(FVoxelHeightSparseQuery::Create(
				Query.Query,
				Heights.View(),
				{},
				{},
				Positions2D,
				false,
				{}));

			for (const float Height : Heights)
			{
				if (FVoxelUtilities::IsNaN(Height))
				{
					continue;
				}

				ensure(FMath::Abs(Query.PositionBounds.Min.Z - Height) > MaxDistance);
				ensure(FMath::Abs(Query.PositionBounds.Max.Z - Height) > MaxDistance);
			}
		}

		return;
	}

	FVoxelFloatBuffer IndirectHeights;
	IndirectHeights.Allocate(Query.IndirectDistances.Num());
	IndirectHeights.SetAll(FVoxelUtilities::NaNf());

	FVoxelDoubleVector2DBuffer Positions2D;
	Positions2D.X = Query.Positions.X;
	Positions2D.Y = Query.Positions.Y;

	ensure(!Query.QueryPrevious);

	const FVoxelHeightSparseQuery SparseQuery2D
	{
		Query.Query,
		Query.IndirectSurfaceTypes,
		Query.IndirectMetadata,
		Query.Indirection,
		Query.bQuerySurfaceTypes,
		Query.MetadatasToQuery,
		IndirectHeights.View(),
		Positions2D,
		FVoxelBox2D(Query.PositionBounds),
		{}
	};

	Sample(SparseQuery2D);

	VOXEL_SCOPE_COUNTER_NUM("HeightToVolume_Sparse", Query.Num());

	ispc::VoxelHeightLayer_HeightToVolume_Sparse(
		Query.ISPC(),
		IndirectHeights.GetData(),
		MaxDistance);
}