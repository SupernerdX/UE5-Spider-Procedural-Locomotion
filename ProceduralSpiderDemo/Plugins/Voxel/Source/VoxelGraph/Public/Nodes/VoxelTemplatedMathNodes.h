// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelComputeNode.h"
#include "VoxelTemplateNode.h"
#include "VoxelTemplatedMathNodes.generated.h"

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_FloatMathNode : public FVoxelTemplateNode
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
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Frac_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {Value} - floor({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Frac_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {Value} - floor({Value})";
	}
};

// Returns the fractional part of a float.
USTRUCT(Category = "Math|Float")
struct VOXELGRAPH_API FVoxelTemplateNode_Frac : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, A, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Frac_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Frac_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Sqrt_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = sqrt({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Sqrt_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = sqrt({Value})";
	}
};

// Returns the square root of a value.
USTRUCT(Category = "Math|Float", meta = (CompactNodeTitle = "SQRT"))
struct VOXELGRAPH_API FVoxelTemplateNode_Sqrt : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Value to compute the square root of
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// Square root of the input value
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Sqrt_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Sqrt_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Power_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Base, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, Exp, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		// Base cannot be negative
		return "{ReturnValue} = pow(abs({Base}), {Exp})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Power_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Base, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, Exp, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		// Base cannot be negative
		return "{ReturnValue} = pow(abs({Base}), {Exp})";
	}
};

// Power (Base to the Exp-th power)
USTRUCT(Category = "Math|Float")
struct VOXELGRAPH_API FVoxelTemplateNode_Power : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// If negative, abs(Base) will be used instead
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Base, nullptr);
	// Exponent
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Exp, nullptr);
	// Result
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Power_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Power_Double::StaticStruct();
	}
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Sin_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = sin({Value})";
	}
};


USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Sin_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = sin({Value})";
	}
};


// Returns the sine of A (expects Radians)
USTRUCT(Category = "Math|Trig", DisplayName = "Sin (Radians)", meta = (CompactNodeTitle = "SIN"))
struct VOXELGRAPH_API FVoxelTemplateNode_Sin : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Angle in radians
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// sin(x), between -1 and 1
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Sin_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Sin_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_SinDegrees_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = sin(PI / 180.f * {Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_SinDegrees_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = sin(PI / 180.f * {Value})";
	}
};

// Returns the sine of A (expects Degrees)
USTRUCT(Category = "Math|Trig", DisplayName = "Sin (Degrees)", meta = (CompactNodeTitle = "SINd"))
struct VOXELGRAPH_API FVoxelTemplateNode_SinDegrees : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Angle in degrees
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// sin(x), between -1 and 1
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_SinDegrees_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_SinDegrees_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Cos_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = cos({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Cos_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = cos({Value})";
	}
};

// Returns the cosine of A (expects Radians)
USTRUCT(Category = "Math|Trig", DisplayName = "Cos (Radians)", meta = (CompactNodeTitle = "COS"))
struct VOXELGRAPH_API FVoxelTemplateNode_Cos : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Angle in radians
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// cos(x), between -1 and 1
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Cos_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Cos_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_CosDegrees_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = cos(PI / 180.f * {Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_CosDegrees_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = cos(PI / 180.f * {Value})";
	}
};

// Returns the cosine of A (expects Degrees)
USTRUCT(Category = "Math|Trig", DisplayName = "Cos (Degrees)", meta = (CompactNodeTitle = "COSd"))
struct VOXELGRAPH_API FVoxelTemplateNode_CosDegrees : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Angle in degrees
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// cos(x), between -1 and 1
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_CosDegrees_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_CosDegrees_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Tan_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = tan({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Tan_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = tan({Value})";
	}
};

// Returns the tangent of A (expects Radians)
USTRUCT(Category = "Math|Trig", DisplayName = "Tan (Radians)", meta = (CompactNodeTitle = "TAN"))
struct VOXELGRAPH_API FVoxelTemplateNode_Tan : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Angle in radians
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// tan(x)
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Tan_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Tan_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_TanDegrees_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = tan(PI / 180.f * {Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_TanDegrees_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = tan(PI / 180.f * {Value})";
	}
};

// Returns the tangent of A (expects Degrees)
USTRUCT(Category = "Math|Trig", DisplayName = "Tan (Degrees)", meta = (CompactNodeTitle = "TANd"))
struct VOXELGRAPH_API FVoxelTemplateNode_TanDegrees : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Angle in degrees
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// tan(x)
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_TanDegrees_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_TanDegrees_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Asin_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = asin({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Asin_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = asin({Value})";
	}
};

// Returns the inverse sine (arcsin) of A (result is in Radians)
USTRUCT(Category = "Math|Trig", DisplayName = "Asin (Radians)", meta = (CompactNodeTitle = "ASIN"))
struct VOXELGRAPH_API FVoxelTemplateNode_Asin : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Value between -1 and 1
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// Angle in radians
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Asin_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Asin_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_AsinDegrees_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 180.f / PI * asin({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_AsinDegrees_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 180.f / PI * asin({Value})";
	}
};

