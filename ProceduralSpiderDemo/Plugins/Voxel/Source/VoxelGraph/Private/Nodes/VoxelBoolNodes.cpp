// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelBoolNodes.h"

FVoxelTemplateNode::FPin* FVoxelTemplateNode_MultiInputBooleanNode::ExpandPins(FNode& Node, const TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const
{
	return Reduce(Pins, [&](FPin* PinA, FPin* PinB)
	{
		return Call_Single(GetBooleanNode(), PinA, PinB);
	});
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelTemplateNode_MultiInputBooleanNode::GetPromotionTypes(const FVoxelPin& Pin) const
{
	FVoxelPinTypeSet OutTypes;
	OutTypes.Add<bool>();
	OutTypes.Add<FVoxelBoolBuffer>();
	return OutTypes;
}

void FVoxelTemplateNode_MultiInputBooleanNode::PromotePin(FVoxelPin& InPin, const FVoxelPinType& NewType)
{
	for (FVoxelPin& Pin : GetPins())
	{
		Pin.SetType(NewType);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void FVoxelTemplateNode_MultiInputBooleanNode::FDefinition::AddInputPin()
{
	Variadic_AddPinTo(Node.InputPins.GetName());

	const FVoxelPinType BoolType = Node.GetPin(Node.ResultPin).GetType();
	for (FVoxelPin& Pin : Node.GetPins())
	{
		Pin.SetType(BoolType);
	}
}
#endif