// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelOutputNode_MetadataBase.h"
#include "VoxelMetadata.h"

void FVoxelOutputNode_MetadataBase::Initialize(FInitializer& Initializer)
{
	for (FMetadataPin& MetadataPin : MetadataPins)
	{
		Initializer.InitializePinRef(MetadataPin.Metadata);
		Initializer.InitializePinRef(MetadataPin.Value);
	}
}

void FVoxelOutputNode_MetadataBase::PostSerialize()
{
	if (PinNames_DEPRECATED.Num() > 0)
	{
		ensure(MetadataNames.Num() == 0);

		for (int32 Index = 0; Index < PinNames_DEPRECATED.Num(); Index++)
		{
			MetadataNames.Add("Entry");
		}
	}

	FixupInputPins();

	Super::PostSerialize();
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelOutputNode_MetadataBase::GetPromotionTypes(const FVoxelPin& Pin) const
{
	for (const FMetadataPin& MetadataPin : MetadataPins)
	{
		if (MetadataPin.Metadata == Pin)
		{
			return FVoxelPinTypeSet::AllBuffers();
		}
	}

	return Super::GetPromotionTypes(Pin);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelOutputNode_MetadataBase::FixupInputPins()
{
	VOXEL_FUNCTION_COUNTER();

	TMap<FName, FVoxelPinType> ValuePinTypes;
	for (const FMetadataPin& MetadataPin : MetadataPins)
	{
		ValuePinTypes.Add(MetadataPin.Value.GetName(), GetPin(MetadataPin.Value).GetType());

		RemovePin(MetadataPin.Metadata.GetName());
		RemovePin(MetadataPin.Value.GetName());
	}

	MetadataPins.Reset();

	TVoxelSet<FName> VisitedNames;
	for (int32 Index = 0; Index < MetadataNames.Num(); Index++)
	{
		FName& Name = MetadataNames[Index];
		while (VisitedNames.Contains(Name))
		{
			Name.SetNumber(Name.GetNumber() + 1);
		}

		VisitedNames.Add(Name);

		const FMetadataPin& MetadataPin = MetadataPins.Add_GetRef(FMetadataPin
		{
			CreateInputPin<FVoxelMetadataRef>(
				"MetadataName_" + Name,
				VOXEL_PIN_METADATA(
					FName,
					nullptr,
					DisplayName("Metadata"),
					Category("Metadata " + LexToString(Index + 1)))),

			TPinRef_Input<FVoxelBuffer>(CreateInputPin(
				FVoxelPinType::MakeWildcardBuffer(),
				"MetadataValue_" + Name,
				VOXEL_PIN_METADATA(
					void,
					nullptr,
					DisplayName("Value"),
					Category("Metadata " + LexToString(Index + 1)))))
		});

		if (const FVoxelPinType* Type = ValuePinTypes.Find(MetadataPin.Value.GetName()))
		{
			GetPin(MetadataPin.Value).SetType(*Type);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
FString FVoxelOutputNode_MetadataBase::FDefinition::GetAddPinLabel() const
{
	return "Add Output";
}

FString FVoxelOutputNode_MetadataBase::FDefinition::GetAddPinTooltip() const
{
	return "Adds an additional metadata output";
}

void FVoxelOutputNode_MetadataBase::FDefinition::AddInputPin()
{
	Node.MetadataNames.Add("Entry");
	Node.FixupInputPins();
}

bool FVoxelOutputNode_MetadataBase::FDefinition::CanRemoveInputPin() const
{
	return Node.MetadataNames.Num() > 0;
}

void FVoxelOutputNode_MetadataBase::FDefinition::RemoveInputPin()
{
	Node.MetadataNames.Pop();
	Node.FixupInputPins();
}

bool FVoxelOutputNode_MetadataBase::FDefinition::CanRemoveSelectedPin(const FName PinName) const
{
	FString BaseName = PinName.ToString();
	BaseName.RemoveFromStart("MetadataName_");
	BaseName.RemoveFromStart("MetadataValue_");
	return Node.MetadataNames.Contains(FName(BaseName));
}

void FVoxelOutputNode_MetadataBase::FDefinition::RemoveSelectedPin(const FName PinName)
{
	FString BaseName = PinName.ToString();
	BaseName.RemoveFromStart("MetadataName_");
	BaseName.RemoveFromStart("MetadataValue_");
	Node.MetadataNames.RemoveSingle(FName(BaseName));

	Node.FixupInputPins();
}

bool FVoxelOutputNode_MetadataBase::FDefinition::OnPinDefaultValueChanged(const FName PinName, const FVoxelPinValue& NewDefaultValue)
{
	FString BaseName = PinName.ToString();
	BaseName.RemoveFromStart("MetadataName_");

	if (!Node.MetadataNames.Contains(FName(BaseName)))
	{
		return false;
	}

	BaseName = "MetadataValue_" + BaseName;

	const TSharedPtr<FVoxelPin> ValuePin = Node.FindPin(FName(BaseName));
	if (!ensure(ValuePin))
	{
		return false;
	}

	const UVoxelMetadata* Value = NewDefaultValue.Get<UVoxelMetadata>();
	if (!Value)
	{
		return false;
	}

	const FVoxelPinType CurrentType = ValuePin->GetType();
	FVoxelPinType NewType = Value->GetInnerType();
	if (ValuePin->GetType().IsBuffer())
	{
		NewType = NewType.GetBufferType();
	}

	if (CurrentType == NewType)
	{
		return false;
	}

	Node.PromotePin(*ValuePin, NewType);
	return true;
}
#endif