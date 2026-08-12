// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelFunctionInputNodes.generated.h"

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_FunctionInputBase : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	UPROPERTY()
	FGuid Guid;

	//~ Begin FVoxelNode Interface
#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
#endif
	//~ End FVoxelNode Interface
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_FunctionInput : public FVoxelNode_FunctionInputBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, Value);
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_FunctionInputDefault : public FVoxelNode_FunctionInputBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Default, nullptr);
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_FunctionInputPreview : public FVoxelNode_FunctionInputBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Preview, nullptr);
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_FunctionInput_WithDefaults : public FVoxelNode_FunctionInputBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Default, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Preview, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, Value);

public:
	UPROPERTY()
	bool bHasDefaultNode = false;

	UPROPERTY()
	bool bHasPreviewNode = false;

	// Default value for preview and or array types with default and no default node
	UPROPERTY()
	FVoxelPinValue DefaultValue;

public:
	//~ Begin FVoxelNode Interface
	virtual void Initialize(FInitializer& Initializer) override;
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface

private:
	FVoxelRuntimePinValue RuntimeDefaultValue;
};