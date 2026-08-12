// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Surface/VoxelSurfaceTypeBlend.h"

class FVoxelQuery;
struct FVoxelHeightBulkQuery;
struct FVoxelVolumeBulkQuery;
struct FVoxelHeightSparseQuery;
struct FVoxelVolumeSparseQuery;

namespace ispc
{
#ifndef __ISPC_STRUCT_FVoxelHeightBulkQuery__
#define __ISPC_STRUCT_FVoxelHeightBulkQuery__
struct FVoxelHeightBulkQuery
{
	int2 StartIndex;
	int2 EndIndex;
	double2 Start;
	float Step;

	float* Heights;

	int32 StrideX;
};
#endif

#ifndef __ISPC_STRUCT_FVoxelVolumeBulkQuery__
#define __ISPC_STRUCT_FVoxelVolumeBulkQuery__
struct FVoxelVolumeBulkQuery
{
	int3 StartIndex;
	int3 EndIndex;
	double3 Start;
	float Step;

	float* Distances;

	int32 StrideX;
	int32 StrideXY;
};
#endif

#ifndef __ISPC_STRUCT_FVoxelHeightSparseQuery__
#define __ISPC_STRUCT_FVoxelHeightSparseQuery__
struct FVoxelHeightSparseQuery
{
	float* IndirectHeights;
	int32 Num;

	const double* PositionsX;
	const double* PositionsY;

	const int32* Indirection;
	float* IndirectDistances;
};
#endif

#ifndef __ISPC_STRUCT_FVoxelVolumeSparseQuery__
#define __ISPC_STRUCT_FVoxelVolumeSparseQuery__
struct FVoxelVolumeSparseQuery
{
	float* IndirectDistances;
	int32 Num;

	const double* PositionsX;
	const double* PositionsY;
	const double* PositionsZ;

