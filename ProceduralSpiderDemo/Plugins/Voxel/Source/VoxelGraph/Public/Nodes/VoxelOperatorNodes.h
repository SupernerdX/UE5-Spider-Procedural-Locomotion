// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelComputeNode.h"
#include "VoxelTemplateNode.h"
#include "VoxelOperatorNodes.generated.h"

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_OperatorBase : public FVoxelTemplateNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

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

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_UnaryOperator : public FVoxelTemplateNode_OperatorBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, Result);
};

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_BinaryOperator : public FVoxelTemplateNode_OperatorBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, Result);
};

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_CommutativeAssociativeOperator : public FVoxelTemplateNode_OperatorBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_VARIADIC_INPUT_PIN(FVoxelWildcard, Input, nullptr, 2);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, Result);

public:
#if WITH_EDITOR
	class FDefinition : public Super::FDefinition
	{
	public:
		GENERATED_VOXEL_NODE_DEFINITION_BODY(FVoxelTemplateNode_CommutativeAssociativeOperator);

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
struct FVoxelComputeNode_Add_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} + {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Add_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} + {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Add_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} + {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Add_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} + {B}";
	}
};

// Addition (A + B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "+", Keywords = "+ add plus", Operator = "+"))
struct VOXELGRAPH_API FVoxelTemplateNode_Add : public FVoxelTemplateNode_CommutativeAssociativeOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Add_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Add_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Add_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Add_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Subtract_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} - {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Subtract_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} - {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Subtract_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} - {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Subtract_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} - {B}";
	}
};

// Subtraction (A - B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "-", Keywords = "- subtract minus", Operator = "-"))
struct VOXELGRAPH_API FVoxelTemplateNode_Subtract : public FVoxelTemplateNode_BinaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Subtract_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Subtract_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Subtract_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Subtract_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Multiply_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} * {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Multiply_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} * {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Multiply_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} * {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Multiply_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} * {B}";
	}
};

// Multiplication (A * B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "*", Keywords = "* multiply", Operator = "*"))
struct VOXELGRAPH_API FVoxelTemplateNode_Multiply : public FVoxelTemplateNode_CommutativeAssociativeOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Multiply_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Multiply_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Multiply_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Multiply_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Divide_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		// Make sure to only generate finite numbers
		// This is mainly to not trigger nan checks, but also
		// because nan behavior might differ based on hardware
		// (typically ARM vs x86)
		return
			"{ReturnValue} = {B} != 0.f "
			"? {A} / {B} "
			// + / 0 = inf
			": {A} > 0.f ? BIG_NUMBER "
			// - / 0 = -inf
			": {A} < 0.f ? -BIG_NUMBER "
			// 0 / 0 = 1
			": 1.f";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Divide_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} / {B}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Divide_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "IGNORE_PERF_WARNING\n{ReturnValue} = {B} != 0 ? {A} / {B} : 0";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Divide_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "IGNORE_PERF_WARNING\n{ReturnValue} = {B} != 0 ? {A} / {B} : 0";
	}
};

// Division (A / B)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "/", Keywords = "/ divide division", Operator = "/"))
struct VOXELGRAPH_API FVoxelTemplateNode_Divide : public FVoxelTemplateNode_BinaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Divide_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Divide_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Divide_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Divide_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Modulus_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "IGNORE_PERF_WARNING\n{ReturnValue} = {B} != 0 ? ({A} - floor({A} / {B}) * {B}) : 0";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Modulus_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "IGNORE_PERF_WARNING\n{ReturnValue} = {B} != 0 ? ({A} - floor({A} / {B}) * {B}) : 0";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Modulus_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "IGNORE_PERF_WARNING\n{ReturnValue} = {B} != 0 ? {A} % {B} : 0";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Modulus_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "IGNORE_PERF_WARNING\n{ReturnValue} = {B} != 0 ? {A} % {B} : 0";
	}
};

USTRUCT(Category = "Math|Operators", meta = (DisplayName = "%", CompactNodeTitle = "%", Keywords = "% modulus", Operator = "%"))
struct VOXELGRAPH_API FVoxelTemplateNode_Modulus : public FVoxelTemplateNode_BinaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Modulus_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Modulus_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Modulus_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Modulus_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Min_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = min({A}, {B})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Min_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = min({A}, {B})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Min_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = min({A}, {B})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Min_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = min({A}, {B})";
	}
};

USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "MIN"))
struct VOXELGRAPH_API FVoxelTemplateNode_Min : public FVoxelTemplateNode_CommutativeAssociativeOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Min_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Min_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Min_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Min_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Max_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = max({A}, {B})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Max_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = max({A}, {B})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Max_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = max({A}, {B})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Max_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int64, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = max({A}, {B})";
	}
};

USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "MAX"))
struct VOXELGRAPH_API FVoxelTemplateNode_Max : public FVoxelTemplateNode_CommutativeAssociativeOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Max_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Max_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Max_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Max_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Abs_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = abs({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Abs_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = abs({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Abs_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = abs({Value})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Abs_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = abs({Value})";
	}
};

// Returns the absolute (positive) value
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "ABS"))
struct VOXELGRAPH_API FVoxelTemplateNode_Abs : public FVoxelTemplateNode_UnaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Abs_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Abs_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Abs_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Abs_Int64::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_OneMinus_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 1 - {Value}";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_OneMinus_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 1 - {Value}";
	}
};

USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "1-X"))
struct VOXELGRAPH_API FVoxelTemplateNode_OneMinus : public FVoxelTemplateNode_UnaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_OneMinus_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_OneMinus_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return nullptr;
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return nullptr;
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Sign_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {Value} < 0.f ? -1.f : {Value} > 0.f ? 1.f : 0.f";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Sign_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {Value} < 0.d ? -1.d : {Value} > 0.f ? 1.d : 0.d";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Sign_Int32 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {Value} < 0 ? -1 : {Value} > 0 ? 1 : 0";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Sign_Int64 : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int64, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int64, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {Value} < 0 ? -1 : {Value} > 0 ? 1 : 0";
	}
};

// Sign (return -1 if A < 0, 0 if A is zero, and +1 if A > 0)
USTRUCT(Category = "Math|Operators", meta = (CompactNodeTitle = "Sign"))
struct VOXELGRAPH_API FVoxelTemplateNode_Sign : public FVoxelTemplateNode_UnaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Sign_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Sign_Double::StaticStruct();
	}
	virtual UScriptStruct* GetInt32InnerNode() const override
	{
		return FVoxelComputeNode_Sign_Int32::StaticStruct();
	}
	virtual UScriptStruct* GetInt64InnerNode() const override
	{
		return FVoxelComputeNode_Sign_Int64::StaticStruct();
	}
};