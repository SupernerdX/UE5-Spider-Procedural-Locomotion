// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "VoxelHeightBlendMode.h"
#include "VoxelVolumeBlendMode.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Nodes/VoxelPreviewDataNode.h"
#include "VoxelPreviewDataNode_BaseMetadata.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "VoxelPreviewStampDataNodes.generated.h"

namespace FVoxelGraphParameters
{
	struct FBaseStampPreviewData : FUniformParameter
	{
		const FVoxelGraphQueryImpl& Query;
		const FVoxelNode::TPinRef_Input<FVoxelFloatBuffer>& ValuePin;
		const FVoxelNode::TPinRef_Input<FVoxelSurfaceTypeBlendBuffer>& SurfaceTypePin;
		const TVoxelMap<FVoxelMetadataRef, FVoxelNode::TPinRef_Input<FVoxelBuffer>> MetadataPins;
		FBaseStampPreviewData(
			const FVoxelGraphQueryImpl& Query,
			const FVoxelNode::TPinRef_Input<FVoxelFloatBuffer>& ValuePin,
			const FVoxelNode::TPinRef_Input<FVoxelSurfaceTypeBlendBuffer>& SurfaceTypePin,
			const TVoxelMap<FVoxelMetadataRef, FVoxelNode::TPinRef_Input<FVoxelBuffer>>& MetadataPins)
			: Query(Query)
			, ValuePin(ValuePin)
			, SurfaceTypePin(SurfaceTypePin)
			, MetadataPins(MetadataPins)
		{}
	};
	struct FHeightStampPreviewData : FBaseStampPreviewData
	{
		using FBaseStampPreviewData::FBaseStampPreviewData;
	};
	struct FVolumeStampPreviewData : FBaseStampPreviewData
	{
		using FBaseStampPreviewData::FBaseStampPreviewData;
	};
}

USTRUCT(meta = (Abstract))
struct VOXEL_API FVoxelPreviewDataNode_BaseStampPreviewData : public FVoxelPreviewDataNode_BaseMetadata
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(int32, LOD, nullptr);
	VOXEL_INPUT_PIN(FVoxelSeed, StampSeed, nullptr);

	VOXEL_INPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType, nullptr);

	virtual void Query_WithPosition(
		const FVoxelGraphQueryImpl& Query,
		FVoxelGraphQueryImpl& OutQuery) const override;
	//~ End FVoxelQueryParameterNode Interface
};

USTRUCT(meta = (AllowList = "Height, HeightSpline"))
struct VOXEL_API FVoxelPreviewDataNode_HeightPreviewData : public FVoxelPreviewDataNode_BaseStampPreviewData
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(EVoxelHeightBlendMode, BlendMode, nullptr);
	VOXEL_INPUT_PIN(float, Smoothness, nullptr);

	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Height, nullptr);

	//~ Begin FVoxelQueryParameterNode_BaseStampData Interface
	virtual void Query_WithPosition(
		const FVoxelGraphQueryImpl& Query,
		FVoxelGraphQueryImpl& OutQuery) const override;
	//~ End FVoxelQueryParameterNode_BaseStampData Interface
};

USTRUCT(meta = (AllowList = "Volume, VolumeSpline"))
struct VOXEL_API FVoxelPreviewDataNode_VolumePreviewData : public FVoxelPreviewDataNode_BaseStampPreviewData
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(EVoxelVolumeBlendMode, BlendMode, nullptr);
	VOXEL_INPUT_PIN(float, Smoothness, nullptr);

	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Distance, nullptr);

	//~ Begin FVoxelQueryParameterNode_BaseStampData Interface
	virtual void Query_WithPosition(
		const FVoxelGraphQueryImpl& Query,
		FVoxelGraphQueryImpl& OutQuery) const override;
	//~ End FVoxelQueryParameterNode_BaseStampData Interface
};