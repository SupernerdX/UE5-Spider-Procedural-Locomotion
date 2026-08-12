// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelComputeNode.h"
#include "VoxelTemplateNode.h"
#include "VoxelTemplateNode_NearlyEqual.generated.h"

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_NearlyEqual_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, ErrorTolerance, 1.e-6f);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = abs({A} - {B}) <= {ErrorTolerance}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_NearlyEqual_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, ErrorTolerance, 1.e-6f);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = abs({A} - {B}) <= {ErrorTolerance}";
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Returns true if A is nearly equal to B (|A - B| < ErrorTolerance)
USTRUCT(Category = "Math|Operators", meta = (Keywords = "== equal"))
struct VOXELGRAPH_API FVoxelTemplateNode_NearlyEqual : public FVoxelTemplateNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, B, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, ErrorTolerance, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, Result);

public:
	virtual FPin* ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const override;

#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
	virtual void PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType) override;
#endif
};