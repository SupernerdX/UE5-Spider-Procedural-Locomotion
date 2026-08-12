// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelComputeNode.h"
#include "VoxelObjectNodes.h"
#include "VoxelTemplateNode.h"
#include "VoxelBoolNodes.generated.h"

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_MultiInputBooleanNode : public FVoxelTemplateNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_VARIADIC_INPUT_PIN(bool, Input, false, 2);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, Result);

public:
	virtual bool ShowPromotablePinsAsWildcards() const override
	{
		return false;
	}

	virtual FPin* ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const override;

#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
	virtual void PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType) override;
#endif

	virtual UScriptStruct* GetBooleanNode() const VOXEL_PURE_VIRTUAL({});

public:
#if WITH_EDITOR
	struct FDefinition : public Super::FDefinition
	{
		GENERATED_VOXEL_NODE_DEFINITION_BODY(FVoxelTemplateNode_MultiInputBooleanNode);

		virtual bool CanAddInputPin() const override
		{
			return Variadic_CanAddPinTo(Node.InputPins.GetName());
		}
		virtual void AddInputPin() override;

		virtual bool CanRemoveInputPin() const override
		{
			return Variadic_CanRemovePinFrom(Node.InputPins.GetName());
		}
		virtual void RemoveInputPin() override
		{
			Variadic_RemovePinFrom(Node.InputPins.GetName());
		}
	};
#endif
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_BooleanAND : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(bool, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(bool, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} && {B}";
	}
};

// Returns the logical AND of two values (A AND B)
USTRUCT(Category = "Math|Boolean", meta = (DisplayName = "AND Boolean", CompactNodeTitle = "AND", Keywords = "& and"))
struct VOXELGRAPH_API FVoxelTemplateNode_BooleanAND : public FVoxelTemplateNode_MultiInputBooleanNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetBooleanNode() const override
	{
		return FVoxelComputeNode_BooleanAND::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_BooleanOR : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(bool, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(bool, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} || {B}";
	}
};

// Returns the logical OR of two values (A OR B)
USTRUCT(Category = "Math|Boolean", meta = (DisplayName = "OR Boolean", CompactNodeTitle = "OR", Keywords = "| or"))
struct VOXELGRAPH_API FVoxelTemplateNode_BooleanOR : public FVoxelTemplateNode_MultiInputBooleanNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetBooleanNode() const override
	{
		return FVoxelComputeNode_BooleanOR::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_BooleanNAND : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(bool, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(bool, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = !({A} && {B})";
	}
};

// Returns the logical NAND of two values NOT (A AND B)
USTRUCT(Category = "Math|Boolean", meta = (DisplayName = "NAND Boolean", CompactNodeTitle = "NAND", Keywords = "!& nand"))
struct VOXELGRAPH_API FVoxelTemplateNode_BooleanNAND : public FVoxelTemplateNode_MultiInputBooleanNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetBooleanNode() const override
	{
		return FVoxelComputeNode_BooleanNAND::StaticStruct();
	}
};