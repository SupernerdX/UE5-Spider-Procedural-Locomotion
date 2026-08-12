// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "VoxelTemplateNode.generated.h"

namespace Voxel::Graph
{
	class FPin;
	class FNode;
	class FGraph;
	enum class ENodeType : uint8;
}

// We need to make template node refs deterministic
struct FVoxelTemplateNodeContext
{
	const FVoxelGraphNodeRef& NodeRef;
	int32 Counter = 1;

	explicit FVoxelTemplateNodeContext(const FVoxelGraphNodeRef& NodeRef)
		: NodeRef(NodeRef)
	{
		ensure(NodeRef.TemplateInstance == 0);
	}
};
extern VOXELGRAPH_API FVoxelTemplateNodeContext* GVoxelTemplateNodeContext;

struct VOXELGRAPH_API FVoxelTemplateNodeUtilities
{
public:
	using FPin = Voxel::Graph::FPin;
	using FNode = Voxel::Graph::FNode;
	using FGraph = Voxel::Graph::FGraph;
	using ENodeType = Voxel::Graph::ENodeType;

	static FVoxelGraphNodeRef GetNodeRef()
	{
		check(GVoxelTemplateNodeContext);
		FVoxelGraphNodeRef Result = GVoxelTemplateNodeContext->NodeRef;
		Result.TemplateInstance = GVoxelTemplateNodeContext->Counter++;
		return Result;
	}

	// Keeps the same dimension
	// ie will convert FIntVector to FVector, FVector2D to FIntPoint etc
	template<typename T>
	static void SetPinScalarType(FVoxelPin& Pin)
	{
		checkStatic(
			std::is_same_v<T, float> ||
			std::is_same_v<T, double> ||
			std::is_same_v<T, int32> ||
			std::is_same_v<T, int64>);

		Pin.SetType(FVoxelTemplateNodeUtilities::GetVectorType(FVoxelPinType::Make<T>(), GetDimension(Pin.GetType())));
	}
	static void SetPinDimension(FVoxelPin& Pin, int32 Dimension);

public:
	static TConstVoxelArrayView<FVoxelPinType> GetFloatTypes();
	static TConstVoxelArrayView<FVoxelPinType> GetDoubleTypes();
	static TConstVoxelArrayView<FVoxelPinType> GetInt32Types();
	static TConstVoxelArrayView<FVoxelPinType> GetInt64Types();
	static TConstVoxelArrayView<FVoxelPinType> GetObjectTypes();

public:
	static bool IsBool(const FVoxelPinType& PinType)
	{
		return PinType.GetInnerType().Is<bool>();
	}
	static bool IsByte(const FVoxelPinType& PinType)
	{
		return PinType.GetInnerType().Is<uint8>();
	}
	static bool IsFloat(const FVoxelPinType& PinType)
	{
		return GetFloatTypes().Contains(PinType);
	}
	static bool IsDouble(const FVoxelPinType& PinType)
	{
		return GetDoubleTypes().Contains(PinType);
	}
	static bool IsInt32(const FVoxelPinType& PinType)
	{
		return GetInt32Types().Contains(PinType);
	}
	static bool IsInt64(const FVoxelPinType& PinType)
	{
		return GetInt64Types().Contains(PinType);
	}
	static bool IsObject(const FVoxelPinType& PinType)
	{
		return GetObjectTypes().Contains(PinType);
	}

	static bool IsPinBool(const FPin* Pin);
	static bool IsPinByte(const FPin* Pin);
	static bool IsPinFloat(const FPin* Pin);
	static bool IsPinDouble(const FPin* Pin);
	static bool IsPinInt32(const FPin* Pin);
	static bool IsPinInt64(const FPin* Pin);
	static bool IsPinObject(const FPin* Pin);

	static bool IsPinFloatOrDouble(const FPin* Pin);
	static bool IsPinInt32OrInt64(const FPin* Pin);

public:
	static int32 GetDimension(const FVoxelPinType& PinType);
	static FVoxelPinType GetVectorType(const FVoxelPinType& PinType, int32 Dimension);

	static TSharedPtr<FVoxelNode> GetBreakNode(const FVoxelPinType& PinType);
	static TSharedPtr<FVoxelNode> GetMakeNode(const FVoxelPinType& PinType);

public:
	static int32 GetMaxDimension(const TArray<FPin*>& Pins);
	static FPin* GetLinkedPin(FPin* Pin);

	static FPin* ConvertToByte(FPin* Pin);
	static FPin* ConvertToFloat(FPin* Pin);
	static FPin* ConvertToDouble(FPin* Pin);
	static FPin* ConvertToInt64(FPin* Pin);

	static FPin* ScalarToVector(FPin* Pin, int32 HighestDimension);
	static FPin* ZeroExpandVector(FPin* Pin, int32 HighestDimension);

	static TArray<FPin*> BreakVector(FPin* Pin);
	static TArray<FPin*> BreakNode(FPin* Pin, const TSharedPtr<FVoxelNode>& BreakVoxelNode, int32 NumExpectedOutputPins);

	static FPin* MakeVector(TArray<FPin*> Pins);

	static bool IsPinOfName(const FPin* Pin, TSet<FName> Names);
	static FPin* MakeConstant(const FNode& Node, const FVoxelPinValue& Value);

private:
	static TSharedPtr<FVoxelNode> GetConvertToFloatNode(const FPin* Pin);
	static TSharedPtr<FVoxelNode> GetConvertToDoubleNode(const FPin* Pin);
	static TSharedPtr<FVoxelNode> GetConvertToInt64Node(const FPin* Pin);
	static TSharedPtr<FVoxelNode> GetMakeNode(const FPin* Pin, int32 Dimension);

