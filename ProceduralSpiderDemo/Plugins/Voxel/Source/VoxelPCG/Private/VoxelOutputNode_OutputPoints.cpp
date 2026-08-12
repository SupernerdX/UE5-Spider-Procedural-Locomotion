// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelOutputNode_OutputPoints.h"

void FVoxelOutputNode_OutputPoints::Initialize(FInitializer& Initializer)
{
	for (FPinRef_Input& Pin : InputPins)
	{
		Initializer.InitializePinRef(Pin);
	}
}

void FVoxelOutputNode_OutputPoints::PostSerialize()
{
	FixupInputPins();

	Super::PostSerialize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelOutputNode_OutputPoints::FixupInputPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (const FPinRef& InputPin : InputPins)
	{
		RemovePin(InputPin.GetName());
	}

	InputPins.Reset();

	TVoxelSet<FName> VisitedPins;
	VisitedPins.Add(VOXEL_PIN_NAME(FVoxelOutputNode_OutputPoints, PointsPin));

	for (int32 Index = 0; Index < PinNames.Num(); Index++)
	{
		FName& PinName = PinNames[Index];

		while (VisitedPins.Contains(PinName))
		{
			PinName.SetNumber(PinName.GetNumber() + 1);
		}
		VisitedPins.Add(PinName);

		InputPins.Add(CreateInputPin<FVoxelPointSet>(
			PinName,
			VOXEL_PIN_METADATA(void, nullptr, DisplayName(PinName.ToString()))));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString FVoxelOutputNode_OutputPoints::FDefinition::GetAddPinLabel() const
{
	return "Add Points Output";
}

FString FVoxelOutputNode_OutputPoints::FDefinition::GetAddPinTooltip() const
{
	return "Adds an additional points output";
}

void FVoxelOutputNode_OutputPoints::FDefinition::AddInputPin()
{
	Node.PinNames.Add("Points");
	Node.FixupInputPins();
	Node.RequestRenamePin(Node.PinNames.Last());
}

bool FVoxelOutputNode_OutputPoints::FDefinition::CanRemoveInputPin() const
{
	return Node.PinNames.Num() > 0;
}

void FVoxelOutputNode_OutputPoints::FDefinition::RemoveInputPin()
{
	Node.PinNames.Pop();
	Node.FixupInputPins();
}

bool FVoxelOutputNode_OutputPoints::FDefinition::CanRemoveSelectedPin(const FName PinName) const
{
	return Node.PinNames.Contains(PinName);
}

void FVoxelOutputNode_OutputPoints::FDefinition::RemoveSelectedPin(const FName PinName)
{
	Node.PinNames.RemoveSingle(PinName);
	Node.FixupInputPins();
}

bool FVoxelOutputNode_OutputPoints::FDefinition::CanRenameSelectedPin(const FName PinName) const
{
	return Node.PinNames.Contains(PinName);
}

bool FVoxelOutputNode_OutputPoints::FDefinition::IsNewPinNameValid(const FName PinName, const FName NewName) const
{
	if (NewName == PinName)
	{
		return true;
	}

	if (NewName.ToString().TrimStartAndEnd().IsEmpty())
	{
		return false;
	}

	return !Node.FindPin(NewName);
}

void FVoxelOutputNode_OutputPoints::FDefinition::RenameSelectedPin(const FName PinName, const FName NewName)
{
	const int32 Index = Node.PinNames.IndexOfByKey(PinName);
	if (!ensure(Index != -1) ||
		Node.PinNames[Index] == NewName)
	{
		return;
	}

	Node.PinNames[Index] = NewName;
	Node.FixupInputPins();
}
#endif