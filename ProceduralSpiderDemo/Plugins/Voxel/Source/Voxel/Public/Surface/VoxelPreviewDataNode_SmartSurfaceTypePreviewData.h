// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelSurfaceTypeBlendBuffer.h"
#include "Graphs/VoxelPreviewStampDataNodes.h"
#include "Graphs/VoxelPreviewDataNode_BaseMetadata.h"
#include "VoxelPreviewDataNode_SmartSurfaceTypePreviewData.generated.h"

namespace FVoxelGraphParameters
{
	struct FSmartSurfaceTypePreview : FBaseStampPreviewData
	{
		using FBaseStampPreviewData::FBaseStampPreviewData;
	};
}

USTRUCT(meta = (AllowList = "SmartSurfaceType"))
struct VOXEL_API FVoxelPreviewDataNode_SmartSurfaceTypePreviewData : public FVoxelPreviewDataNode_BaseMetadata
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType, nullptr);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Distance, nullptr);

	VOXEL_INPUT_PIN(float, NormalGranularity, nullptr);

	//~ Begin FVoxelQueryParameterNode_BaseStampData Interface
	virtual void Query_WithPosition(
		const FVoxelGraphQueryImpl& Query,
		FVoxelGraphQueryImpl& OutQuery) const override;
	//~ End FVoxelQueryParameterNode_BaseStampData Interface
};