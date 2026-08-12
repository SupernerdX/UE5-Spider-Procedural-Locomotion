// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelNode_RandomSelect.generated.h"

USTRUCT(Category = "Random")
struct VOXELGRAPH_API FVoxelNode_RandomSelect : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelSeed, Seed, nullptr);
	VOXEL_INPUT_PIN(FVoxelWildcardBuffer, Values, nullptr, ArrayPin);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Weights, 1.f, ArrayPin);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, Result);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, Index);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;

#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
	virtual void PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType) override;
#endif
	//~ End FVoxelNode Interface
};