// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelComputeNode.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelMathNodes.generated.h"

// Linearly interpolates between four input values based on input coordinates
USTRUCT(Category = "Math|Float")
struct VOXELGRAPH_API FVoxelComputeNode_BilinearInterpolation : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Usually frac(Position)
	VOXEL_TEMPLATE_INPUT_PIN(FVector2D, Alpha, nullptr);
	// Value at X=0 Y=0
	VOXEL_TEMPLATE_INPUT_PIN(float, X0Y0, nullptr);
	// Value at X=1 Y=0
	VOXEL_TEMPLATE_INPUT_PIN(float, X1Y0, nullptr);
	// Value at X=0 Y=1
	VOXEL_TEMPLATE_INPUT_PIN(float, X0Y1, nullptr);
	// Value at X=1 Y=1
	VOXEL_TEMPLATE_INPUT_PIN(float, X1Y1, nullptr);
	// Interpolated value
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = BilinearInterpolation({X0Y0}, {X1Y0}, {X0Y1}, {X1Y1}, {Alpha}.x, {Alpha}.y)";
	}
};

// Returns the logical complement of the Boolean value (NOT A)
USTRUCT(Category = "Math|Boolean", DisplayName = "NOT Boolean", meta = (Keywords = "! not negate", CompactNodeTitle = "NOT"))
struct VOXELGRAPH_API FVoxelComputeNode_BooleanNOT : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(bool, A, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = !{A}";
	}
};

// Returns the logical Not OR of two values: !(A || B)
USTRUCT(Category = "Math|Boolean", DisplayName = "NOR Boolean", meta = (Keywords = "!^ nor", CompactNodeTitle = "NOR"))
struct VOXELGRAPH_API FVoxelComputeNode_BooleanNOR : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(bool, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(bool, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = !({A} || {B})";
	}
};

// Returns the logical exclusive OR of two values (A XOR B)
USTRUCT(Category = "Math|Boolean", DisplayName = "XOR Boolean", meta = (Keywords = "^ xor", CompactNodeTitle = "XOR"))
struct VOXELGRAPH_API FVoxelComputeNode_BooleanXOR : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(bool, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(bool, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} != {B}";
	}
};

USTRUCT(Category = "Math|Integer")
struct VOXELGRAPH_API FVoxelComputeNode_LeftShift : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, Value, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, Shift, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "IGNORE_PERF_WARNING\n{ReturnValue} = {Value} << clamp({Shift}, 0, 31)";
	}
};

USTRUCT(Category = "Math|Integer")
struct VOXELGRAPH_API FVoxelComputeNode_RightShift : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, Value, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, Shift, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "IGNORE_PERF_WARNING\n{ReturnValue} = {Value} >> clamp({Shift}, 0, 31)";
	}
};

// Bitwise AND (A & B)
USTRUCT(Category = "Math|Integer", DisplayName = "Bitwise AND", meta = (Keywords = "& and", CompactNodeTitle = "&"))
struct VOXELGRAPH_API FVoxelComputeNode_Bitwise_And : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} & {B}";
	}
};

// Bitwise OR (A | B)
USTRUCT(Category = "Math|Integer", DisplayName = "Bitwise OR", meta = (Keywords = "| or", CompactNodeTitle = "|"))
struct VOXELGRAPH_API FVoxelComputeNode_Bitwise_Or : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} | {B}";
	}
};

// Bitwise XOR (A ^ B)
USTRUCT(Category = "Math|Integer", DisplayName = "Bitwise XOR", meta = (Keywords = "^ xor", CompactNodeTitle = "^"))
struct VOXELGRAPH_API FVoxelComputeNode_Bitwise_Xor : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(int32, B, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = {A} ^ {B}";
	}
};

// Bitwise NOT (~A)
USTRUCT(Category = "Math|Integer", DisplayName = "Bitwise NOT", meta = (Keywords = "~ not", CompactNodeTitle = "~"))
struct VOXELGRAPH_API FVoxelComputeNode_Bitwise_Not : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(int32, A, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(int32, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = ~{A}";
	}
};

USTRUCT(Category = "Math|Rotation", meta = (NativeMakeFunc))
struct VOXELGRAPH_API FVoxelComputeNode_MakeQuaternion : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Roll, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, Pitch, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, Yaw, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(FQuat, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = MakeQuaternionFromEuler({Pitch}, {Yaw}, {Roll})";
	}
};

USTRUCT(Category = "Math|Rotation", meta = (NativeBreakFunc))
struct VOXELGRAPH_API FVoxelComputeNode_BreakQuaternion : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(FQuat, Quat, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, Roll);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, Pitch);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, Yaw);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "MakeEulerFromQuaternion({Quat}, {Pitch}, {Yaw}, {Roll})";
	}
};

// Make a rotation from a X axis. X doesn't need to be normalized
USTRUCT(Category = "Math|Rotation")
struct VOXELGRAPH_API FVoxelComputeNode_MakeRotationFromX : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Doesn't need to be normalized
	VOXEL_TEMPLATE_INPUT_PIN(FVector, X, nullptr);
	// Rotation
	VOXEL_TEMPLATE_OUTPUT_PIN(FQuat, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = MakeQuaternionFromX({X})";
	}
};

// Make a rotation from a Y axis. Y doesn't need to be normalized
USTRUCT(Category = "Math|Rotation")
struct VOXELGRAPH_API FVoxelComputeNode_MakeRotationFromY : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Doesn't need to be normalized
	VOXEL_TEMPLATE_INPUT_PIN(FVector, Y, nullptr);
	// Rotation
	VOXEL_TEMPLATE_OUTPUT_PIN(FQuat, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = MakeQuaternionFromY({Y})";
	}
};

// Make a rotation from a Z axis. Z doesn't need to be normalized
USTRUCT(Category = "Math|Rotation")
struct VOXELGRAPH_API FVoxelComputeNode_MakeRotationFromZ : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	// Doesn't need to be normalized
	VOXEL_TEMPLATE_INPUT_PIN(FVector, Z, nullptr);
	// Rotation
	VOXEL_TEMPLATE_OUTPUT_PIN(FQuat, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = MakeQuaternionFromZ({Z})";
	}
};

USTRUCT(Category = "Color")
struct VOXELGRAPH_API FVoxelComputeNode_DistanceToColor : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Distance, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, Scale, 100.f);
	VOXEL_TEMPLATE_OUTPUT_PIN(FLinearColor, Color);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{Color} = GetDistanceFieldColor({Distance} / {Scale})";
	}
};

// Returns if the float value is Finite
USTRUCT(Category = "Math|Float")
struct VOXELGRAPH_API FVoxelComputeNode_IsFinite : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = IsFinite({Value})";
	}
};

// Use to check if previous stamp values are valid
USTRUCT(Category = "Math|Float", DisplayName = "Is Void")
struct VOXELGRAPH_API FVoxelComputeNode_IsVoid : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(bool, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = isnan({Value})";
	}
};

// Replace NaNs with Fallback
USTRUCT(Category = "Math|Float", DisplayName = "Replace Void", meta = (Keywords = "select"))
struct VOXELGRAPH_API FVoxelComputeNode_ReplaceVoid : public FVoxelComputeNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_TEMPLATE_INPUT_PIN(float, Value, nullptr);
	VOXEL_TEMPLATE_INPUT_PIN(float, Fallback, nullptr);
	VOXEL_TEMPLATE_OUTPUT_PIN(float, ReturnValue);

	virtual FString GenerateCode(FCode& Code) const override
	{
		return "{ReturnValue} = isnan({Value}) ? {Fallback} : {Value}";
	}
};