// Returns the inverse sine (arcsin) of A (result is in Degrees)
USTRUCT(Category = "Math|Trig", DisplayName = "Asin (Degrees)", meta = (CompactNodeTitle = "ASINd"))
struct VOXELGRAPH_API FVoxelTemplateNode_AsinDegrees : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Value between -1 and 1
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// Angle in degrees
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_AsinDegrees_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_AsinDegrees_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Acos_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = acos({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Acos_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = acos({Value})";
	}
};

// Returns the inverse cosine (arccos) of A (result is in Radians)
USTRUCT(Category = "Math|Trig", DisplayName = "Acos (Radians)", meta = (CompactNodeTitle = "ACOS"))
struct VOXELGRAPH_API FVoxelTemplateNode_Acos : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Value between -1 and 1
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// Angle in radians
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Acos_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Acos_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_AcosDegrees_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 180.f / PI * acos({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_AcosDegrees_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 180.f / PI * acos({Value})";
	}
};

// Returns the inverse cosine (arccos) of A (result is in Degrees)
USTRUCT(Category = "Math|Trig", DisplayName = "Acos (Degrees)", meta = (CompactNodeTitle = "ACOSd"))
struct VOXELGRAPH_API FVoxelTemplateNode_AcosDegrees : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Value between -1 and 1
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// Angle in degrees
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_AcosDegrees_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_AcosDegrees_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Atan_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = atan({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Atan_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = atan({Value})";
	}
};

// Returns the inverse tan (atan) (result is in Radians)
USTRUCT(Category = "Math|Trig", DisplayName = "Atan (Radians)", meta = (CompactNodeTitle = "ATAN"))
struct VOXELGRAPH_API FVoxelTemplateNode_Atan : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Value
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// Angle in radians
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Atan_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Atan_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_AtanDegrees_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 180.f / PI * atan({Value})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_AtanDegrees_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 180.f / PI * atan({Value})";
	}
};

// Returns the inverse tan (atan) (result is in Degrees)
USTRUCT(Category = "Math|Trig", DisplayName = "Atan (Degrees)", meta = (CompactNodeTitle = "ATANd"))
struct VOXELGRAPH_API FVoxelTemplateNode_AtanDegrees : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Value
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);
	// Angle in degrees
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_AtanDegrees_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_AtanDegrees_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Atan2_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Y, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, X, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = atan2({Y}, {X})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Atan2_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Y, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, X, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = atan2({Y}, {X})";
	}
};

// Returns the inverse tan (atan2) of Y/X (result is in Radians)
USTRUCT(Category = "Math|Trig", DisplayName = "Atan2 (Radians)", meta = (CompactNodeTitle = "ATAN2"))
struct VOXELGRAPH_API FVoxelTemplateNode_Atan2 : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Y, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, X, nullptr);
	// Angle in radians
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Atan2_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Atan2_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Atan2Degrees_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Y, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, X, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 180.f / PI * atan2({Y}, {X})";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_Atan2Degrees_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, Y, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(double, X, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = 180.f / PI * atan2({Y}, {X})";
	}
};

// Returns the inverse tan (atan2) of Y/X (result is in Degrees)
USTRUCT(Category = "Math|Trig", DisplayName = "Atan2 (Degrees)", meta = (CompactNodeTitle = "ATAN2d"))
struct VOXELGRAPH_API FVoxelTemplateNode_Atan2Degrees : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Y, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, X, nullptr);
	// Angle in degrees
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_Atan2Degrees_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_Atan2Degrees_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_GetPI_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = PI";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_GetPI_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = PI";
	}
};

USTRUCT(Category = "Math|Trig", DisplayName = "Get PI", meta = (CompactNodeTitle = "PI"))
struct VOXELGRAPH_API FVoxelTemplateNode_GetPI : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_GetPI_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_GetPI_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_RadiansToDegrees_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} * (180.f / PI)";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_RadiansToDegrees_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} * (180.f / PI)";
	}
};

USTRUCT(Category = "Math|Trig", meta = (CompactNodeTitle = "R2D"))
struct VOXELGRAPH_API FVoxelTemplateNode_RadiansToDegrees : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, A, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_RadiansToDegrees_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_RadiansToDegrees_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_DegreesToRadians_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, A, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} * (PI / 180.f)";
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_DegreesToRadians_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(double, A, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} * (PI / 180.f)";
	}
};

USTRUCT(Category = "Math|Trig", meta = (CompactNodeTitle = "D2R"))
struct VOXELGRAPH_API FVoxelTemplateNode_DegreesToRadians : public FVoxelTemplateNode_FloatMathNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, A, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

	virtual UScriptStruct* GetFloatInnerNode() const override
	{
		return FVoxelComputeNode_DegreesToRadians_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleInnerNode() const override
	{
		return FVoxelComputeNode_DegreesToRadians_Double::StaticStruct();
	}
};