// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelClampNodes.h"
#include "VoxelCompilationGraph.h"

FVoxelTemplateNode::FPin* FVoxelTemplateNode_AbstractClampBase::ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const
{
	const FVoxelPinType OutputType = Node.GetOutputPin(0).Type;
	const int32 NumPins = Pins.Num();
	const int32 MaxDimension = GetDimension(OutputType);

	UScriptStruct* NodeStruct;
	if (IsInt32(OutputType))
	{
		NodeStruct = GetInt32InnerNode();
	}
	else if (IsInt64(OutputType))
	{
		Pins = Apply(Pins, ConvertToInt64);
		NodeStruct = GetInt64InnerNode();
	}
	else if (IsFloat(OutputType))
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

	if (!ensure(NodeStruct))
	{
		return nullptr;
	}

	Pins = Apply(Pins, ScalarToVector, MaxDimension);
	check(Pins.Num() == NumPins);

	const TArray<TArray<FPin*>> BrokenPins = ApplyVector(Pins, BreakVector);
	check(BrokenPins.Num() == NumPins);

	return MakeVector(Call_Multi(NodeStruct, BrokenPins));
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelTemplateNode_AbstractClampBase::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == ResultPin &&
		!GetInt32InnerNode())
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add(GetFloatTypes());
		OutTypes.Add(GetDoubleTypes());
		return OutTypes;
	}

	FVoxelPinTypeSet OutTypes;
	OutTypes.Add(GetFloatTypes());
	OutTypes.Add(GetDoubleTypes());
	OutTypes.Add(GetInt32Types());
	OutTypes.Add(GetInt64Types());
	return OutTypes;
}

void FVoxelTemplateNode_AbstractClampBase::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);
	ON_SCOPE_EXIT
	{
		ensure(Pin.GetType() == NewType);
	};

	FixupWildcards(NewType);
	EnforceNoPrecisionLoss(Pin, NewType, GetAllPins());
	EnforceSameDimensions(Pin, NewType, GetAllPins());
	FixupBuffers(NewType, GetAllPins());
}
#endif