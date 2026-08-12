// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelCompareNodes.h"

FVoxelTemplateNode::FPin* FVoxelTemplateNode_CompareBase::ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const
{
	check(Pins.Num() == 2);

	UScriptStruct* NodeStruct;
	if (All(Pins, IsPinInt32))
	{
		NodeStruct = GetInt32InnerNode();
	}
	else if (All(Pins, IsPinInt32OrInt64))
	{
		Pins = Apply(Pins, ConvertToInt64);
		NodeStruct = GetInt64InnerNode();
	}
	else if (All(Pins, IsPinFloat))
	{
		NodeStruct = GetFloatInnerNode();
	}
	else
	{
		// If we have int32 also use double to ensure no precision loss
		Pins = Apply(Pins, ConvertToDouble);
		NodeStruct = GetDoubleInnerNode();
	}

	return Call_Single(NodeStruct, Pins);
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelTemplateNode_CompareBase::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == ResultPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add<bool>();
		OutTypes.Add<FVoxelBoolBuffer>();
		return OutTypes;
	}

	FVoxelPinTypeSet OutTypes;

	if (GetFloatInnerNode())
	{
		OutTypes.Add<float>();
		OutTypes.Add<FVoxelFloatBuffer>();
	}
	if (GetDoubleInnerNode())
	{
		OutTypes.Add<double>();
		OutTypes.Add<FVoxelDoubleBuffer>();
	}
	if (GetInt32InnerNode())
	{
		OutTypes.Add<int32>();
		OutTypes.Add<FVoxelInt32Buffer>();
	}
	if (GetInt64InnerNode())
	{
		OutTypes.Add<int64>();
		OutTypes.Add<FVoxelInt64Buffer>();
	}

	return OutTypes;
}

void FVoxelTemplateNode_CompareBase::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);
	ON_SCOPE_EXIT
	{
		ensure(Pin.GetType() == NewType);
	};

	FixupWildcards(NewType);
	FixupBuffers(NewType, GetAllPins());
}
#endif