	static FNode& CreateNode(const FPin* Pin, const TSharedRef<FVoxelNode>& VoxelNode);

public:
	static bool Any(const TArray<FPin*>& Pins, TFunctionRef<bool(FPin*)> Lambda);
	static bool All(const TArray<FPin*>& Pins, TFunctionRef<bool(FPin*)> Lambda);
	static TArray<FPin*> Filter(const TArray<FPin*>& Pins, TFunctionRef<bool(const FPin*)> Lambda);

	static FPin* Reduce(TArray<FPin*> Pins, TFunctionRef<FPin*(FPin*, FPin*)> Lambda);

	template<typename T>
	requires std::derived_from<T, FVoxelNode>
	static FPin* Call_Single(
		const TArray<FPin*>& Pins,
		TOptional<FVoxelPinType> OutputPinType = {})
	{
		return Call_Single(T::StaticStruct(), Pins, OutputPinType);
	}
	template<typename... ArgTypes>
	requires (... && std::is_same_v<ArgTypes, FPin*>)
	static FPin* Call_Single(UScriptStruct* NodeStruct, ArgTypes... Args)
	{
		return Call_Single(NodeStruct, { Args... });
	}
	template<typename T, typename... ArgTypes>
	requires (std::derived_from<T, FVoxelNode> && ... && std::is_same_v<ArgTypes, FPin*>)
	static FPin* Call_Single(ArgTypes... Args)
	{
		return Call_Single(T::StaticStruct(), { Args... });
	}
	static FPin* Call_Single(
		const UScriptStruct* NodeStruct,
		const TArray<FPin*>& Pins,
		TOptional<FVoxelPinType> OutputPinType = {});

	template<typename T>
	requires std::derived_from<T, FVoxelNode>
	static TArray<FPin*> Call_Multi(const TArray<TArray<FPin*>>& Pins)
	{
		return Call_Multi(T::StaticStruct(), Pins);
	}
	template<typename... ArgTypes>
	requires (... && std::is_same_v<ArgTypes, TArray<FPin*>>)
	static TArray<FPin*> Call_Multi(UScriptStruct* NodeStruct, ArgTypes... Args)
	{
		return Call_Multi(NodeStruct, { Args... });
	}
	template<typename T, typename... ArgTypes>
	requires (std::derived_from<T, FVoxelNode> && ... && std::is_same_v<ArgTypes, FPin*>)
	static TArray<FPin*> Call_Multi(ArgTypes... Args)
	{
		return Call_Multi(T::StaticStruct(), { Args... });
	}
	static TArray<FPin*> Call_Multi(
		const UScriptStruct* NodeStruct,
		const TArray<TArray<FPin*>>& Pins,
		TOptional<FVoxelPinType> OutputPinType = {});

	static TArray<FPin*> Call_NoInput(
		const FNode& TargetNode,
		const UScriptStruct* NodeStruct,
		int32 MaxDimension,
		const TOptional<FVoxelPinType>& OutputPinType);

	template<typename LambdaType, typename... ArgTypes>
	requires std::is_invocable_v<LambdaType, FPin*, ArgTypes...>
	static TArray<FPin*> Apply(const TConstVoxelArrayView<FPin*> InPins, LambdaType&& Lambda, ArgTypes... Args)
	{
		TArray<FPin*> Pins(InPins);
		for (FPin*& Pin : Pins)
		{
			Pin = Lambda(Pin, Args...);
		}
		return Pins;
	}

	template<typename LambdaType, typename... ArgTypes>
	requires std::is_invocable_v<LambdaType, FPin*, ArgTypes...>
	static TArray<TArray<FPin*>> ApplyVector(const TConstVoxelArrayView<FPin*> InPins, LambdaType&& Lambda, ArgTypes... Args)
	{
		TArray<TArray<FPin*>> Result;
		TArray<FPin*> Pins(InPins);
		for (FPin*& Pin : Pins)
		{
			Result.Add(Lambda(Pin, Args...));
		}
		return Result;
	}
};

USTRUCT(Category = "Template", meta = (Abstract))
struct VOXELGRAPH_API FVoxelTemplateNode
	: public FVoxelNode
#if CPP
	, public FVoxelTemplateNodeUtilities
#endif
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	FVoxelTemplateNode() = default;

	virtual bool IsPureNode() const override
	{
		return true;
	}

	virtual void ExpandNode(FGraph& Graph, FNode& Node) const;
	virtual void ExpandPins(FNode& Node, const TArray<FPin*> Pins, const TArray<FPin*>& AllPins, TArray<FPin*>& OutPins) const
	{
		OutPins = { ExpandPins(Node, Pins, AllPins) };
	}
	virtual FPin* ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const VOXEL_PURE_VIRTUAL({});

protected:
	using FVoxelTemplateNodeUtilities::GetMaxDimension;

	TVoxelArray<FName> GetAllPins() const;
	int32 GetMaxDimension(const TVoxelArray<FName>& PinNames) const;

	void FixupWildcards(const FVoxelPinType& NewType);
	void FixupBuffers(
		const FVoxelPinType& NewType,
		const TVoxelArray<FName>& PinNames);

	// Setting output will set all inputs to have less or same precision: eg, setting output to int will set all inputs to int
	// Setting input will set output to have at least the same precision: eg, setting input to double will set output to double
	// Will also force output to have a dimension same or equal as max input
	void EnforceNoPrecisionLoss(
		const FVoxelPin& InPin,
		const FVoxelPinType& NewType,
		const TVoxelArray<FName>& PinNames);

	// All pins will have to have the same dimension or be scalars
	// Allowed: float int32 FVector FIntVector
	// Not allowed: float FVector2D FVector
	void EnforceSameDimensions(
		const FVoxelPin& InPin,
		const FVoxelPinType& NewType,
		const TVoxelArray<FName>& PinNames);
};