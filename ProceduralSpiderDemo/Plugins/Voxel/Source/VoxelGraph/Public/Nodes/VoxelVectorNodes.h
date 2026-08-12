// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelComputeNode.h"
#include "VoxelTemplateNode.h"
#include "VoxelVectorNodes.generated.h"

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_AbstractVectorBase : public FVoxelTemplateNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelWildcard, ReturnValue);

public:
	virtual FPin* ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const override;

#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override;
	virtual void PromotePin(FVoxelPin& InPin, const FVoxelPinType& NewType) override;
#endif

	virtual bool HasScalarOutput2D() const VOXEL_PURE_VIRTUAL({});
	virtual bool HasScalarOutput3D() const VOXEL_PURE_VIRTUAL({});

	virtual UScriptStruct* GetVector2DInnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetDoubleVector2DInnerNode() const VOXEL_PURE_VIRTUAL({});

	virtual UScriptStruct* GetVector3DInnerNode() const VOXEL_PURE_VIRTUAL({});
	virtual UScriptStruct* GetDoubleVector3DInnerNode() const VOXEL_PURE_VIRTUAL({});
};

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_VectorUnaryOperator : public FVoxelTemplateNode_AbstractVectorBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Vector, nullptr);
};

USTRUCT(meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode_VectorBinaryOperator : public FVoxelTemplateNode_AbstractVectorBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, V2, nullptr);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_CrossProduct_2D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {V1}.x * {V2}.y - {V1}.y * {V2}.x";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_CrossProduct_2D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector2D, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector2D, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {V1}.x * {V2}.y - {V1}.y * {V2}.x";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_CrossProduct_3D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVector, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVector, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = cross({V1}, {V2})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_CrossProduct_3D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelDoubleVector, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = cross({V1}, {V2})";
	}
};

