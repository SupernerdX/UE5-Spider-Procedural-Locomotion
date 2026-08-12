// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelTemplateNode_NearlyEqual.h"
#include "Nodes/VoxelBoolNodes.h"

FVoxelTemplateNode::FPin* FVoxelTemplateNode_NearlyEqual::ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const
{
	const int32 MaxDimension = GetMaxDimension(AllPins);

	const bool bIsDouble = Any(Pins, IsPinDouble);

	if (bIsDouble)
	{
		Pins = Apply(Pins, ConvertToDouble);
	}
	else
	{
		ensure(All(Pins, IsPinFloat));
	}

	Pins = Apply(Pins, ScalarToVector, MaxDimension);
	check(Pins.Num() == 3);

	const TArray<TArray<FPin*>> BrokenPins = ApplyVector(Pins, BreakVector);
	check(BrokenPins.Num() == 3);

	const TArray<FPin*> BooleanPins = bIsDouble
		? Call_Multi<FVoxelComputeNode_NearlyEqual_Double>(BrokenPins)
		: Call_Multi<FVoxelComputeNode_NearlyEqual_Float>(BrokenPins);

	return Reduce(BooleanPins, [&](FPin* PinA, FPin* PinB)
	{
		return Call_Single<FVoxelComputeNode_BooleanAND>(PinA, PinB);
	});
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelTemplateNode_NearlyEqual::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == ErrorTolerancePin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add<float>();
		OutTypes.Add<double>();
		OutTypes.Add<FVoxelFloatBuffer>();
		OutTypes.Add<FVoxelDoubleBuffer>();
		return OutTypes;
	}

	if (Pin == ResultPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add<bool>();
		OutTypes.Add<FVoxelBoolBuffer>();
		return OutTypes;
	}

	FVoxelPinTypeSet OutTypes;
	OutTypes.Add(GetFloatTypes());
	OutTypes.Add(GetDoubleTypes());
	return OutTypes;
}

void FVoxelTemplateNode_NearlyEqual::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);
	ON_SCOPE_EXIT
	{
		ensure(Pin.GetType() == NewType);
	};

	FixupWildcards(NewType);
	EnforceSameDimensions(Pin, NewType, { APin.GetName(), BPin.GetName() });

	// Fixup ErrorTolerance
	SetPinDimension(GetPin(ErrorTolerancePin), 1);

	FixupBuffers(NewType, GetAllPins());
}
#endif