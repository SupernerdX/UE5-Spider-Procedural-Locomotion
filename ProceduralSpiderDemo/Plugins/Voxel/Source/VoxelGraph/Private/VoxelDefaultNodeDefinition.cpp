// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelDefaultNodeDefinition.h"
#include "VoxelNode.h"

#if WITH_EDITOR
TSharedPtr<const IVoxelNodeDefinition::FNode> FVoxelDefaultNodeDefinition::GetInputs() const
{
	return GetPins(true);
}

TSharedPtr<const IVoxelNodeDefinition::FNode> FVoxelDefaultNodeDefinition::GetOutputs() const
{
	return GetPins(false);
}

TSharedPtr<const IVoxelNodeDefinition::FNode> FVoxelDefaultNodeDefinition::GetPins(const bool bIsInput) const
{
	const TSharedRef<FCategoryNode> RootNode = FNode::MakeRoot(bIsInput);

	for (const FName PinName : Node.PrivatePinsOrder)
	{
		const FVoxelNode::FDeferredPin* Pin = Node.PrivateNameToPinBackup.Find(PinName);
		if (!ensure(Pin) ||
			Pin->bIsInput != bIsInput ||
			Pin->IsVariadicChild())
		{
			continue;
		}

		if (Pin->IsVariadicRoot())
		{
			const TSharedRef<FVariadicPinNode> VariadicPinNode = RootNode->FindOrAddCategory(Pin->Metadata.Category)->AddVariadicPin(PinName);
			const TSharedPtr<FVoxelNode::FVariadicPin> VariadicPin = Node.PrivateNameToVariadicPin.FindRef(PinName);
			if (!ensure(VariadicPin))
			{
				continue;
			}

			for (const FName ArrayPin : VariadicPin->Pins)
			{
				VariadicPinNode->AddPin(ArrayPin);
			}
			continue;
		}

		RootNode->FindOrAddCategory(Pin->Metadata.Category)->AddPin(PinName);
	}

	return RootNode;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString FVoxelDefaultNodeDefinition::GetAddPinLabel() const
{
	return "Add pin";
}

FString FVoxelDefaultNodeDefinition::GetAddPinTooltip() const
{
	return "Add pin";
}

FString FVoxelDefaultNodeDefinition::GetRemovePinTooltip() const
{
	return "Remove pin";
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool FVoxelDefaultNodeDefinition::Variadic_CanAddPinTo(const FName VariadicPinName) const
{
	ensure(!VariadicPinName.IsNone());
	return Node.PrivateNameToVariadicPin.Contains(VariadicPinName);
}

FName FVoxelDefaultNodeDefinition::Variadic_AddPinTo(const FName VariadicPinName)
{
	ensure(!VariadicPinName.IsNone());
	return Node.Variadic_AddPin(VariadicPinName);
}

bool FVoxelDefaultNodeDefinition::Variadic_CanRemovePinFrom(const FName VariadicPinName) const
{
	ensure(!VariadicPinName.IsNone());

	const TSharedPtr<FVoxelNode::FVariadicPin> VariadicPin = Node.PrivateNameToVariadicPin.FindRef(VariadicPinName);

	return
		VariadicPin &&
		VariadicPin->Pins.Num() > VariadicPin->PinTemplate.MinVariadicNum;
}

void FVoxelDefaultNodeDefinition::Variadic_RemovePinFrom(const FName VariadicPinName)
{
	ensure(!VariadicPinName.IsNone());

	const TSharedPtr<FVoxelNode::FVariadicPin> VariadicPin = Node.PrivateNameToVariadicPin.FindRef(VariadicPinName);
	if (!ensure(VariadicPin) ||
		!ensure(VariadicPin->Pins.Num() > 0) ||
		!ensure(VariadicPin->Pins.Num() > VariadicPin->PinTemplate.MinVariadicNum))
	{
		return;
	}

	const FName PinName = VariadicPin->Pins.Last();
	Node.RemovePin(PinName);

	Node.ExposedPins.Remove(PinName);
	const int32 ExposedPinIndex = Node.ExposedPinValues.IndexOfByKey(PinName);
	if (ExposedPinIndex != -1)
	{
		Node.ExposedPinValues.RemoveAt(ExposedPinIndex);
	}

	Node.FixupVariadicPinNames(VariadicPinName);
}

bool FVoxelDefaultNodeDefinition::CanRemoveSelectedPin(const FName PinName) const
{
	const TSharedPtr<FVoxelPin> Pin = Node.FindPin(PinName);
	if (!Pin)
	{
		return false;
	}

	if (Pin->VariadicPinName.IsNone())
	{
		return false;
	}

	return Variadic_CanRemovePinFrom(Pin->VariadicPinName);
}

void FVoxelDefaultNodeDefinition::RemoveSelectedPin(const FName PinName)
{
	if (!ensure(CanRemoveSelectedPin(PinName)))
	{
		return;
	}

	const TSharedPtr<FVoxelPin> Pin = Node.FindPin(PinName);

	if (Pin->Metadata.bShowInDetail)
	{
		Node.ExposedPins.Remove(Pin->Name);
		const int32 ExposedPinIndex = Node.ExposedPinValues.IndexOfByKey(Pin->Name);
		if (ExposedPinIndex != -1)
		{
			Node.ExposedPinValues.RemoveAt(ExposedPinIndex);
		}
	}

	Node.RemovePin(Pin->Name);

	Node.FixupVariadicPinNames(Pin->VariadicPinName);
}

void FVoxelDefaultNodeDefinition::InsertPinBefore(const FName PinName)
{
	const TSharedPtr<FVoxelPin> Pin = Node.FindPin(PinName);
	if (!Pin ||
		Pin->VariadicPinName.IsNone())
	{
		return;
	}

	const TSharedPtr<FVoxelNode::FVariadicPin> VariadicPin = Node.PrivateNameToVariadicPin.FindRef(Pin->VariadicPinName);
	if (!VariadicPin)
	{
		return;
	}

	const int32 PinPosition = VariadicPin->Pins.IndexOfByPredicate([&Pin] (const FName& Name)
	{
		return Pin->Name == Name;
	});

	if (!ensure(PinPosition != -1))
	{
		return;
	}

	const FName NewPinName = Node.Variadic_InsertPin(Pin->VariadicPinName, PinPosition);
	Node.SortVariadicPinNames(Pin->VariadicPinName);

	if (Pin->Metadata.bShowInDetail)
	{
		if (Node.ExposedPins.Contains(Pin->Name))
		{
			Node.ExposedPins.Add(NewPinName);
		}
	}
}

void FVoxelDefaultNodeDefinition::DuplicatePin(const FName PinName)
{
	const TSharedPtr<FVoxelPin> Pin = Node.FindPin(PinName);
	if (!Pin ||
		Pin->VariadicPinName.IsNone())
	{
		return;
	}

	const TSharedPtr<FVoxelNode::FVariadicPin> PinArray = Node.PrivateNameToVariadicPin.FindRef(Pin->VariadicPinName);
	if (!PinArray)
	{
		return;
	}

	const int32 PinPosition = PinArray->Pins.IndexOfByPredicate([&Pin] (const FName& Name)
	{
		return Pin->Name == Name;
	});

	if (!ensure(PinPosition != -1))
	{
		return;
	}

	const FName NewPinName = Node.Variadic_InsertPin(Pin->VariadicPinName, PinPosition + 1);
	Node.SortVariadicPinNames(Pin->VariadicPinName);

	if (Pin->Metadata.bShowInDetail)
	{
		FVoxelPinValue NewValue;
		if (const FVoxelNodeExposedPinValue* PinValue = Node.ExposedPinValues.FindByKey(Pin->Name))
		{
			NewValue = PinValue->Value;
		}

		Node.ExposedPinValues.Add({ NewPinName, NewValue });

		if (Node.ExposedPins.Contains(Pin->Name))
		{
			Node.ExposedPins.Add(NewPinName);
		}
	}
}

void FVoxelDefaultNodeDefinition::ExposePin(const FName PinName)
{
	const TSharedPtr<FVoxelPin> Pin = Node.FindPin(PinName);
	if (!Pin ||
		!Pin->Metadata.bShowInDetail)
	{
		return;
	}

	Node.ExposedPins.Add(PinName);
}

bool FVoxelDefaultNodeDefinition::ShouldPromptRenameOnSpawn(const FName PinName) const
{
	return Node.PinToRename == PinName;
}
#endif