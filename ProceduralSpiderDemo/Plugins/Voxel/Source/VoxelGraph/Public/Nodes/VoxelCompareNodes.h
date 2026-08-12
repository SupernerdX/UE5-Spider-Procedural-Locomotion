// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelComputeNode.h"
#include "VoxelTemplateNode.h"
#include "VoxelCompareNodes.generated.h"

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_CompareBase : public FVoxelTemplateNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, Result);

public:
	virtual FPin* ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const override;

#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
	virtual void PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType) override;
#endif

	virtual UScriptStruct* GetFloatInnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetDoubleInnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetInt32InnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetInt64InnerNode() const VOXEL_PURE_VIRTUAL({});
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Less_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} < {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Less_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} < {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Less_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} < {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Less_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} < {B}";
	}
};

// Returns true if A is Less than B (A < B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "<", Keywords = "< less"))
struct VOXELGRAPH_API FVoxelTemplateNode_Less : public FVoxelTemplateNode_CompareBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Less_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Less_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Less_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Less_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Greater_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} > {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Greater_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} > {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Greater_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} > {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Greater_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} > {B}";
	}
};

// Returns true if A is greater than B (A > B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = ">", Keywords = "> greater"))
struct VOXELGRAPH_API FVoxelTemplateNode_Greater : public FVoxelTemplateNode_CompareBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Greater_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Greater_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Greater_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Greater_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_LessEqual_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} <= {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_LessEqual_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} <= {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_LessEqual_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} <= {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_LessEqual_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} <= {B}";
	}
};

// Returns true if A is less than or equal to B (A <= B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "<=", Keywords = "<= less"))
struct VOXELGRAPH_API FVoxelTemplateNode_LessEqual : public FVoxelTemplateNode_CompareBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_LessEqual_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_LessEqual_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_LessEqual_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_LessEqual_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_GreaterEqual_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} >= {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_GreaterEqual_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} >= {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_GreaterEqual_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} >= {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_GreaterEqual_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} >= {B}";
	}
};

// Returns true if A is greater than or equal to B (A >= B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = ">=", Keywords = ">= greater"))
struct VOXELGRAPH_API FVoxelTemplateNode_GreaterEqual : public FVoxelTemplateNode_CompareBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_GreaterEqual_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_GreaterEqual_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_GreaterEqual_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_GreaterEqual_Int64::StaticStruct();
	}
};