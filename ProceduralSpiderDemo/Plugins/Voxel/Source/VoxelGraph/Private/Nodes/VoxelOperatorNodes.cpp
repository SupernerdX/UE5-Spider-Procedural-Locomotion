// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelOperatorNodes.h"
#include "VoxelCompilationGraph.h"

FVoxelTemplateNode::FPin* FVoxelTemplateNode_OperatorBase::ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const
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
	Pins = Apply(Pins, ZeroExpandVector, MaxDimension);
	check(Pins.Num() == NumPins);

	if (NumPins == 1)
	{
		return MakeVector(Call_Multi(NodeStruct, BreakVector(Pins[0])));
	}

	return Reduce(Pins, [&](FPin* PinA, FPin* PinB)
	{
		return MakeVector(Call_Multi(NodeStruct, BreakVector(PinA), BreakVector(PinB)));
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelTemplateNode_OperatorBase::GetPromotionTypes(const FVoxelPin& Pin) const
{
	FVoxelPinTypeSet OutTypes;

	if (GetFloatInnerNode())
	{
		OutTypes.Add(GetFloatTypes());
	}
	if (GetDoubleInnerNode())
	{
		OutTypes.Add(GetDoubleTypes());
	}
	if (GetInt32InnerNode())
	{
		OutTypes.Add(GetInt32Types());
	}
	if (GetInt64InnerNode())
	{
		OutTypes.Add(GetInt64Types());
	}

	return OutTypes;
}

void FVoxelTemplateNode_OperatorBase::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);
	ON_SCOPE_EXIT
	{
		ensure(Pin.GetType() == NewType);
	};

	FixupWildcards(NewType);
	EnforceNoPrecisionLoss(Pin, NewType, GetAllPins());
	FixupBuffers(NewType, GetAllPins());
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void FVoxelTemplateNode_CommutativeAssociativeOperator::FDefinition::AddInputPin()
{
	Variadic_AddPinTo(Node.InputPins.GetName());

	FVoxelPinType Type;
	for (const FVoxelPin& Pin : Node.GetPins())
	{
		if (Pin.GetType().IsWildcard())
		{
			continue;
		}

		Type = Pin.GetType();
		break;
	}

	if (!Type.IsValid())
	{
		return;
	}

	for (FVoxelPin& Pin : Node.GetPins())
	{
		if (Pin.GetType().IsWildcard())
		{
			Pin.SetType(Type);
		}
	}
}
#endif