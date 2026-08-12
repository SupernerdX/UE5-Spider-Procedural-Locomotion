// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelTemplatedMathNodes.h"
#include "VoxelCompilationGraph.h"

FVoxelTemplateNode::FPin* FVoxelTemplateNode_FloatMathNode::ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const
{
	const FVoxelPinType OutputType = Node.GetOutputPin(0).Type;
	const int32 NumPins = Pins.Num();
	const int32 MaxDimension = GetDimension(OutputType);

	UScriptStruct* NodeStruct;
	if (IsFloat(OutputType))
	{
		Pins = Apply(Pins, ConvertToFloat);
		NodeStruct = GetFloatInnerNode();
	}
	else if (IsDouble(OutputType))
	{
		Pins = Apply(Pins, ConvertToDouble);
		NodeStruct = GetDoubleInnerNode();
	}
	else
	{
		ensure(false);
		return nullptr;
	}

	Pins = Apply(Pins, ScalarToVector, MaxDimension);
	check(Pins.Num() == NumPins);

	const TArray<TArray<FPin*>> BrokenPins = ApplyVector(Pins, BreakVector);
	check(BrokenPins.Num() == NumPins);

	if (Pins.Num() == 0)
	{
		return MakeVector(Call_NoInput(Node, NodeStruct, MaxDimension, OutputType));
	}

	return MakeVector(Call_Multi(NodeStruct, BrokenPins));
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelTemplateNode_FloatMathNode::GetPromotionTypes(const FVoxelPin& Pin) const
{
	FVoxelPinTypeSet OutTypes;
	OutTypes.Add(GetFloatTypes());
	OutTypes.Add(GetDoubleTypes());
	return OutTypes;
}

void FVoxelTemplateNode_FloatMathNode::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);
	ON_SCOPE_EXIT
	{
		ensure(Pin.GetType() == NewType);
	};

	FixupWildcards(NewType);
	EnforceSameDimensions(Pin, NewType, GetAllPins());
	FixupBuffers(NewType, GetAllPins());
}
#endif