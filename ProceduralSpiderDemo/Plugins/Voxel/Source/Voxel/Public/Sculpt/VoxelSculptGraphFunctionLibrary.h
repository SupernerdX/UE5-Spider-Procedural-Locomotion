// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFunctionLibrary.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Nodes/VoxelPreviewDataNode.h"
#include "VoxelSculptGraphFunctionLibrary.generated.h"

namespace FVoxelGraphParameters
{
	struct VOXEL_API FHeightSculpt : FBufferParameter
	{
		FVoxelFloatBuffer PreviousHeights;
		FVoxelBoolBuffer IsValid;

		void Split(
			const FVoxelBufferSplitter& Splitter,
			TConstVoxelArrayView<FHeightSculpt*> OutResult) const;
	};
	struct VOXEL_API FVolumeSculpt : FBufferParameter
	{
		FVoxelFloatBuffer PreviousDistances;

		void Split(
			const FVoxelBufferSplitter& Splitter,
			TConstVoxelArrayView<FVolumeSculpt*> OutResult) const;
	};
}

USTRUCT(meta = (AllowList = "HeightSculpt"))
struct VOXEL_API FVoxelPreviewDataNode_HeightSculptPreviewData : public FVoxelPreviewDataNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Height, nullptr);
	VOXEL_INPUT_PIN(FVoxelBoolBuffer, IsValid, nullptr);

	//~ Begin FVoxelQueryParameterNode_BaseStampData Interface
	virtual void Query_WithPosition(
		const FVoxelGraphQueryImpl& Query,
		FVoxelGraphQueryImpl& OutQuery) const override;
	//~ End FVoxelQueryParameterNode_BaseStampData Interface
};

USTRUCT(meta = (AllowList = "VolumeSculpt"))
struct VOXEL_API FVoxelPreviewDataNode_VolumeSculptPreviewData : public FVoxelPreviewDataNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Distance, nullptr);

	//~ Begin FVoxelQueryParameterNode_BaseStampData Interface
	virtual void Query_WithPosition(
		const FVoxelGraphQueryImpl& Query,
		FVoxelGraphQueryImpl& OutQuery) const override;
	//~ End FVoxelQueryParameterNode_BaseStampData Interface
};

UCLASS()
class VOXEL_API UVoxelSculptGraphFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Sculpt", meta = (AllowList = "HeightSculpt"))
	void GetPreviousHeight(
		FVoxelFloatBuffer& Height,
		FVoxelBoolBuffer& IsValid) const;

	UFUNCTION(Category = "Sculpt", meta = (AllowList = "VolumeSculpt"))
	FVoxelFloatBuffer GetPreviousDistance() const;
};