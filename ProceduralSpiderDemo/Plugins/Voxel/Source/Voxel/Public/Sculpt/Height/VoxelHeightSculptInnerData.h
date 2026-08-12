// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

/*
#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "VoxelHeightBlendMode.h"

struct FVoxelSurfaceTypeBlend;
struct FVoxelDoubleVector2DBuffer;
struct FVoxelHeightTransform;
struct FVoxelHeightBulkQuery;
struct FVoxelHeightSparseQuery;
struct FVoxelHeightHeightChunk;
struct FVoxelHeightMetadataChunk;
struct FVoxelHeightSurfaceTypeChunk;
struct FVoxelHeightSculptStampRuntime;

template<typename>
class TVoxelHeightChunkTree;
template<typename>
class TVoxelHeightChunkTreeIterator;

class VOXEL_API FVoxelHeightSculptInnerData
{
public:
	const TSharedRef<TVoxelHeightChunkTree<FVoxelHeightHeightChunk>> HeightChunkTree;
	const TSharedRef<TVoxelHeightChunkTree<FVoxelHeightSurfaceTypeChunk>> SurfaceTypeChunkTree;
	TVoxelMap<FVoxelMetadataRef, TSharedPtr<TVoxelHeightChunkTree<FVoxelHeightMetadataChunk>>> MetadataRefToChunkTree;

	float MinHeight = MAX_flt;
	float MaxHeight = -MAX_flt;

	FVoxelHeightSculptInnerData();

	void Serialize(FArchive& Ar);
	void CopyFrom(const FVoxelHeightSculptInnerData& Other);

public:
	void ApplyShape(
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& StampToQuery,
		float ScaleXY,
		EVoxelHeightBlendMode BlendMode,
		bool bApplyOnVoid,
		bool bRelativeHeight) const;

	void ApplyShape(
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery,
		float ScaleXY,
		EVoxelHeightBlendMode BlendMode,
		bool bApplyOnVoid,
		bool bRelativeHeight) const;

	void ApplySurfaceType(
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery,
		float ScaleXY) const;

	void ApplyMetadata(
		const FVoxelMetadataRef& MetadataRef,
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery,
		float ScaleXY) const;

public:
	void GetSurfaceTypes(
		const FVoxelDoubleVector2DBuffer& Positions,
		TVoxelArray<float>& OutAlphas,
		TVoxelArray<FVoxelSurfaceTypeBlend>& OutSurfaceTypes) const;

	void GetMetadatas(
		const FVoxelMetadataRef& MetadataRef,
		const FVoxelDoubleVector2DBuffer& Positions,
		TVoxelArray<float>& OutAlphas,
		TSharedPtr<FVoxelBuffer>& OutMetadatas) const;

private:
	template<typename LambdaType>
	static void ProcessHeights(
		const TVoxelHeightChunkTreeIterator<FVoxelHeightHeightChunk>& Iterator,
		LambdaType Lambda);

	template<typename LambdaType>
	static void ProcessSurfaceTypes(
		const TVoxelHeightChunkTreeIterator<FVoxelHeightSurfaceTypeChunk>& Iterator,
		LambdaType Lambda);

	template<typename InnerType, typename LambdaType>
	static void ProcessMetadatas(
		const TVoxelHeightChunkTreeIterator<FVoxelHeightMetadataChunk>& Iterator,
		LambdaType Lambda);
};*/