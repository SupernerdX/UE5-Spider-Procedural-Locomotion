// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelComputeNode.h"
#include "VoxelTemplateNode.h"
#include "VoxelMathConvertNodes.generated.h"

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_AbstractMathConvert : public FVoxelTemplateNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, Result);

public:
	virtual FPin* ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const override;

#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
	virtual void PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType) override;
#endif

	virtual UScriptStruct* GetFloatToFloatInnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetDoubleToDoubleInnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetFloatToInt32InnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetDoubleToInt32InnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetFloatToInt64InnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetDoubleToInt64InnerNode() const VOXEL_PURE_VIRTUAL({});
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Ceil_FloatToFloat : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = ceil({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Ceil_DoubleToDouble : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = ceil({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Ceil_FloatToInt32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int32)ceil({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Ceil_FloatToInt64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int64)ceil({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Ceil_DoubleToInt32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int32)ceil({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Ceil_DoubleToInt64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int64)ceil({Value})";
	}
};

// Rounds decimal values upwards
USTRUCT(Category = "Math|Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_Ceil : public FVoxelTemplateNode_AbstractMathConvert
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatToFloatInnerNode() const override
	{
		return FVoxelComputeNode_Ceil_FloatToFloat::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleToDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Ceil_DoubleToDouble::StaticStruct();
	}
	virtual UScriptStruct* GetFloatToInt32InnerNode() const override
	{
		return FVoxelComputeNode_Ceil_FloatToInt32::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleToInt32InnerNode() const override
	{
		return FVoxelComputeNode_Ceil_DoubleToInt32::StaticStruct();
	}
	virtual UScriptStruct* GetFloatToInt64InnerNode() const override
	{
		return FVoxelComputeNode_Ceil_FloatToInt64::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleToInt64InnerNode() const override
	{
		return FVoxelComputeNode_Ceil_DoubleToInt64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Round_FloatToFloat : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = round({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Round_DoubleToDouble : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = round({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Round_FloatToInt32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int32)round({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Round_FloatToInt64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int64)round({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Round_DoubleToInt32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int32)round({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Round_DoubleToInt64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int64)round({Value})";
	}
};

USTRUCT(Category = "Math|Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_Round : public FVoxelTemplateNode_AbstractMathConvert
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatToFloatInnerNode() const override
	{
		return FVoxelComputeNode_Round_FloatToFloat::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleToDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Round_DoubleToDouble::StaticStruct();
	}
	virtual UScriptStruct* GetFloatToInt32InnerNode() const override
	{
		return FVoxelComputeNode_Round_FloatToInt32::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleToInt32InnerNode() const override
	{
		return FVoxelComputeNode_Round_DoubleToInt32::StaticStruct();
	}
	virtual UScriptStruct* GetFloatToInt64InnerNode() const override
	{
		return FVoxelComputeNode_Round_FloatToInt64::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleToInt64InnerNode() const override
	{
		return FVoxelComputeNode_Round_DoubleToInt64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Floor_FloatToFloat : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = floor({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Floor_DoubleToDouble : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = floor({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Floor_FloatToInt32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int32)floor({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Floor_FloatToInt64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int64)floor({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Floor_DoubleToInt32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int32)floor({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Floor_DoubleToInt64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = (int64)floor({Value})";
	}
};

USTRUCT(Category = "Math|Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_Floor : public FVoxelTemplateNode_AbstractMathConvert
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatToFloatInnerNode() const override
	{
		return FVoxelComputeNode_Floor_FloatToFloat::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleToDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Floor_DoubleToDouble::StaticStruct();
	}
	virtual UScriptStruct* GetFloatToInt32InnerNode() const override
	{
		return FVoxelComputeNode_Floor_FloatToInt32::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleToInt32InnerNode() const override
	{
		return FVoxelComputeNode_Floor_DoubleToInt32::StaticStruct();
	}
	virtual UScriptStruct* GetFloatToInt64InnerNode() const override
	{
		return FVoxelComputeNode_Floor_FloatToInt64::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleToInt64InnerNode() const override
	{
		return FVoxelComputeNode_Floor_DoubleToInt64::StaticStruct();
	}
};