	const int32* Indirection;
};
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VOXEL_API FVoxelMetadataMap
{
public:
	const TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>>& MetadataToBuffer;

	FVoxelMetadataMap();
	FVoxelMetadataMap(TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>>&&) = delete;
	FVoxelMetadataMap(const TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>>& MetadataToBuffer)
		: MetadataToBuffer(MetadataToBuffer)
	{
	}

public:
	FORCEINLINE FVoxelBuffer* Find(const FVoxelMetadataRef& Metadata) const
	{
		return MetadataToBuffer.FindSmartPtr(Metadata);
	}
	FORCEINLINE FVoxelBuffer& FindChecked(const FVoxelMetadataRef& Metadata) const
	{
		return *MetadataToBuffer.FindChecked(Metadata);
	}
	template<typename T>
	requires std::derived_from<T, FVoxelBuffer>
	FORCEINLINE T& FindChecked(const FVoxelMetadataRef& Metadata) const
	{
		return FindChecked(Metadata).AsChecked<T>();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VOXEL_API FVoxelStampQuery
{
public:
	const FVoxelQuery& Query;

	explicit FVoxelStampQuery(const FVoxelQuery& Query)
		: Query(Query)
	{
	}

public:
	void AddDependency(const FVoxelDependency& Dependency) const;

	void AddDependency(
		const FVoxelDependency2D& Dependency,
		const FVoxelBox2D& Bounds) const;

	void AddDependency(
		const FVoxelDependency3D& Dependency,
		const FVoxelBox& Bounds) const;
};

struct VOXEL_API FVoxelStampSparseQuery : FVoxelStampQuery
{
	const int32 NumPositions;
	const TVoxelArrayView<FVoxelSurfaceTypeBlend> IndirectSurfaceTypes;
	const FVoxelMetadataMap IndirectMetadata;
	const TVoxelOptional<FVoxelInt32Buffer> Indirection;
	bool bQuerySurfaceTypes = false;
	TVoxelArray<FVoxelMetadataRef> MetadatasToQuery;

	FVoxelStampSparseQuery(
		const FVoxelQuery& Query,
		const int32 NumPositions,
		const TVoxelArrayView<FVoxelSurfaceTypeBlend> IndirectSurfaceTypes,
		const FVoxelMetadataMap& IndirectMetadata,
		const TVoxelOptional<FVoxelInt32Buffer>& Indirection,
		const bool bQuerySurfaceTypes,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery)
		: FVoxelStampQuery(Query)
		, NumPositions(NumPositions)
		, IndirectSurfaceTypes(IndirectSurfaceTypes)
		, IndirectMetadata(IndirectMetadata)
		, Indirection(Indirection)
		, bQuerySurfaceTypes(bQuerySurfaceTypes)
		, MetadatasToQuery(MetadatasToQuery)
	{
	}

	FORCEINLINE int32 Num() const
	{
		checkVoxelSlow(!Indirection || Indirection->Num() == NumPositions);
		return NumPositions;
	}
	FORCEINLINE int32 GetIndirectIndex(const int32 Index) const
	{
		return Indirection ? (*Indirection)[Index] : Index;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VOXEL_API FVoxelHeightQueryPrevious
{
	TFunction<void(const FVoxelHeightBulkQuery&)> QueryPrevious_Bulk;
	TFunction<void(const FVoxelHeightSparseQuery&)> QueryPrevious_Sparse;

	void Query(const FVoxelHeightBulkQuery& Query) const;
	void Query(const FVoxelHeightSparseQuery& Query) const;

	template<typename LambdaType>
	explicit FVoxelHeightQueryPrevious(LambdaType Lambda)
		: QueryPrevious_Bulk(Lambda)
		, QueryPrevious_Sparse(Lambda)
	{
	}
};

struct VOXEL_API FVoxelVolumeQueryPrevious
{
	TFunction<void(const FVoxelVolumeBulkQuery&)> QueryPrevious_Bulk;
	TFunction<void(const FVoxelVolumeSparseQuery&)> QueryPrevious_Sparse;

	void Query(const FVoxelVolumeBulkQuery& Query) const;
	void Query(const FVoxelVolumeSparseQuery& Query) const;

	template<typename LambdaType>
	explicit FVoxelVolumeQueryPrevious(LambdaType Lambda)
		: QueryPrevious_Bulk(Lambda)
		, QueryPrevious_Sparse(Lambda)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VOXEL_API FVoxelHeightBulkQuery : FVoxelStampQuery
{
public:
	const FVoxelIntBox2D Indices;
	const FVector2d Start;
	const float Step;

	const TVoxelArrayView<float> Heights;
	const int32 StrideX;

	const FVoxelHeightQueryPrevious* QueryPrevious = nullptr;

	static FVoxelHeightBulkQuery Create(
		const FVoxelQuery& Query,
		TVoxelArrayView<float> Heights,
		const FVector2D& Start,
		const FIntPoint& Size,
		float Step);

	static FVoxelHeightBulkQuery Create(
		const FVoxelQuery& Query,
		TVoxelArrayView<float> Heights,
		const FVector2D& Start,
		const FIntPoint& Size,
		const FVoxelIntBox2D& Indices,
		float Step);

	FVoxelHeightBulkQuery WithQuery(const FVoxelQuery& NewQuery) const
	{
		return FVoxelHeightBulkQuery
		{
			NewQuery,
			Indices,
			Start,
			Step,
			Heights,
			StrideX,
			QueryPrevious
		};
	}

private:
	FVoxelHeightBulkQuery(
		const FVoxelQuery& Query,
		const FVoxelIntBox2D& Indices,
		const FVector2d& Start,
		const float Step,
		const TVoxelArrayView<float> Heights,
		const int32 StrideX,
		const FVoxelHeightQueryPrevious* QueryPrevious)
		: FVoxelStampQuery(Query)
		, Indices(Indices)
		, Start(Start)
		, Step(Step)
		, Heights(Heights)
		, StrideX(StrideX)
		, QueryPrevious(QueryPrevious)
	{
	}

public:
	FORCEINLINE ispc::FVoxelHeightBulkQuery ISPC() const
	{
		return ispc::FVoxelHeightBulkQuery
		{
			GetISPCValue(Indices.Min),
			GetISPCValue(Indices.Max),
			GetISPCValue(Start),
			Step,
			Heights.GetData(),
			StrideX
		};
	}

public:
	FORCEINLINE int32 Num() const
	{
		return Indices.Count_int32();
	}
	FORCEINLINE FVoxelBox2D GetBounds() const
	{
		return Indices.ToVoxelBox2D().Scale(Step).ShiftBy(Start);
	}
	FORCEINLINE int32 GetIndex(const int32 X, const int32 Y) const
	{
		checkVoxelSlow(Indices.Contains(X, Y));
		return X + StrideX * Y;
	}
	FORCEINLINE FVector2d GetPosition(const int32 X, const int32 Y) const
	{
		checkVoxelSlow(Indices.Contains(X, Y));
		return Start + Step * FVector2d(X, Y);
	}
	FORCEINLINE FVector2d GetPositionFromIndex(const int32 Index) const
	{
		const int32 Y = Index / StrideX;
		const int32 X = Index - StrideX * Y;
		checkVoxelSlow(Index == GetIndex(X, Y));

		return GetPosition(X, Y);
	}

public:
	TVoxelOptional<FVoxelHeightBulkQuery> ShrinkTo(const FVoxelBox2D& Bounds) const;

	void Split(
		const FVoxelBox2D& Bounds,
		TVoxelOptional<FVoxelHeightBulkQuery>& Inside,
		TVoxelInlineArray<FVoxelHeightBulkQuery, 1>& Outside) const;
};

struct VOXEL_API FVoxelVolumeBulkQuery : FVoxelStampQuery
{
public:
	const FVoxelIntBox Indices;
	const FVector3d Start;
	const float Step;

	const TVoxelArrayView<float> Distances;
	const int32 StrideX;
	const int32 StrideXY;

	using FQueryHeights = TVoxelFunctionPtr<bool(TVoxelArrayView<float> OutHeights, const FVoxelIntBox2D& Indices)>;

	FQueryHeights QueryHeights;
	const FVoxelVolumeQueryPrevious* QueryPrevious = nullptr;

	static FVoxelVolumeBulkQuery Create(
		const FVoxelQuery& Query,
		TVoxelArrayView<float> Distances,
		const FVector& Start,
		const FIntVector& Size,
		float Step);

	static FVoxelVolumeBulkQuery Create(
		const FVoxelQuery& Query,
		TVoxelArrayView<float> Distances,
		const FVector& Start,
		const FIntVector& Size,
		const FVoxelIntBox& Indices,
		float Step);

	FVoxelVolumeBulkQuery WithQuery(const FVoxelQuery& NewQuery) const
	{
		return FVoxelVolumeBulkQuery
		{
			NewQuery,
			Indices,
			Start,
			Step,
			Distances,
			StrideX,
			StrideXY,
			QueryHeights,
			QueryPrevious
		};
	}

private:
	FVoxelVolumeBulkQuery(
		const FVoxelQuery& Query,
		const FVoxelIntBox& Indices,
		const FVector3d& Start,
		const float Step,
		const TVoxelArrayView<float> Distances,
		const int32 StrideX,
		const int32 StrideXY,
		const FQueryHeights QueryHeights,
		const FVoxelVolumeQueryPrevious* QueryPrevious)
		: FVoxelStampQuery(Query)
		, Indices(Indices)
		, Start(Start)
		, Step(Step)
		, Distances(Distances)
		, StrideX(StrideX)
		, StrideXY(StrideXY)
		, QueryHeights(QueryHeights)
		, QueryPrevious(QueryPrevious)
	{
	}

public:
	FORCEINLINE ispc::FVoxelVolumeBulkQuery ISPC() const
	{
		return ispc::FVoxelVolumeBulkQuery
		{
			GetISPCValue(Indices.Min),
			GetISPCValue(Indices.Max),
			GetISPCValue(Start),
			Step,
			Distances.GetData(),
			StrideX,
			StrideXY
		};
	}

public:
	FORCEINLINE int32 Num() const
	{
		return Indices.Count_int32();
	}
	FORCEINLINE FVoxelBox GetBounds() const
	{
		return Indices.ToVoxelBox().Scale(Step).ShiftBy(Start);
	}
	FORCEINLINE int32 GetIndex(const int32 X, const int32 Y, const int32 Z) const
	{
		checkVoxelSlow(Indices.Contains(X, Y, Z));
		return X + StrideX * Y + StrideXY * Z;
	}
	FORCEINLINE FVector3d GetPosition(const int32 X, const int32 Y, const int32 Z) const
	{
		checkVoxelSlow(Indices.Contains(X, Y, Z));
		return Start + Step * FVector3d(X, Y, Z);
	}

public:
	TVoxelOptional<FVoxelVolumeBulkQuery> ShrinkTo(const FVoxelBox& Bounds) const;

	void Split(
		const FVoxelBox& Bounds,
		TVoxelOptional<FVoxelVolumeBulkQuery>& Inside,
		TVoxelInlineArray<FVoxelVolumeBulkQuery, 1>& Outside) const;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VOXEL_API FVoxelHeightSparseQuery : FVoxelStampSparseQuery
{
public:
	const TVoxelArrayView<float> IndirectHeights;
	const FVoxelDoubleVector2DBuffer Positions;
	const FVoxelBox2D PositionBounds;

	const FVoxelHeightQueryPrevious* QueryPrevious = nullptr;

	static FVoxelHeightSparseQuery Create(
		const FVoxelQuery& Query,
		TVoxelArrayView<float> Heights,
		TVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypes,
		const FVoxelMetadataMap& Metadata,
		const FVoxelDoubleVector2DBuffer& Positions,
		bool bQuerySurfaceTypes,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery);

	FVoxelHeightSparseQuery WithQuery(const FVoxelQuery& NewQuery) const
	{
		return FVoxelHeightSparseQuery
		{
			NewQuery,
			IndirectSurfaceTypes,
			IndirectMetadata,
			Indirection,
			bQuerySurfaceTypes,
			MetadatasToQuery,
			IndirectHeights,
			Positions,
			PositionBounds,
			QueryPrevious
		};
	}
	FVoxelHeightSparseQuery WithHeights(const TVoxelArrayView<float> NewIndirectHeights) const
	{
		checkVoxelSlow(IndirectHeights.Num() == NewIndirectHeights.Num());

		return FVoxelHeightSparseQuery
		{
			Query,
			IndirectSurfaceTypes,
			IndirectMetadata,
			Indirection,
			bQuerySurfaceTypes,
			MetadatasToQuery,
			NewIndirectHeights,
			Positions,
			PositionBounds,
			QueryPrevious
		};
	}

private:
	FVoxelHeightSparseQuery(
		const FVoxelQuery& Query,
		const TVoxelArrayView<FVoxelSurfaceTypeBlend> IndirectSurfaceTypes,
		const FVoxelMetadataMap& IndirectMetadata,
		const TVoxelOptional<FVoxelInt32Buffer>& Indirection,
		const bool bQuerySurfaceTypes,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery,
		const TVoxelArrayView<float>& IndirectHeights,
		const FVoxelDoubleVector2DBuffer& Positions,
		const FVoxelBox2D& PositionBounds,
		const FVoxelHeightQueryPrevious* QueryPrevious)
		: FVoxelStampSparseQuery(
			Query,
			Positions.Num(),
			IndirectSurfaceTypes,
			IndirectMetadata,
			Indirection,
			bQuerySurfaceTypes,
			MetadatasToQuery)
		, IndirectHeights(IndirectHeights)
		, Positions(Positions)
		, PositionBounds(PositionBounds)
		, QueryPrevious(QueryPrevious)
	{
	}

	// For SampleAsVolume
	friend class FVoxelHeightLayer;

public:
	FORCEINLINE ispc::FVoxelHeightSparseQuery ISPC() const
	{
		return ispc::FVoxelHeightSparseQuery
		{
			IndirectHeights.GetData(),
			Num(),
			Positions.X.GetData(),
			Positions.Y.GetData(),
			Indirection ? Indirection->GetData() : nullptr
		};
	}
	FORCEINLINE float& GetHeightRef(const int32 Index) const
	{
		return IndirectHeights[GetIndirectIndex(Index)];
	}

public:
	TVoxelOptional<FVoxelHeightSparseQuery> ShrinkTo(const FVoxelBox2D& Bounds) const;
	TVoxelOptional<FVoxelHeightSparseQuery> ReverseAlphaCull(const FVoxelFloatBuffer& Alphas) const;

	void Split(
		const FVoxelBox2D& Bounds,
		TVoxelOptional<FVoxelHeightSparseQuery>& Inside,
		TVoxelInlineArray<FVoxelHeightSparseQuery, 1>& Outside) const;
};

struct VOXEL_API FVoxelVolumeSparseQuery : FVoxelStampSparseQuery
{
public:
	const TVoxelArrayView<float> IndirectDistances;
	const FVoxelDoubleVectorBuffer Positions;
	const FVoxelBox PositionBounds;

	const FVoxelVolumeQueryPrevious* QueryPrevious = nullptr;

	static FVoxelVolumeSparseQuery Create(
		const FVoxelQuery& Query,
		TVoxelArrayView<float> Distances,
		TVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypes,
		const FVoxelMetadataMap& Metadata,
		const FVoxelDoubleVectorBuffer& Positions,
		bool bQuerySurfaceTypes,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery);

	FVoxelVolumeSparseQuery WithQuery(const FVoxelQuery& NewQuery) const
	{
		return FVoxelVolumeSparseQuery
		{
			NewQuery,
			IndirectSurfaceTypes,
			IndirectMetadata,
			Indirection,
			bQuerySurfaceTypes,
			MetadatasToQuery,
			IndirectDistances,
			Positions,
			PositionBounds,
			QueryPrevious
		};
	}
	FVoxelVolumeSparseQuery WithDistances(const TVoxelArrayView<float> NewIndirectDistances) const
	{
		checkVoxelSlow(IndirectDistances.Num() == NewIndirectDistances.Num());

		return FVoxelVolumeSparseQuery
		{
			Query,
			IndirectSurfaceTypes,
			IndirectMetadata,
			Indirection,
			bQuerySurfaceTypes,
			MetadatasToQuery,
			NewIndirectDistances,
			Positions,
			PositionBounds,
			QueryPrevious
		};
	}

private:
	FVoxelVolumeSparseQuery(
		const FVoxelQuery& Query,
		const TVoxelArrayView<FVoxelSurfaceTypeBlend> IndirectSurfaceTypes,
		const FVoxelMetadataMap& IndirectMetadata,
		const TVoxelOptional<FVoxelInt32Buffer>& Indirection,
		const bool bQuerySurfaceTypes,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery,
		const TVoxelArrayView<float> IndirectDistances,
		const FVoxelDoubleVectorBuffer& Positions,
		const FVoxelBox& PositionBounds,
		const FVoxelVolumeQueryPrevious* QueryPrevious)
		: FVoxelStampSparseQuery(
			Query,
			Positions.Num(),
			IndirectSurfaceTypes,
			IndirectMetadata,
			Indirection,
			bQuerySurfaceTypes,
			MetadatasToQuery)
		, IndirectDistances(IndirectDistances)
		, Positions(Positions)
		, PositionBounds(PositionBounds)
		, QueryPrevious(QueryPrevious)
	{
	}

public:
	FORCEINLINE ispc::FVoxelVolumeSparseQuery ISPC() const
	{
		return ispc::FVoxelVolumeSparseQuery
		{
			IndirectDistances.GetData(),
			Num(),
			Positions.X.GetData(),
			Positions.Y.GetData(),
			Positions.Z.GetData(),
			Indirection ? Indirection->GetData() : nullptr
		};
	}
	FORCEINLINE float& GetDistanceRef(const int32 Index) const
	{
		return IndirectDistances[GetIndirectIndex(Index)];
	}

public:
	TVoxelOptional<FVoxelVolumeSparseQuery> ShrinkTo(const FVoxelBox& Bounds) const;
	TVoxelOptional<FVoxelVolumeSparseQuery> ReverseAlphaCull(const FVoxelFloatBuffer& Alphas) const;

	void Split(
		const FVoxelBox& Bounds,
		TVoxelOptional<FVoxelVolumeSparseQuery>& Inside,
		TVoxelInlineArray<FVoxelVolumeSparseQuery, 1>& Outside) const;
};