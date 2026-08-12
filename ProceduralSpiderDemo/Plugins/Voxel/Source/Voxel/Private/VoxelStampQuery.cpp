// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampQuery.h"
#include "VoxelQuery.h"

FVoxelMetadataMap::FVoxelMetadataMap()
	: MetadataToBuffer(INLINE_LAMBDA -> auto&
	{
		static const TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> Static;
		return Static;
	})
{
}

void FVoxelStampQuery::AddDependency(const FVoxelDependency& Dependency) const
{
	Query.DependencyCollector.AddDependency(Dependency);
}

void FVoxelStampQuery::AddDependency(
	const FVoxelDependency2D& Dependency,
	const FVoxelBox2D& Bounds) const
{
	Query.DependencyCollector.AddDependency(Dependency, Bounds);
}

void FVoxelStampQuery::AddDependency(
	const FVoxelDependency3D& Dependency,
	const FVoxelBox& Bounds) const
{
	Query.DependencyCollector.AddDependency(Dependency, Bounds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightQueryPrevious::Query(const FVoxelHeightBulkQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelHeightBulkQuery LocalQuery = Query;
	LocalQuery.QueryPrevious = {};

	QueryPrevious_Bulk(LocalQuery);
}

void FVoxelHeightQueryPrevious::Query(const FVoxelHeightSparseQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelHeightSparseQuery LocalQuery = Query;
	LocalQuery.QueryPrevious = {};

	QueryPrevious_Sparse(LocalQuery);
}

void FVoxelVolumeQueryPrevious::Query(const FVoxelVolumeBulkQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelVolumeBulkQuery LocalQuery = Query;
	// Make sure to not reuse the height cache
	LocalQuery.QueryHeights = {};
	LocalQuery.QueryPrevious = {};

	QueryPrevious_Bulk(LocalQuery);
}

void FVoxelVolumeQueryPrevious::Query(const FVoxelVolumeSparseQuery& Query) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelVolumeSparseQuery LocalQuery = Query;
	LocalQuery.QueryPrevious = {};

	QueryPrevious_Sparse(LocalQuery);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelHeightBulkQuery FVoxelHeightBulkQuery::Create(
	const FVoxelQuery& Query,
	const TVoxelArrayView<float> Heights,
	const FVector2D& Start,
	const FIntPoint& Size,
	const float Step)
{
	return Create(
		Query,
		Heights,
		Start,
		Size,
		FVoxelIntBox2D(0, Size),
		Step);
}

FVoxelHeightBulkQuery FVoxelHeightBulkQuery::Create(
	const FVoxelQuery& Query,
	const TVoxelArrayView<float> Heights,
	const FVector2D& Start,
	const FIntPoint& Size,
	const FVoxelIntBox2D& Indices,
	const float Step)
{
	checkVoxelSlow(Heights.Num() == Size.X * Size.Y);
	checkVoxelSlow(FVoxelIntBox2D(0, Size).Contains(Indices));

#if VOXEL_DEBUG
	for (int32 Y = Indices.Min.Y; Y < Indices.Max.Y; Y++)
	{
		for (int32 X = Indices.Min.X; X < Indices.Max.X; X++)
		{
			checkVoxelSlow(FVoxelUtilities::IsNaN(Heights[FVoxelUtilities::Get2DIndex<int32>(Size, X, Y)]));
		}
	}
#endif

	return FVoxelHeightBulkQuery
	{
		Query,
		Indices,
		Start,
		Step,
		Heights,
		Size.X,
		{}
	};
}

TVoxelOptional<FVoxelHeightBulkQuery> FVoxelHeightBulkQuery::ShrinkTo(const FVoxelBox2D& Bounds) const
{
	if (Bounds.IsInfinite())
	{
		return *this;
	}

	const FVoxelBox2D LocalBounds =
		Bounds
		.ShiftBy(-Start)
		.Scale(1. / Step);

	// We only want to query indices within the bounds: ie, Min <= Index <= Max
	// hence ceil(Min) <= Index <= floor(Max)
	const FVoxelIntBox2D NewIndices
	{
		FVoxelUtilities::CeilToInt(LocalBounds.Min),
		FVoxelUtilities::FloorToInt(LocalBounds.Max) + 1
	};

	const FVoxelIntBox2D InsideIndices = Indices.IntersectWith(NewIndices);
	if (!InsideIndices.IsValid())
	{
		return {};
	}

	return FVoxelHeightBulkQuery
	{
		Query,
		InsideIndices,
		Start,
		Step,
		Heights,
		StrideX,
		QueryPrevious
	};
}

void FVoxelHeightBulkQuery::Split(
	const FVoxelBox2D& Bounds,
	TVoxelOptional<FVoxelHeightBulkQuery>& Inside,
	TVoxelInlineArray<FVoxelHeightBulkQuery, 1>& Outside) const
{
	if (Bounds.IsInfinite())
	{
		Inside = *this;
		return;
	}

	const FVoxelBox2D LocalBounds =
		Bounds
		.ShiftBy(-Start)
		.Scale(1. / Step);

	// We only want to query indices within the bounds: ie, Min <= Index <= Max
	// hence ceil(Min) <= Index <= floor(Max)
	const FVoxelIntBox2D NewIndices
	{
		FVoxelUtilities::CeilToInt(LocalBounds.Min),
		FVoxelUtilities::FloorToInt(LocalBounds.Max) + 1
	};

	const FVoxelIntBox2D InsideIndices = Indices.IntersectWith(NewIndices);
	if (InsideIndices.IsValid())
	{
		Inside = FVoxelHeightBulkQuery
		{
			Query,
			InsideIndices,
			Start,
			Step,
			Heights,
			StrideX,
			QueryPrevious
		};
	}

	TVoxelArray<FVoxelIntBox2D> AllOutsideIndices;
	Indices.Remove_Split(NewIndices, AllOutsideIndices);

	for (const FVoxelIntBox2D& OutsideIndices : AllOutsideIndices)
	{
		Outside.Add(FVoxelHeightBulkQuery
		{
			Query,
			OutsideIndices,
			Start,
			Step,
			Heights,
			StrideX,
			QueryPrevious
		});
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVolumeBulkQuery FVoxelVolumeBulkQuery::Create(
	const FVoxelQuery& Query,
	const TVoxelArrayView<float> Distances,
	const FVector& Start,
	const FIntVector& Size,
	const float Step)
{
	return Create(
		Query,
		Distances,
		Start,
		Size,
		FVoxelIntBox(0, Size),
		Step);
}

FVoxelVolumeBulkQuery FVoxelVolumeBulkQuery::Create(
	const FVoxelQuery& Query,
	const TVoxelArrayView<float> Distances,
	const FVector& Start,
	const FIntVector& Size,
	const FVoxelIntBox& Indices,
	const float Step)
{
	checkVoxelSlow(Distances.Num() == Size.X * Size.Y * Size.Z);
	checkVoxelSlow(FVoxelIntBox(0, Size).Contains(Indices));

#if VOXEL_DEBUG
	for (int32 Z = Indices.Min.Z; Z < Indices.Max.Z; Z++)
	{
		for (int32 Y = Indices.Min.Y; Y < Indices.Max.Y; Y++)
		{
			for (int32 X = Indices.Min.X; X < Indices.Max.X; X++)
			{
				checkVoxelSlow(FVoxelUtilities::IsNaN(Distances[FVoxelUtilities::Get3DIndex<int32>(Size, X, Y, Z)]));
			}
		}
	}
#endif

	return FVoxelVolumeBulkQuery
	{
		Query,
		Indices,
		Start,
		Step,
		Distances,
		Size.X,
		Size.X * Size.Y,
		{},
		{}
	};
}

TVoxelOptional<FVoxelVolumeBulkQuery> FVoxelVolumeBulkQuery::ShrinkTo(const FVoxelBox& Bounds) const
{
	if (Bounds.IsInfinite())
	{
		return *this;
	}

	const FVoxelBox LocalBounds =
		Bounds
		.ShiftBy(-Start)
		.Scale(1. / Step);

	// We only want to query indices within the bounds: ie, Min <= Index <= Max
	// hence ceil(Min) <= Index <= floor(Max)
	const FVoxelIntBox NewIndices
	{
		FVoxelUtilities::CeilToInt(LocalBounds.Min),
		FVoxelUtilities::FloorToInt(LocalBounds.Max) + 1
	};

	const FVoxelIntBox InsideIndices = Indices.IntersectWith(NewIndices);
	if (!InsideIndices.IsValid())
	{
		return {};
	}

	return FVoxelVolumeBulkQuery
	{
		Query,
		InsideIndices,
		Start,
		Step,
		Distances,
		StrideX,
		StrideXY,
		QueryHeights,
		QueryPrevious
	};
}

void FVoxelVolumeBulkQuery::Split(
	const FVoxelBox& Bounds,
	TVoxelOptional<FVoxelVolumeBulkQuery>& Inside,
	TVoxelInlineArray<FVoxelVolumeBulkQuery, 1>& Outside) const
{
	if (Bounds.IsInfinite())
	{
		Inside = *this;
		return;
	}

	const FVoxelBox LocalBounds =
		Bounds
		.ShiftBy(-Start)
		.Scale(1. / Step);

	// We only want to query indices within the bounds: ie, Min <= Index <= Max
	// hence ceil(Min) <= Index <= floor(Max)
	const FVoxelIntBox NewIndices
	{
		FVoxelUtilities::CeilToInt(LocalBounds.Min),
		FVoxelUtilities::FloorToInt(LocalBounds.Max) + 1
	};

	const FVoxelIntBox InsideIndices = Indices.IntersectWith(NewIndices);
	if (InsideIndices.IsValid())
	{
		Inside = FVoxelVolumeBulkQuery
		{
			Query,
			InsideIndices,
			Start,
			Step,
			Distances,
			StrideX,
			StrideXY,
			QueryHeights,
			QueryPrevious
		};
	}

	TVoxelArray<FVoxelIntBox> AllOutsideIndices;
	Indices.Remove_Split(NewIndices, AllOutsideIndices);

	for (const FVoxelIntBox& OutsideIndices : AllOutsideIndices)
	{
		Outside.Add(FVoxelVolumeBulkQuery
		{
			Query,
			OutsideIndices,
			Start,
			Step,
			Distances,
			StrideX,
			StrideXY,
			QueryHeights,
			QueryPrevious
		});
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelHeightSparseQuery FVoxelHeightSparseQuery::Create(
	const FVoxelQuery& Query,
	const TVoxelArrayView<float> Heights,
	const TVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypes,
	const FVoxelMetadataMap& Metadata,
	const FVoxelDoubleVector2DBuffer& Positions,
	const bool bQuerySurfaceTypes,
	const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery)
{
	checkVoxelSlow(Heights.Num() == Positions.Num());

	if (bQuerySurfaceTypes)
	{
		checkVoxelSlow(SurfaceTypes.Num() == Positions.Num());
	}

	for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
	{
		checkVoxelSlow(Metadata.FindChecked(MetadataToQuery).Num_Slow() == Positions.Num());
	}

	return FVoxelHeightSparseQuery
	{
		Query,
		SurfaceTypes,
		Metadata,
		{},
		bQuerySurfaceTypes,
		MetadatasToQuery,
		Heights,
		Positions,
		FVoxelBox2D::FromPositions(
			Positions.X.View(),
			Positions.Y.View()),
		{}
	};
}

TVoxelOptional<FVoxelHeightSparseQuery> FVoxelHeightSparseQuery::ShrinkTo(const FVoxelBox2D& Bounds) const
{
	if (!PositionBounds.Intersects(Bounds))
	{
		return {};
	}

	if (Bounds.Contains(PositionBounds))
	{
		return *this;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Num());

	FVoxelDoubleVector2DBuffer NewPositions;
	FVoxelInt32Buffer NewIndirection;

	NewPositions.Allocate(Num());
	NewIndirection.Allocate(Num());

	int32 WriteIndex = 0;
	for (int32 Index = 0; Index < Num(); Index++)
	{
		const FVector2d Position = Positions[Index];
		if (!Bounds.Contains(Position))
		{
			continue;
		}

		NewPositions.Set(WriteIndex, Position);
		NewIndirection.Set(WriteIndex, GetIndirectIndex(Index));

		WriteIndex++;
	}

	if (WriteIndex == 0)
	{
		return {};
	}
	if (WriteIndex == Num())
	{
		return *this;
	}

	NewPositions.ShrinkTo(WriteIndex);
	NewIndirection.ShrinkTo(WriteIndex);

	return FVoxelHeightSparseQuery
	{
		Query,
		IndirectSurfaceTypes,
		IndirectMetadata,
		MoveTemp(NewIndirection),
		bQuerySurfaceTypes,
		MetadatasToQuery,
		IndirectHeights,
		MoveTemp(NewPositions),
		PositionBounds.IntersectWith(Bounds),
		QueryPrevious
	};
}

TVoxelOptional<FVoxelHeightSparseQuery> FVoxelHeightSparseQuery::ReverseAlphaCull(const FVoxelFloatBuffer& Alphas) const
{
	if (Alphas.IsConstant())
	{
		if (Alphas.GetConstant() >= 1.f)
		{
			return {};
		}
		else
		{
			return *this;
		}
	}

	VOXEL_FUNCTION_COUNTER_NUM(Num());

	FVoxelDoubleVector2DBuffer NewPositions;
	FVoxelInt32Buffer NewIndirection;

	NewPositions.Allocate(Num());
	NewIndirection.Allocate(Num());

	int32 WriteIndex = 0;
	for (int32 Index = 0; Index < Num(); Index++)
	{
		const float Alpha = Alphas[Index];
		if (Alpha >= 1.f)
		{
			continue;
		}

		NewPositions.Set(WriteIndex, Positions[Index]);
		NewIndirection.Set(WriteIndex, GetIndirectIndex(Index));

		WriteIndex++;
	}

	if (WriteIndex == 0)
	{
		return {};
	}
	if (WriteIndex == Num())
	{
		return *this;
	}

	NewPositions.ShrinkTo(WriteIndex);
	NewIndirection.ShrinkTo(WriteIndex);

	return FVoxelHeightSparseQuery
	{
		Query,
		IndirectSurfaceTypes,
		IndirectMetadata,
		MoveTemp(NewIndirection),
		bQuerySurfaceTypes,
		MetadatasToQuery,
		IndirectHeights,
		MoveTemp(NewPositions),
		PositionBounds,
		QueryPrevious
	};
}

void FVoxelHeightSparseQuery::Split(
	const FVoxelBox2D& Bounds,
	TVoxelOptional<FVoxelHeightSparseQuery>& Inside,
	TVoxelInlineArray<FVoxelHeightSparseQuery, 1>& Outside) const
{
	if (!PositionBounds.Intersects(Bounds))
	{
		Outside.Add(*this);
		return;
	}

	if (Bounds.Contains(PositionBounds))
	{
		Inside = *this;
		return;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Num());

	FVoxelDoubleVector2DBuffer InsidePositions;
	FVoxelInt32Buffer InsideIndirection;

	FVoxelDoubleVector2DBuffer OutsidePositions;
	FVoxelInt32Buffer OutsideIndirection;

	InsidePositions.Allocate(Num());
	InsideIndirection.Allocate(Num());

	OutsidePositions.Allocate(Num());
	OutsideIndirection.Allocate(Num());

	int32 NumInside = 0;
	int32 NumOutside = 0;
	for (int32 Index = 0; Index < Num(); Index++)
	{
		const FVector2d Position = Positions[Index];

		if (Bounds.Contains(Position))
		{
			InsidePositions.Set(NumInside, Position);
			InsideIndirection.Set(NumInside, GetIndirectIndex(Index));

			NumInside++;
		}
		else
		{
			OutsidePositions.Set(NumOutside, Position);
			OutsideIndirection.Set(NumOutside, GetIndirectIndex(Index));

			NumOutside++;
		}
	}

	if (NumInside == 0)
	{
		Outside.Add(*this);
		return;
	}
	if (NumOutside == 0)
	{
		Inside = *this;
		return;
	}

	InsidePositions.ShrinkTo(NumInside);
	InsideIndirection.ShrinkTo(NumInside);

	OutsidePositions.ShrinkTo(NumOutside);
	OutsideIndirection.ShrinkTo(NumOutside);

	Inside = FVoxelHeightSparseQuery
	{
		Query,
		IndirectSurfaceTypes,
		IndirectMetadata,
		MoveTemp(InsideIndirection),
		bQuerySurfaceTypes,
		MetadatasToQuery,
		IndirectHeights,
		MoveTemp(InsidePositions),
		PositionBounds.IntersectWith(Bounds),
		QueryPrevious
	};

	Outside.Add(FVoxelHeightSparseQuery
	{
		Query,
		IndirectSurfaceTypes,
		IndirectMetadata,
		MoveTemp(OutsideIndirection),
		bQuerySurfaceTypes,
		MetadatasToQuery,
		IndirectHeights,
		MoveTemp(OutsidePositions),
		PositionBounds.Remove_Union(Bounds),
		QueryPrevious
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVolumeSparseQuery FVoxelVolumeSparseQuery::Create(
	const FVoxelQuery& Query,
	const TVoxelArrayView<float> Distances,
	const TVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypes,
	const FVoxelMetadataMap& Metadata,
	const FVoxelDoubleVectorBuffer& Positions,
	const bool bQuerySurfaceTypes,
	const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery)
{
	checkVoxelSlow(Distances.Num() == Positions.Num());

	if (bQuerySurfaceTypes)
	{
		checkVoxelSlow(SurfaceTypes.Num() == Positions.Num());
	}

	for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
	{
		checkVoxelSlow(Metadata.FindChecked(MetadataToQuery).Num_Slow() == Positions.Num());
	}

	return FVoxelVolumeSparseQuery
	{
		Query,
		SurfaceTypes,
		Metadata,
		{},
		bQuerySurfaceTypes,
		MetadatasToQuery,
		Distances,
		Positions,
		FVoxelBox::FromPositions(
			Positions.X.View(),
			Positions.Y.View(),
			Positions.Z.View()),
		{}
	};
}

TVoxelOptional<FVoxelVolumeSparseQuery> FVoxelVolumeSparseQuery::ShrinkTo(const FVoxelBox& Bounds) const
{
	if (!PositionBounds.Intersects(Bounds))
	{
		return {};
	}

	if (Bounds.Contains(PositionBounds))
	{
		return *this;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Num());

	FVoxelDoubleVectorBuffer NewPositions;
	FVoxelInt32Buffer NewIndirection;

	NewPositions.Allocate(Num());
	NewIndirection.Allocate(Num());

	int32 WriteIndex = 0;
	for (int32 Index = 0; Index < Num(); Index++)
	{
		const FVector Position = Positions[Index];
		if (!Bounds.Contains(Position))
		{
			continue;
		}

		NewPositions.Set(WriteIndex, Position);
		NewIndirection.Set(WriteIndex, GetIndirectIndex(Index));

		WriteIndex++;
	}

	if (WriteIndex == 0)
	{
		return {};
	}
	if (WriteIndex == Num())
	{
		return *this;
	}

	NewPositions.ShrinkTo(WriteIndex);
	NewIndirection.ShrinkTo(WriteIndex);

	return FVoxelVolumeSparseQuery
	{
		Query,
		IndirectSurfaceTypes,
		IndirectMetadata,
		MoveTemp(NewIndirection),
		bQuerySurfaceTypes,
		MetadatasToQuery,
		IndirectDistances,
		MoveTemp(NewPositions),
		PositionBounds.IntersectWith(Bounds),
		QueryPrevious
	};
}

TVoxelOptional<FVoxelVolumeSparseQuery> FVoxelVolumeSparseQuery::ReverseAlphaCull(const FVoxelFloatBuffer& Alphas) const
{
	if (Alphas.IsConstant())
	{
		if (Alphas.GetConstant() >= 1.f)
		{
			return {};
		}
		else
		{
			return *this;
		}
	}

	VOXEL_FUNCTION_COUNTER_NUM(Num());

	FVoxelDoubleVectorBuffer NewPositions;
	FVoxelInt32Buffer NewIndirection;

	NewPositions.Allocate(Num());
	NewIndirection.Allocate(Num());

	int32 WriteIndex = 0;
	for (int32 Index = 0; Index < Num(); Index++)
	{
		const float Alpha = Alphas[Index];
		if (Alpha >= 1.f)
		{
			continue;
		}

		NewPositions.Set(WriteIndex, Positions[Index]);
		NewIndirection.Set(WriteIndex, GetIndirectIndex(Index));

		WriteIndex++;
	}

	if (WriteIndex == 0)
	{
		return {};
	}
	if (WriteIndex == Num())
	{
		return *this;
	}

	NewPositions.ShrinkTo(WriteIndex);
	NewIndirection.ShrinkTo(WriteIndex);

	return FVoxelVolumeSparseQuery
	{
		Query,
		IndirectSurfaceTypes,
		IndirectMetadata,
		MoveTemp(NewIndirection),
		bQuerySurfaceTypes,
		MetadatasToQuery,
		IndirectDistances,
		MoveTemp(NewPositions),
		PositionBounds,
		QueryPrevious
	};
}

void FVoxelVolumeSparseQuery::Split(
	const FVoxelBox& Bounds,
	TVoxelOptional<FVoxelVolumeSparseQuery>& Inside,
	TVoxelInlineArray<FVoxelVolumeSparseQuery, 1>& Outside) const
{
	if (!PositionBounds.Intersects(Bounds))
	{
		Outside.Add(*this);
		return;
	}

	if (Bounds.Contains(PositionBounds))
	{
		Inside = *this;
		return;
	}

	VOXEL_FUNCTION_COUNTER_NUM(Num());

	FVoxelDoubleVectorBuffer InsidePositions;
	FVoxelInt32Buffer InsideIndirection;

	FVoxelDoubleVectorBuffer OutsidePositions;
	FVoxelInt32Buffer OutsideIndirection;

	InsidePositions.Allocate(Num());
	InsideIndirection.Allocate(Num());

	OutsidePositions.Allocate(Num());
	OutsideIndirection.Allocate(Num());

	int32 NumInside = 0;
	int32 NumOutside = 0;
	for (int32 Index = 0; Index < Num(); Index++)
	{
		const FVector Position = Positions[Index];

		if (Bounds.Contains(Position))
		{
			InsidePositions.Set(NumInside, Position);
			InsideIndirection.Set(NumInside, GetIndirectIndex(Index));

			NumInside++;
		}
		else
		{
			OutsidePositions.Set(NumOutside, Position);
			OutsideIndirection.Set(NumOutside, GetIndirectIndex(Index));

			NumOutside++;
		}
	}

	if (NumInside == 0)
	{
		Outside.Add(*this);
		return;
	}
	if (NumOutside == 0)
	{
		Inside = *this;
		return;
	}

	InsidePositions.ShrinkTo(NumInside);
	InsideIndirection.ShrinkTo(NumInside);

	OutsidePositions.ShrinkTo(NumOutside);
	OutsideIndirection.ShrinkTo(NumOutside);

	Inside = FVoxelVolumeSparseQuery
	{
		Query,
		IndirectSurfaceTypes,
		IndirectMetadata,
		MoveTemp(InsideIndirection),
		bQuerySurfaceTypes,
		MetadatasToQuery,
		IndirectDistances,
		MoveTemp(InsidePositions),
		PositionBounds.IntersectWith(Bounds),
		QueryPrevious
	};

	Outside.Add(FVoxelVolumeSparseQuery
	{
		Query,
		IndirectSurfaceTypes,
		IndirectMetadata,
		MoveTemp(OutsideIndirection),
		bQuerySurfaceTypes,
		MetadatasToQuery,
		IndirectDistances,
		MoveTemp(OutsidePositions),
		PositionBounds.Remove_Union(Bounds),
		QueryPrevious
	});
}