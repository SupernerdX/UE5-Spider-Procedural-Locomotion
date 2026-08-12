// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Nodes/VoxelPreviewDataNode.h"
#include "VoxelPreviewDataNode_SplinePreviewData.generated.h"

USTRUCT(meta = (AllowList = "HeightSpline, VolumeSpline"))
struct VOXEL_API FVoxelPreviewDataNode_SplinePreviewData : public FVoxelPreviewDataNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, Positions, nullptr, ArrayPin);
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, ArriveTangents, nullptr, ArrayPin);
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, LeaveTangents, nullptr, ArrayPin);
	VOXEL_INPUT_PIN(FVoxelQuaternionBuffer, Rotations, nullptr, ArrayPin);
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, Scales, nullptr, ArrayPin);
	VOXEL_TYPED_INPUT_PIN(FVoxelByteBuffer, FVoxelPinType::Make<EInterpCurveMode>().GetBufferType(), Types, nullptr, ArrayPin);
	VOXEL_INPUT_PIN(bool, ClosedLoop, false);
	VOXEL_INPUT_PIN(int32, ReparamStepsPerSegment, 10);

public:
	//~ Begin FVoxelPreviewDataNode Interface
	virtual void Query_WithPosition(
		const FVoxelGraphQueryImpl& Query,
		FVoxelGraphQueryImpl& OutQuery) const override;
	//~ End FVoxelPreviewDataNode Interface
};