// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampUtilities.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"
#include "VoxelBlendModeUtilities.h"
#include "Graphs/VoxelStampGraphParameters.h"
#include "Graphs/VoxelOutputNode_OutputHeightBase.h"
#include "Graphs/VoxelOutputNode_OutputVolumeBase.h"
#include "VoxelStampUtilitiesImpl.ispc.generated.h"

FVoxelDoubleVector2DBuffer FVoxelStampUtilities::ComputePositions(
	const FVoxelHeightBulkQuery& Query,
	const FVoxelHeightTransform& StampToQuery)
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	FVoxelDoubleVector2DBuffer Result;
	Result.Allocate(Query.Num());

	ispc::VoxelStampUtilities_ComputePositions_BulkHeight(
		Query.ISPC(),
		StampToQuery.ISPC(),
		Result.X.GetData(),
		Result.Y.GetData());

	return Result;
}

FVoxelDoubleVector2DBuffer FVoxelStampUtilities::ComputePositions(
	const FVoxelHeightSparseQuery& Query,
	const FVoxelHeightTransform& StampToQuery)
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	FVoxelDoubleVector2DBuffer Result;
	Result.Allocate(Query.Num());

	ispc::VoxelStampUtilities_ComputePositions_SparseHeight(
		Query.ISPC(),
		StampToQuery.ISPC(),
		Result.X.GetData(),
		Result.Y.GetData());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDoubleVectorBuffer FVoxelStampUtilities::ComputePositions(
	const FVoxelVolumeBulkQuery& Query,
	const FVoxelVolumeTransform& StampToQuery)
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	FVoxelDoubleVectorBuffer Result;
	Result.Allocate(Query.Num());

	ispc::VoxelStampUtilities_ComputePositions_BulkVolume(
		Query.ISPC(),
		StampToQuery.ISPC(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

FVoxelDoubleVectorBuffer FVoxelStampUtilities::ComputePositions(
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery)
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	FVoxelDoubleVectorBuffer Result;
	Result.Allocate(Query.Num());

	ispc::VoxelStampUtilities_ComputePositions_SparseVolume(
		Query.ISPC(),
		StampToQuery.ISPC(),
		Result.X.GetData(),
		Result.Y.GetData(),
		Result.Z.GetData());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampUtilities::ApplyHeights(
	const FVoxelHeightStampRuntime& StampRuntime,
	const FVoxelHeightBulkQuery& Query,
	const FVoxelHeightTransform& StampToQuery,
	const FVoxelFloatBuffer& NewHeights,
	const FVoxelDoubleVector2DBuffer& Positions)
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	if (StampRuntime.GetBlendMode() == EVoxelHeightBlendMode::Override)
	{
		int32 ReadIndex = 0;
		for (int32 IndexY = Query.Indices.Min.Y; IndexY < Query.Indices.Max.Y; IndexY++)
		{
			for (int32 IndexX = Query.Indices.Min.X; IndexX < Query.Indices.Max.X; IndexX++)
			{
				Query.Heights[Query.GetIndex(IndexX, IndexY)] = StampToQuery.ClampHeight(StampToQuery.TransformHeight(NewHeights[ReadIndex], Positions[ReadIndex]));
				ReadIndex++;
			}
		}
		checkVoxelSlow(ReadIndex == Query.Num());

		return;
	}

	int32 ReadIndex = 0;
	for (int32 IndexY = Query.Indices.Min.Y; IndexY < Query.Indices.Max.Y; IndexY++)
	{
		for (int32 IndexX = Query.Indices.Min.X; IndexX < Query.Indices.Max.X; IndexX++)
		{
			float& Height = Query.Heights[Query.GetIndex(IndexX, IndexY)];

			Height = FVoxelBlendModeUtilities::ComputeHeight(
				StampToQuery,
				Height,
				NewHeights[ReadIndex],
				Positions[ReadIndex],
				StampRuntime.GetBlendMode(),
				StampRuntime.GetStamp().bApplyOnVoid,
				StampRuntime.GetSmoothness(),
				nullptr);

			ReadIndex++;
		}
	}
	checkVoxelSlow(ReadIndex == Query.Num());
}

void FVoxelStampUtilities::ApplyDistances(
	const FVoxelVolumeStampRuntime& StampRuntime,
	const FVoxelVolumeBulkQuery& Query,
	const FVoxelVolumeTransform& StampToQuery,
	const FVoxelFloatBuffer& NewDistances)
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	if (StampRuntime.GetBlendMode() == EVoxelVolumeBlendMode::Override)
	{
		int32 ReadIndex = 0;
		for (int32 IndexZ = Query.Indices.Min.Z; IndexZ < Query.Indices.Max.Z; IndexZ++)
		{
			for (int32 IndexY = Query.Indices.Min.Y; IndexY < Query.Indices.Max.Y; IndexY++)
			{
				for (int32 IndexX = Query.Indices.Min.X; IndexX < Query.Indices.Max.X; IndexX++)
				{
					// Don't apply max distance on stamp overrides, we have no previous value
					Query.Distances[Query.GetIndex(IndexX, IndexY, IndexZ)] = StampToQuery.TransformDistance_NoMaxDistance(NewDistances[ReadIndex++]);
				}
			}
		}
		checkVoxelSlow(ReadIndex == Query.Num());

		return;
	}

	int32 ReadIndex = 0;
	for (int32 IndexZ = Query.Indices.Min.Z; IndexZ < Query.Indices.Max.Z; IndexZ++)
	{
		for (int32 IndexY = Query.Indices.Min.Y; IndexY < Query.Indices.Max.Y; IndexY++)
		{
			for (int32 IndexX = Query.Indices.Min.X; IndexX < Query.Indices.Max.X; IndexX++)
			{
				float& Distance = Query.Distances[Query.GetIndex(IndexX, IndexY, IndexZ)];

				Distance = FVoxelBlendModeUtilities::ComputeDistance(
					StampToQuery,
					Distance,
					NewDistances[ReadIndex],
					StampRuntime.GetBlendMode(),
					StampRuntime.GetStamp().bApplyOnVoid,
					StampRuntime.GetSmoothness(),
					nullptr);

				ReadIndex++;
			}
		}
	}
	checkVoxelSlow(ReadIndex == Query.Num());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampUtilities::ComputeHeightStamp(
	const FVoxelHeightStampRuntime& StampRuntime,
	const FVoxelHeightSparseQuery& Query,
	const FVoxelHeightTransform& StampToQuery,
	const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
	FVoxelGraphQueryImpl& GraphQuery,
	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputHeightBase>& Evaluator,
	const FVoxelDoubleVector2DBuffer& Positions)
{
	VOXEL_FUNCTION_COUNTER();

	if (StampRuntime.GetBlendMode() == EVoxelHeightBlendMode::Override)
	{
		ComputeOverrideStamp<true>(
			StampRuntime,
			Query,
			StampToQuery,
			MetadataOverrides,
			GraphQuery,
			Evaluator,
			Positions);

		return;
	}
	ensureVoxelSlow(!StampRuntime.ShouldUseQueryPrevious());
	ensureVoxelSlow(!Query.QueryPrevious);

	const bool bComputeSurfaceTypes =
		StampRuntime.AffectSurfaceType() &&
		Query.bQuerySurfaceTypes;

	const bool bComputeMetadatas =
		StampRuntime.AffectMetadata() &&
		Query.MetadatasToQuery.Num() > 0 &&
		MetadataOverrides.ShouldCompute(Query);

	if (!bComputeSurfaceTypes &&
		!bComputeMetadatas)
	{
		if (StampRuntime.AffectShape())
		{
			ApplyHeights(
				StampRuntime,
				Query,
				StampToQuery,
				*Evaluator->HeightPin.GetSynchronous(GraphQuery),
				Positions,
				true,
				nullptr);
		}

		return;
	}

	FVoxelFloatBuffer Alphas;
	ApplyHeights(
		StampRuntime,
		Query,
		StampToQuery,
		*Evaluator->HeightPin.GetSynchronous(GraphQuery),
		Positions,
		StampRuntime.AffectShape(),
		&Alphas);

	if (bComputeSurfaceTypes)
	{
		ApplySurfaceTypes(
			Query,
			*Evaluator->SurfaceTypePin.GetSynchronous(GraphQuery),
			Alphas);
	}

	if (bComputeMetadatas)
	{
		ComputeMetadatas(
			Query,
			MetadataOverrides,
			GraphQuery,
			Evaluator,
			Alphas);
	}
}

void FVoxelStampUtilities::ComputeVolumeStamp(
	const FVoxelVolumeStampRuntime& StampRuntime,
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery,
	const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
	FVoxelGraphQueryImpl& GraphQuery,
	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputVolumeBase>& Evaluator,
	const FVoxelDoubleVectorBuffer& Positions)
{
	VOXEL_FUNCTION_COUNTER();

	if (StampRuntime.GetBlendMode() == EVoxelVolumeBlendMode::Override)
	{
		ComputeOverrideStamp<false>(
			StampRuntime,
			Query,
			StampToQuery,
			MetadataOverrides,
			GraphQuery,
			Evaluator,
			Positions);

		return;
	}
	ensureVoxelSlow(!StampRuntime.ShouldUseQueryPrevious());
	ensureVoxelSlow(!Query.QueryPrevious);

	const bool bComputeSurfaceTypes =
		StampRuntime.AffectSurfaceType() &&
		Query.bQuerySurfaceTypes;

	const bool bComputeMetadatas =
		StampRuntime.AffectMetadata() &&
		Query.MetadatasToQuery.Num() > 0 &&
		MetadataOverrides.ShouldCompute(Query);

	if (!bComputeSurfaceTypes &&
		!bComputeMetadatas)
	{
		if (StampRuntime.AffectShape())
		{
			ApplyDistances(
				StampRuntime,
				Query,
				StampToQuery,
				*Evaluator->DistancePin.GetSynchronous(GraphQuery),
				true,
				nullptr);
		}

		return;
	}

	FVoxelFloatBuffer Alphas;
	ApplyDistances(
		StampRuntime,
		Query,
		StampToQuery,
		*Evaluator->DistancePin.GetSynchronous(GraphQuery),
		StampRuntime.AffectShape(),
		&Alphas);

	if (bComputeSurfaceTypes)
	{
		ApplySurfaceTypes(
			Query,
			*Evaluator->SurfaceTypePin.GetSynchronous(GraphQuery),
			Alphas);
	}

	if (bComputeMetadatas)
	{
		ComputeMetadatas(
			Query,
			MetadataOverrides,
			GraphQuery,
			Evaluator,
			Alphas);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<bool bIsHeight>
void FVoxelStampUtilities::ComputeOverrideStamp(
	const FVoxelStampRuntime& StampRuntime,
	const std::conditional_t<bIsHeight, FVoxelHeightSparseQuery, FVoxelVolumeSparseQuery>& Query,
	const std::conditional_t<bIsHeight, FVoxelHeightTransform, FVoxelVolumeTransform>& StampToQuery,
	const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
	FVoxelGraphQueryImpl& GraphQuery,
	const TVoxelNodeEvaluator<std::conditional_t<bIsHeight, FVoxelOutputNode_OutputHeightBase, FVoxelOutputNode_OutputVolumeBase>>& Evaluator,
	const std::conditional_t<bIsHeight, FVoxelDoubleVector2DBuffer, FVoxelDoubleVectorBuffer>& Positions)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(Query.QueryPrevious))
	{
		return;
	}

	using QueryPreviousType = std::conditional_t<
		bIsHeight,
		FVoxelGraphParameters::FHeightQueryPrevious,
		FVoxelGraphParameters::FVolumeQueryPrevious>;

	QueryPreviousType& Parameter = GraphQuery.AddParameter<QueryPreviousType>();
	Parameter.GlobalPositions = Positions;
	Parameter.bQueryPreviousValues = !StampRuntime.AffectShape();

	bool bAlphaIsOne =
		Evaluator->AlphaPin.IsDefaultValue() &&
		Evaluator->AlphaPin.GetDefaultValue().template Get<FVoxelFloatBuffer>().GetConstant() == 1.f;

	bool bSurfaceTypeIsNotNull =
		Evaluator->SurfaceTypePin.IsDefaultValue() &&
		!Evaluator->SurfaceTypePin.GetDefaultValue().template Get<FVoxelSurfaceTypeBlendBuffer>().GetConstant().IsNull();

	const auto UpdatePreviousToQuery = [&]
	{
		ensure(!Parameter.bQueried);

		if (Query.bQuerySurfaceTypes)
		{
			// TRICKY: If we override SurfaceType with any null, we still need the previous value even is alpha is 1

			Parameter.bQueryPreviousSurfaceTypes = true;

			if (StampRuntime.AffectSurfaceType() &&
				bAlphaIsOne &&
				bSurfaceTypeIsNotNull)
			{
				// All surface types will be overriden by this new value
				Parameter.bQueryPreviousSurfaceTypes = false;
			}
		}
		else
		{
			Parameter.bQueryPreviousSurfaceTypes = false;
		}

		Parameter.PreviousMetadatasToQuery.Reset(Query.MetadatasToQuery.Num());

		for (const FVoxelMetadataRef& Metadata : Query.MetadatasToQuery)
		{
			if (StampRuntime.AffectMetadata() &&
				bAlphaIsOne &&
				MetadataOverrides.MetadataToValue.Contains(Metadata))
			{
				continue;
			}

			Parameter.PreviousMetadatasToQuery.Add_EnsureNoGrow(Metadata);
		}
	};

	UpdatePreviousToQuery();

	const bool bApplySurfaceTypes =
		StampRuntime.AffectSurfaceType() &&
		Query.bQuerySurfaceTypes;

	const bool bApplyMetadatas =
		StampRuntime.AffectMetadata() &&
		Query.MetadatasToQuery.Num() > 0 &&
		MetadataOverrides.ShouldCompute(Query);

	FVoxelFloatBuffer Alphas;
	if (bApplySurfaceTypes ||
		bApplyMetadatas)
	{
		VOXEL_SCOPE_COUNTER("Compute alphas");

		Alphas = *Evaluator->AlphaPin.GetSynchronous(GraphQuery);

		if (!Parameter.bQueried &&
			!bAlphaIsOne)
		{
			bAlphaIsOne = INLINE_LAMBDA
			{
				for (const float Alpha : Alphas)
				{
					if (Alpha < 1.f)
					{
						return false;
					}
				}

				return true;
			};

			if (bAlphaIsOne)
			{
				UpdatePreviousToQuery();
			}
		}
	}

	FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
	if (bApplySurfaceTypes)
	{
		VOXEL_SCOPE_COUNTER("Compute surface types");

		SurfaceTypes = *Evaluator->SurfaceTypePin.GetSynchronous(GraphQuery);

		if (!Parameter.bQueried &&
			!bSurfaceTypeIsNotNull)
		{
			bSurfaceTypeIsNotNull = INLINE_LAMBDA
			{
				for (const FVoxelSurfaceTypeBlend& SurfaceType : SurfaceTypes)
				{
					if (SurfaceType.IsNull())
					{
						return false;
					}
				}

				return true;
			};

			if (bSurfaceTypeIsNotNull)
			{
				UpdatePreviousToQuery();
			}
		}
	}

	const FVoxelFloatBuffer Values = INLINE_LAMBDA
	{
		VOXEL_SCOPE_COUNTER("Compute values");

		if constexpr (bIsHeight)
		{
			return *Evaluator->HeightPin.GetSynchronous(GraphQuery);
		}
		else
		{
			return *Evaluator->DistancePin.GetSynchronous(GraphQuery);
		}
	};

	TVoxelMap<int32, TSharedRef<const FVoxelBuffer>> PinIndexToMetadata;
	if (bApplyMetadatas)
	{
		VOXEL_SCOPE_COUNTER("Compute metadatas");

		PinIndexToMetadata.Reserve(Query.MetadatasToQuery.Num());

		for (const FVoxelMetadataRef& Metadata : Query.MetadatasToQuery)
		{
			const FVoxelRuntimeMetadataOverrides::FMetadataValue* Value = MetadataOverrides.MetadataToValue.Find(Metadata);
			if (!Value ||
				Value->PinIndex == -1)
			{
				continue;
			}
			checkVoxelSlow(!Value->Constant.IsValid());

			PinIndexToMetadata.Add_EnsureNew(
				Value->PinIndex,
				Evaluator->MetadataPins[Value->PinIndex].Value.GetSynchronous(GraphQuery));
		}
	}

	if (Parameter.bQueried)
	{
		VOXEL_SCOPE_COUNTER("Copy previous data");

		if (Parameter.bQueryPreviousValues)
		{
			for (int32 Index = 0; Index < Query.Num(); Index++)
			{
				if constexpr (bIsHeight)
				{
					Query.IndirectHeights[Query.GetIndirectIndex(Index)] = Parameter.OutValues[Index];
				}
				else
				{
					Query.IndirectDistances[Query.GetIndirectIndex(Index)] = Parameter.OutValues[Index];
				}
			}
		}

		if (Parameter.bQueryPreviousSurfaceTypes)
		{
			for (int32 Index = 0; Index < Query.Num(); Index++)
			{
				Query.IndirectSurfaceTypes[Query.GetIndirectIndex(Index)] = Parameter.OutSurfaceTypes[Index];
			}
		}

		for (const FVoxelMetadataRef& Metadata : Parameter.PreviousMetadatasToQuery)
		{
			Query.IndirectMetadata.FindChecked(Metadata).IndirectCopyFrom(
				*Parameter.OutMetadataToBuffer[Metadata],
				Query.Indirection);
		}
	}
	else if (
		Parameter.bQueryPreviousValues ||
		Parameter.bQueryPreviousSurfaceTypes ||
		Parameter.PreviousMetadatasToQuery.Num() > 0)
	{
		FString Reason;
		if (AreVoxelStatsEnabled())
		{
			if (Parameter.bQueryPreviousValues)
			{
				if (bIsHeight)
				{
					Reason += "Heights, ";
				}
				else
				{
					Reason += "Distance, ";
				}
			}
			if (Parameter.bQueryPreviousSurfaceTypes)
			{
				Reason += "SurfaceTypes, ";
			}

			for (const FVoxelMetadataRef& Metadata : Parameter.PreviousMetadatasToQuery)
			{
				Reason += Metadata.GetName() + ", ";
			}

			Reason.RemoveFromEnd(", ");
			ensure(!Reason.IsEmpty());
		}
		VOXEL_SCOPE_COUNTER_FORMAT("QueryPrevious: %s", *Reason);

		std::conditional_t<bIsHeight, FVoxelHeightSparseQuery, FVoxelVolumeSparseQuery> LocalQuery = Query;
		LocalQuery.bQuerySurfaceTypes = Parameter.bQueryPreviousSurfaceTypes;
		LocalQuery.MetadatasToQuery = Parameter.PreviousMetadatasToQuery;
		Query.QueryPrevious->Query(Query);

		// QueryPrevious might modify Height/Distance but that's fine - ApplyHeights/ApplyDistances below will replace it if needed
	}

	if (StampRuntime.AffectShape())
	{
		if constexpr (bIsHeight)
		{
			VOXEL_SCOPE_COUNTER("ApplyHeights");

			for (int32 Index = 0; Index < Query.Num(); Index++)
			{
				Query.IndirectHeights[Query.GetIndirectIndex(Index)] = StampToQuery.ClampHeight(StampToQuery.TransformHeight(Values[Index], Positions[Index]));
			}
		}
		else
		{
			VOXEL_SCOPE_COUNTER("ApplyDistances");

			// Unlike ApplyDistances we want to make sure to override every value
			for (int32 Index = 0; Index < Query.Num(); Index++)
			{
				// Don't apply max distance on stamp overrides, we have no previous value
				Query.IndirectDistances[Query.GetIndirectIndex(Index)] = StampToQuery.TransformDistance_NoMaxDistance(Values[Index]);
			}
		}
	}

	if (bApplySurfaceTypes)
	{
		FVoxelStampUtilities::ApplySurfaceTypes(
			Query,
			SurfaceTypes,
			Alphas);
	}

	if (bApplyMetadatas)
	{
		FVoxelStampUtilities::ApplyMetadatas(
			Query,
			MetadataOverrides,
			PinIndexToMetadata,
			Alphas);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampUtilities::ApplyHeights(
	const FVoxelHeightStampRuntime& StampRuntime,
	const FVoxelHeightSparseQuery& Query,
	const FVoxelHeightTransform& StampToQuery,
	const FVoxelFloatBuffer& NewHeights,
	const FVoxelDoubleVector2DBuffer& Positions,
	const bool bWriteHeights,
	FVoxelFloatBuffer* OutAlphas)
{
	VOXEL_FUNCTION_COUNTER();

	if (OutAlphas)
	{
		OutAlphas->Allocate(Query.Num());

		for (int32 Index = 0; Index < Query.Num(); Index++)
		{
			const int32 IndirectIndex = Query.GetIndirectIndex(Index);
			const float OldHeight = Query.IndirectHeights[IndirectIndex];

			const float NewHeight = FVoxelBlendModeUtilities::ComputeHeight(
				StampToQuery,
				OldHeight,
				NewHeights[Index],
				Positions[Index],
				StampRuntime.GetBlendMode(),
				StampRuntime.GetStamp().bApplyOnVoid,
				StampRuntime.GetSmoothness(),
				&OutAlphas->View()[Index]);

			if (bWriteHeights)
			{
				Query.IndirectHeights[IndirectIndex] = NewHeight;
			}
		}
	}
	else
	{
		check(bWriteHeights);

		for (int32 Index = 0; Index < Query.Num(); Index++)
		{
			const int32 IndirectIndex = Query.GetIndirectIndex(Index);
			const float OldHeight = Query.IndirectHeights[IndirectIndex];

			const float NewHeight = FVoxelBlendModeUtilities::ComputeHeight(
				StampToQuery,
				OldHeight,
				NewHeights[Index],
				Positions[Index],
				StampRuntime.GetBlendMode(),
				StampRuntime.GetStamp().bApplyOnVoid,
				StampRuntime.GetSmoothness(),
				nullptr);

			Query.IndirectHeights[IndirectIndex] = NewHeight;
		}
	}
}

void FVoxelStampUtilities::ApplyDistances(
	const FVoxelVolumeStampRuntime& StampRuntime,
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery,
	const FVoxelFloatBuffer& NewDistances,
	const bool bWriteDistances,
	FVoxelFloatBuffer* OutAlphas)
{
	VOXEL_FUNCTION_COUNTER();

	if (OutAlphas)
	{
		OutAlphas->Allocate(Query.Num());

		for (int32 Index = 0; Index < Query.Num(); Index++)
		{
			const int32 IndirectIndex = Query.GetIndirectIndex(Index);
			const float OldDistance = Query.IndirectDistances[IndirectIndex];

			const float NewDistance = FVoxelBlendModeUtilities::ComputeDistance(
				StampToQuery,
				OldDistance,
				NewDistances[Index],
				StampRuntime.GetBlendMode(),
				StampRuntime.GetStamp().bApplyOnVoid,
				StampRuntime.GetSmoothness(),
				&OutAlphas->View()[Index]);

			if (bWriteDistances)
			{
				Query.IndirectDistances[IndirectIndex] = NewDistance;
			}
		}
	}
	else
	{
		check(bWriteDistances);

		for (int32 Index = 0; Index < Query.Num(); Index++)
		{
			const int32 IndirectIndex = Query.GetIndirectIndex(Index);
			const float OldDistance = Query.IndirectDistances[IndirectIndex];

			const float NewDistance = FVoxelBlendModeUtilities::ComputeDistance(
				StampToQuery,
				OldDistance,
				NewDistances[Index],
				StampRuntime.GetBlendMode(),
				StampRuntime.GetStamp().bApplyOnVoid,
				StampRuntime.GetSmoothness(),
				nullptr);

			Query.IndirectDistances[IndirectIndex] = NewDistance;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStampUtilities::ApplySurfaceTypes(
	const FVoxelStampSparseQuery& Query,
	const FVoxelSurfaceTypeBlendBuffer& SurfaceTypes,
	const FVoxelFloatBuffer& Alphas)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(Query.bQuerySurfaceTypes))
	{
		return;
	}

	if (Alphas.IsConstant() &&
		FMath::Max(Alphas.GetConstant(), 0.f) == 0.f)
	{
		return;
	}

	if (SurfaceTypes.IsConstant())
	{
		const FVoxelSurfaceTypeBlend& SurfaceType = SurfaceTypes.GetConstant();

		if (SurfaceType.IsNull())
		{
			return;
		}

		if (SurfaceType.GetLayers().Num() == 1)
		{
			ApplySurfaceType(
				Query,
				SurfaceType.GetLayers()[0].Type,
				Alphas);

			return;
		}
	}

	for (int32 Index = 0; Index < Query.Num(); Index++)
	{
		const float Alpha = FMath::Clamp(Alphas[Index], 0.f, 1.f);
		if (Alpha == 0)
		{
			continue;
		}

		const FVoxelSurfaceTypeBlend& SurfaceType = SurfaceTypes[Index];
		if (SurfaceType.IsNull())
		{
			continue;
		}

		FVoxelSurfaceTypeBlend& SurfaceTypeRef = Query.IndirectSurfaceTypes[Query.GetIndirectIndex(Index)];

		if (Alpha == 1)
		{
			SurfaceTypeRef = SurfaceType;
			continue;
		}

		FVoxelSurfaceTypeBlend::Lerp(
			SurfaceTypeRef,
			SurfaceTypeRef,
			SurfaceType,
			Alpha);
	}
}

void FVoxelStampUtilities::ApplySurfaceType(
	const FVoxelStampSparseQuery& Query,
	const FVoxelSurfaceType SurfaceType,
	const FVoxelFloatBuffer& Alphas)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(Query.bQuerySurfaceTypes) ||
		SurfaceType.IsNull())
	{
		return;
	}

	if (Alphas.IsConstant() &&
		FMath::Max(Alphas.GetConstant(), 0.f) == 0.f)
	{
		return;
	}

	for (int32 Index = 0; Index < Query.Num(); Index++)
	{
		const float Alpha = FMath::Clamp(Alphas[Index], 0.f, 1.f);
		if (Alpha == 0)
		{
			continue;
		}

		FVoxelSurfaceTypeBlend& SurfaceTypeRef = Query.IndirectSurfaceTypes[Query.GetIndirectIndex(Index)];

		if (Alpha == 1)
		{
			SurfaceTypeRef.InitializeFromType(SurfaceType);
			continue;
		}

		FVoxelSurfaceTypeBlend::Lerp(
			SurfaceTypeRef,
			SurfaceTypeRef,
			SurfaceType,
			Alpha);
	}
}

void FVoxelStampUtilities::ApplyMetadatas(
	const FVoxelStampSparseQuery& Query,
	const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
	const TVoxelMap<int32, TSharedRef<const FVoxelBuffer>>& PinIndexToMetadata,
	const FVoxelFloatBuffer& Alphas)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(Query.MetadatasToQuery.Num() > 0);

	for (const FVoxelMetadataRef& Metadata : Query.MetadatasToQuery)
	{
		const FVoxelRuntimeMetadataOverrides::FMetadataValue* Value = MetadataOverrides.MetadataToValue.Find(Metadata);
		if (!Value)
		{
			continue;
		}

		FVoxelBuffer& Buffer = Query.IndirectMetadata.FindChecked(Metadata);

		if (Value->PinIndex == -1)
		{
			Metadata.IndirectBlend(
				Query.Indirection,
				*Value->Constant,
				Alphas,
				Buffer);
		}
		else
		{
			checkVoxelSlow(!Value->Constant.IsValid());

			const TSharedRef<const FVoxelBuffer> NewBuffer = PinIndexToMetadata[Value->PinIndex];

			Metadata.IndirectBlend(
				Query.Indirection,
				*NewBuffer,
				Alphas,
				Buffer);
		}
	}
}

void FVoxelStampUtilities::ComputeMetadatas(
	const FVoxelStampSparseQuery& Query,
	const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
	const FVoxelGraphQueryImpl& GraphQuery,
	const TVoxelNodeEvaluator<FVoxelOutputNode_MetadataBase>& Evaluator,
	const FVoxelFloatBuffer& Alphas)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<int32, TSharedRef<const FVoxelBuffer>> PinIndexToMetadata;
	PinIndexToMetadata.Reserve(Query.MetadatasToQuery.Num());

	for (const FVoxelMetadataRef& Metadata : Query.MetadatasToQuery)
	{
		const FVoxelRuntimeMetadataOverrides::FMetadataValue* Value = MetadataOverrides.MetadataToValue.Find(Metadata);
		if (!Value ||
			Value->PinIndex == -1)
		{
			continue;
		}
		checkVoxelSlow(!Value->Constant.IsValid());

		PinIndexToMetadata.Add_EnsureNew(
			Value->PinIndex,
			Evaluator->MetadataPins[Value->PinIndex].Value.GetSynchronous(GraphQuery));
	}

	ApplyMetadatas(
		Query,
		MetadataOverrides,
		PinIndexToMetadata,
		Alphas);
}