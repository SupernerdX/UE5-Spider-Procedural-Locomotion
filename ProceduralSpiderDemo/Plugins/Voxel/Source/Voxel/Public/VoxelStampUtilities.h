// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelStampQuery.h"
#include "VoxelNodeEvaluator.h"
#include "VoxelStampTransform.h"
#include "Surface/VoxelSurfaceTypeBlend.h"

struct FVoxelStampRuntime;
struct FVoxelHeightStampRuntime;
struct FVoxelVolumeStampRuntime;
struct FVoxelSurfaceTypeBlendBuffer;
struct FVoxelRuntimeMetadataOverrides;
struct FVoxelOutputNode_MetadataBase;
struct FVoxelOutputNode_OutputVolumeBase;
struct FVoxelOutputNode_OutputHeightBase;

struct VOXEL_API FVoxelStampUtilities
{
public:
	static FVoxelDoubleVector2DBuffer ComputePositions(
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& StampToQuery);

	static FVoxelDoubleVector2DBuffer ComputePositions(
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery);

public:
	static FVoxelDoubleVectorBuffer ComputePositions(
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& StampToQuery);

	static FVoxelDoubleVectorBuffer ComputePositions(
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery);

public:
	static void ApplyHeights(
		const FVoxelHeightStampRuntime& StampRuntime,
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& StampToQuery,
		const FVoxelFloatBuffer& NewHeights,
		const FVoxelDoubleVector2DBuffer& Positions);

	static void ApplyDistances(
		const FVoxelVolumeStampRuntime& StampRuntime,
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& StampToQuery,
		const FVoxelFloatBuffer& NewDistances);

public:
	static void ComputeHeightStamp(
		const FVoxelHeightStampRuntime& StampRuntime,
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery,
		const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
		FVoxelGraphQueryImpl& GraphQuery,
		const TVoxelNodeEvaluator<FVoxelOutputNode_OutputHeightBase>& Evaluator,
		const FVoxelDoubleVector2DBuffer& Positions);

	static void ComputeVolumeStamp(
		const FVoxelVolumeStampRuntime& StampRuntime,
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery,
		const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
		FVoxelGraphQueryImpl& GraphQuery,
		const TVoxelNodeEvaluator<FVoxelOutputNode_OutputVolumeBase>& Evaluator,
		const FVoxelDoubleVectorBuffer& Positions);

private:
	template<bool bIsHeight>
	static void ComputeOverrideStamp(
		const FVoxelStampRuntime& StampRuntime,
		const std::conditional_t<bIsHeight, FVoxelHeightSparseQuery, FVoxelVolumeSparseQuery>& Query,
		const std::conditional_t<bIsHeight, FVoxelHeightTransform, FVoxelVolumeTransform>& StampToQuery,
		const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
		FVoxelGraphQueryImpl& GraphQuery,
		const TVoxelNodeEvaluator<std::conditional_t<bIsHeight, FVoxelOutputNode_OutputHeightBase, FVoxelOutputNode_OutputVolumeBase>>& Evaluator,
		const std::conditional_t<bIsHeight, FVoxelDoubleVector2DBuffer, FVoxelDoubleVectorBuffer>& Positions);

public:
	static void ApplyHeights(
		const FVoxelHeightStampRuntime& StampRuntime,
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery,
		const FVoxelFloatBuffer& NewHeights,
		const FVoxelDoubleVector2DBuffer& Positions,
		bool bWriteHeights,
		FVoxelFloatBuffer* OutAlphas);

	static void ApplyDistances(
		const FVoxelVolumeStampRuntime& StampRuntime,
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery,
		const FVoxelFloatBuffer& NewDistances,
		bool bWriteDistances,
		FVoxelFloatBuffer* OutAlphas);

public:
	static void ApplySurfaceTypes(
		const FVoxelStampSparseQuery& Query,
		const FVoxelSurfaceTypeBlendBuffer& SurfaceTypes,
		const FVoxelFloatBuffer& Alphas);

	static void ApplySurfaceType(
		const FVoxelStampSparseQuery& Query,
		FVoxelSurfaceType SurfaceType,
		const FVoxelFloatBuffer& Alphas);

	static void ApplyMetadatas(
		const FVoxelStampSparseQuery& Query,
		const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
		const TVoxelMap<int32, TSharedRef<const FVoxelBuffer>>& PinIndexToMetadata,
		const FVoxelFloatBuffer& Alphas);

	static void ComputeMetadatas(
		const FVoxelStampSparseQuery& Query,
		const FVoxelRuntimeMetadataOverrides& MetadataOverrides,
		const FVoxelGraphQueryImpl& GraphQuery,
		const TVoxelNodeEvaluator<FVoxelOutputNode_MetadataBase>& Evaluator,
		const FVoxelFloatBuffer& Alphas);
};