// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampQuery.h"
#include "VoxelStampTransform.h"
#include "VoxelHeightBlendMode.h"
#include "VoxelVolumeBlendMode.h"
#include "VoxelGraphParameters.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"

namespace FVoxelGraphParameters
{
	struct FQuery : FUniformParameter
	{
		const FVoxelQuery& Query;

		explicit FQuery(const FVoxelQuery& Query)
			: Query(Query)
		{
		}
	};

	struct FStampSeed : FUniformParameter
	{
		FVoxelSeed Seed;

		explicit FStampSeed(const FVoxelSeed& Seed)
			: Seed(Seed)
		{}
	};

	struct FHeightStamp : FUniformParameter
	{
		EVoxelHeightBlendMode BlendMode = EVoxelHeightBlendMode::Max;
		float Smoothness = 100.f;
	};

	struct FVolumeStamp : FUniformParameter
	{
		EVoxelVolumeBlendMode BlendMode = EVoxelVolumeBlendMode::Additive;
		float Smoothness = 100.f;
	};

	struct FHeightQuery : FUniformParameter
	{
		const FVoxelQuery& Query;
		const FVoxelHeightTransform& StampToQuery;
		const FVoxelHeightQueryPrevious* const QueryPrevious;

		explicit FHeightQuery(
			const FVoxelHeightBulkQuery& Query,
			const FVoxelHeightTransform& StampToQuery)
			: Query(Query.Query)
			, StampToQuery(StampToQuery)
			, QueryPrevious(Query.QueryPrevious)
		{
		}
		explicit FHeightQuery(
			const FVoxelHeightSparseQuery& Query,
			const FVoxelHeightTransform& StampToQuery)
			: Query(Query.Query)
			, StampToQuery(StampToQuery)
			, QueryPrevious(Query.QueryPrevious)
		{
		}
	};
	struct FVolumeQuery : FUniformParameter
	{
		const FVoxelQuery& Query;
		const FVoxelVolumeTransform& StampToQuery;
		const FVoxelVolumeQueryPrevious* const QueryPrevious;

		explicit FVolumeQuery(
			const FVoxelVolumeBulkQuery& Query,
			const FVoxelVolumeTransform& StampToQuery)
			: Query(Query.Query)
			, StampToQuery(StampToQuery)
			, QueryPrevious(Query.QueryPrevious)
		{
		}
		explicit FVolumeQuery(
			const FVoxelVolumeSparseQuery& Query,
			const FVoxelVolumeTransform& StampToQuery)
			: Query(Query.Query)
			, StampToQuery(StampToQuery)
			, QueryPrevious(Query.QueryPrevious)
		{
		}
	};

	struct FQueryPreviousBase : FUniformParameter
	{
		bool bQueryPreviousValues = false;
		bool bQueryPreviousSurfaceTypes = false;
		TVoxelArray<FVoxelMetadataRef> PreviousMetadatasToQuery;

		mutable bool bQueried = false;
		mutable FVoxelSurfaceTypeBlendBuffer OutSurfaceTypes;
		mutable TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> OutMetadataToBuffer;
		mutable FVoxelFloatBuffer OutValues;
	};
	struct FHeightQueryPrevious : FQueryPreviousBase
	{
		FVoxelDoubleVector2DBuffer GlobalPositions;
	};
	struct FVolumeQueryPrevious : FQueryPreviousBase
	{
		FVoxelDoubleVectorBuffer GlobalPositions;
	};
}