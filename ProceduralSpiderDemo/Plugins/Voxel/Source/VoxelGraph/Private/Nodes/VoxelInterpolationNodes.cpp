// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelInterpolationNodes.h"
#include "VoxelCompilationGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphNode.h"

FVoxelTemplateNode::FPin* FVoxelTemplateNode_Interpolate::ExpandPins(FNode& Node, TArray<FPin*> Pins, const TArray<FPin*>& AllPins) const
{
	const int32 NumPins = Pins.Num();
	const int32 MaxDimension = GetDimension(Node.GetOutputPin(0).Type);

	TArray<FPin*> GenericPins = Filter(Pins, [this](const FPin* Pin)
	{
		return
			Pin->Name != InterpolationTypePin.GetName() &&
			Pin->Name != StepsPin.GetName() &&
			Pin->Name != ExpPin.GetName();
	});
	const int32 NumGenericPins = GenericPins.Num();

	TMap<int32, int32> MappedPins;
	for (int32 Index = 0; Index < Pins.Num(); Index++)
	{
		const int32 GenericPinIndex = GenericPins.IndexOfByPredicate([&Pins, &Index](const FPin* Pin)
		{
			return Pin == Pins[Index];
		});

		if (GenericPinIndex == -1)
		{
			continue;
		}

		MappedPins.Add(Index, GenericPinIndex);
	}

	GenericPins = Apply(GenericPins, ConvertToFloat);
	GenericPins = Apply(GenericPins, ScalarToVector, MaxDimension);
	check(GenericPins.Num() == NumGenericPins);

	const TArray<TArray<FPin*>> BrokenPins = ApplyVector(GenericPins, BreakVector);
	check(BrokenPins.Num() == NumGenericPins);

	TArray<TArray<FPin*>> SplitPins;
	for (int32 Index = 0; Index < Pins.Num(); Index++)
	{
		if (const int32* GenericPinIndexPtr = MappedPins.Find(Index))
		{
			SplitPins.Add(BrokenPins[*GenericPinIndexPtr]);
			continue;
		}

		TArray<FPin*> SamePinList;
		for (int32 Dimension = 0; Dimension < MaxDimension; Dimension++)
		{
			SamePinList.Add(Pins[Index]);
		}

		SplitPins.Add(SamePinList);
	}
	check(SplitPins.Num() == NumPins);

	return MakeVector(Call_Multi<FVoxelComputeNode_Interpolate>(SplitPins));
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelTemplateNode_Interpolate::GetPromotionTypes(const FVoxelPin& Pin) const
{
	if (Pin == InterpolationTypePin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add(FVoxelPinType::Make<EVoxelInterpolationType>());
		OutTypes.Add(FVoxelPinType::Make<EVoxelInterpolationType>().GetBufferType());
		return OutTypes;
	}
	else if (Pin == StepsPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add<int32>();
		OutTypes.Add<FVoxelInt32Buffer>();
		return OutTypes;
	}
	else if (Pin == ExpPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add<float>();
		OutTypes.Add<FVoxelFloatBuffer>();
		return OutTypes;
	}

	if (Pin == AlphaPin ||
		Pin == ResultPin)
	{
		FVoxelPinTypeSet OutTypes;
		OutTypes.Add(GetFloatTypes());
		return OutTypes;
	}

	FVoxelPinTypeSet OutTypes;
	OutTypes.Add(GetFloatTypes());
	OutTypes.Add(GetInt32Types());
	return OutTypes;
}

void FVoxelTemplateNode_Interpolate::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	Pin.SetType(NewType);
	ON_SCOPE_EXIT
	{
		ensure(Pin.GetType() == NewType);
	};

	FixupWildcards(NewType);
	EnforceSameDimensions(Pin, NewType, { APin.GetName(), BPin.GetName(), AlphaPin.GetName(), ResultPin.GetName() });

	// Fixup Alpha
	SetPinScalarType<float>(GetPin(AlphaPin));

	// Fixup output
	SetPinScalarType<float>(GetPin(ResultPin));

	FixupBuffers(NewType, GetAllPins());
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool FVoxelTemplateNode_Interpolate::FDefinition::IsPinVisible(const UEdGraphPin* Pin, const UEdGraphNode* GraphNode)
{
	const UEdGraphPin* InterpolationPin = GraphNode->FindPin(VOXEL_PIN_NAME(FVoxelComputeNode_Interpolate, InterpolationTypePin));
	if (!InterpolationPin ||
		InterpolationPin->LinkedTo.Num() > 0)
	{
		return true;
	}

	const FVoxelPinValue PinValue = FVoxelPinValue::MakeFromPinDefaultValue(*InterpolationPin);
	if (Pin->PinName == Node.ExpPin.GetName())
	{
		switch (PinValue.Get<EVoxelInterpolationType>())
		{
		case EVoxelInterpolationType::EaseIn:
		case EVoxelInterpolationType::EaseOut:
		case EVoxelInterpolationType::EaseInOut:
		{
			return true;
		}
		default: return false;
		}
	}
	else if (Pin->PinName == Node.StepsPin.GetName())
	{
		switch (PinValue.Get<EVoxelInterpolationType>())
		{
		case EVoxelInterpolationType::Step:
		{
			return true;
		}
		default: return false;
		}
	}
	else
	{
		return true;
	}
}
#endif