USTRUCT(Category = "Math|Vector Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_CrossProduct : public FVoxelTemplateNode_VectorBinaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual bool HasScalarOutput2D() const override
	{
		return true;
	}
	virtual bool HasScalarOutput3D() const override
	{
		return false;
	}

	virtual UScriptStruct* GetVector2DInnerNode() const override
	{
		return FVoxelComputeNode_CrossProduct_2D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector2DInnerNode() const override
	{
		return FVoxelComputeNode_CrossProduct_2D_Double::StaticStruct();
	}

	virtual UScriptStruct* GetVector3DInnerNode() const override
	{
		return FVoxelComputeNode_CrossProduct_3D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector3DInnerNode() const override
	{
		return FVoxelComputeNode_CrossProduct_3D_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_DotProduct_2D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {V1}.x * {V2}.x + {V1}.y * {V2}.y";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_DotProduct_2D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector2D, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector2D, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {V1}.x * {V2}.x + {V1}.y * {V2}.y";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_DotProduct_3D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVector, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {V1}.x * {V2}.x + {V1}.y * {V2}.y + {V1}.z * {V2}.z";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_DotProduct_3D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {V1}.x * {V2}.x + {V1}.y * {V2}.y + {V1}.z * {V2}.z";
	}
};

USTRUCT(Category = "Math|Vector Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_DotProduct : public FVoxelTemplateNode_VectorBinaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual bool HasScalarOutput2D() const override
	{
		return true;
	}
	virtual bool HasScalarOutput3D() const override
	{
		return true;
	}

	virtual UScriptStruct* GetVector2DInnerNode() const override
	{
		return FVoxelComputeNode_DotProduct_2D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector2DInnerNode() const override
	{
		return FVoxelComputeNode_DotProduct_2D_Double::StaticStruct();
	}

	virtual UScriptStruct* GetVector3DInnerNode() const override
	{
		return FVoxelComputeNode_DotProduct_3D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector3DInnerNode() const override
	{
		return FVoxelComputeNode_DotProduct_3D_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Normalize_2D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVector2D, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = normalize({Vector})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Normalize_2D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector2D, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelDoubleVector2D, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = normalize({Vector})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Normalize_3D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVector, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = normalize({Vector})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Normalize_3D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FVoxelDoubleVector, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = normalize({Vector})";
	}
};

USTRUCT(Category = "Math|Vector Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_Normalize : public FVoxelTemplateNode_VectorUnaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual bool HasScalarOutput2D() const override
	{
		return false;
	}
	virtual bool HasScalarOutput3D() const override
	{
		return false;
	}

	virtual UScriptStruct* GetVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Normalize_2D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Normalize_2D_Double::StaticStruct();
	}

	virtual UScriptStruct* GetVector3DInnerNode() const override
	{
		return FVoxelComputeNode_Normalize_3D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector3DInnerNode() const override
	{
		return FVoxelComputeNode_Normalize_3D_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Length_2D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = length({Vector})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Length_2D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector2D, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = length({Vector})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Length_3D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = length({Vector})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Length_3D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = length({Vector})";
	}
};

USTRUCT(Category = "Math|Vector Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_Length : public FVoxelTemplateNode_VectorUnaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual bool HasScalarOutput2D() const override
	{
		return true;
	}
	virtual bool HasScalarOutput3D() const override
	{
		return true;
	}

	virtual UScriptStruct* GetVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Length_2D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Length_2D_Double::StaticStruct();
	}

	virtual UScriptStruct* GetVector3DInnerNode() const override
	{
		return FVoxelComputeNode_Length_3D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector3DInnerNode() const override
	{
		return FVoxelComputeNode_Length_3D_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_LengthXY_3D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = length(MakeFloat2({Vector}.x, {Vector}.y))";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_LengthXY_3D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, Vector, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = length(MakeFloat2({Vector}.x, {Vector}.y))";
	}
};

USTRUCT(Category = "Math|Vector Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_LengthXY : public FVoxelTemplateNode_VectorUnaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual bool HasScalarOutput2D() const override
	{
		return true;
	}
	virtual bool HasScalarOutput3D() const override
	{
		return true;
	}

	virtual UScriptStruct* GetVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Length_2D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Length_2D_Double::StaticStruct();
	}

	virtual UScriptStruct* GetVector3DInnerNode() const override
	{
		return FVoxelComputeNode_LengthXY_3D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector3DInnerNode() const override
	{
		return FVoxelComputeNode_LengthXY_3D_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Distance_2D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = distance({V1}, {V2})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Distance_2D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector2D, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector2D, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = distance({V1}, {V2})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Distance_3D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVector, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = distance({V1}, {V2})";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Distance_3D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = distance({V1}, {V2})";
	}
};

USTRUCT(Category = "Math|Vector Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_Distance : public FVoxelTemplateNode_VectorBinaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual bool HasScalarOutput2D() const override
	{
		return true;
	}
	virtual bool HasScalarOutput3D() const override
	{
		return true;
	}

	virtual UScriptStruct* GetVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Distance_2D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Distance_2D_Double::StaticStruct();
	}

	virtual UScriptStruct* GetVector3DInnerNode() const override
	{
		return FVoxelComputeNode_Distance_3D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector3DInnerNode() const override
	{
		return FVoxelComputeNode_Distance_3D_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Distance2D_3D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVector, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = distance(MakeFloat2({V1}.x, {V1}.y), MakeFloat2({V2}.x, {V2}.y))";
	}
};

USTRUCT(meta = (Internal))
struct FVoxelComputeNode_Distance2D_3D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, V1, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, V2, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = distance(MakeFloat2({V1}.x, {V1}.y), MakeFloat2({V2}.x, {V2}.y))";
	}
};

USTRUCT(Category = "Math|Vector Operators")
struct VOXELGRAPH_API FVoxelTemplateNode_Distance2D : public FVoxelTemplateNode_VectorBinaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual bool HasScalarOutput2D() const override
	{
		return true;
	}
	virtual bool HasScalarOutput3D() const override
	{
		return true;
	}

	virtual UScriptStruct* GetVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Distance_2D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector2DInnerNode() const override
	{
		return FVoxelComputeNode_Distance_2D_Double::StaticStruct();
	}

	virtual UScriptStruct* GetVector3DInnerNode() const override
	{
		return FVoxelComputeNode_Distance2D_3D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector3DInnerNode() const override
	{
		return FVoxelComputeNode_Distance2D_3D_Double::StaticStruct();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_NormalToSlope_2D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, Normal, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, Slope);


	virtual FString GenerateCode(FCode& Code) const override
	{
		FString Result;
		Result += "const float3 Normalized = normalize(MakeFloat3({Normal}.x, {Normal}.y, 1));";
		Result += "{Slope} = acos(Normalized.z) * (180.f / PI)";
		return Result;
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_NormalToSlope_2D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector2D, Normal, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, Slope);

	virtual FString GenerateCode(FCode& Code) const override
	{
		FString Result;
		Result += "const double3 Normalized = normalize(MakeDouble3({Normal}.x, {Normal}.y, 1));";
		Result += "{Slope} = acos(Normalized.z) * (180.f / PI)";
		return Result;
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_NormalToSlope_3D_Float : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVector, Normal, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, Slope);


	virtual FString GenerateCode(FCode& Code) const override
	{
		FString Result;
		Result += "const float3 Normalized = normalize({Normal});";
		Result += "{Slope} = acos(Normalized.z) * (180.f / PI)";
		return Result;
	}
};

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelComputeNode_NormalToSlope_3D_Double : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FVoxelDoubleVector, Normal, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(double, Slope);

	virtual FString GenerateCode(FCode& Code) const override
	{
		FString Result;
		Result += "const double3 Normalized = normalize({Normal});";
		Result += "{Slope} = acos(Normalized.z) * (180.f / PI)";
		return Result;
	}
};

// Converts surface normal to Slope Degrees
// In 2D: will only work if the gradient is NOT normalized
USTRUCT(Category = "Math|Float")
struct VOXELGRAPH_API FVoxelTemplateNode_NormalToSlope : public FVoxelTemplateNode_VectorUnaryOperator
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	virtual bool HasScalarOutput2D() const override
	{
		return true;
	}
	virtual bool HasScalarOutput3D() const override
	{
		return true;
	}

	virtual UScriptStruct* GetVector2DInnerNode() const override
	{
		return FVoxelComputeNode_NormalToSlope_2D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector2DInnerNode() const override
	{
		return FVoxelComputeNode_NormalToSlope_2D_Double::StaticStruct();
	}

	virtual UScriptStruct* GetVector3DInnerNode() const override
	{
		return FVoxelComputeNode_NormalToSlope_3D_Float::StaticStruct();
	}
	virtual UScriptStruct* GetDoubleVector3DInnerNode() const override
	{
		return FVoxelComputeNode_NormalToSlope_3D_Double::StaticStruct();
	}
};