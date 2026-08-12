// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphPinEnum.h"
#include "SGraphPinComboBox.h"
#include "VoxelPinType.h"

void SVoxelGraphPinEnum::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	SVoxelGraphPin::Construct({}, InGraphPinObj);
}

TSharedRef<SWidget> SVoxelGraphPinEnum::GetDefaultValueWidget()
{
	const FVoxelPinType InnerType = FVoxelPinType(GraphPinObj->PinType).GetPinDefaultValueType();
	if (!ensure(InnerType.Is<uint8>()))
	{
		return SNullWidget::NullWidget;
	}

	const UEnum* Enum = InnerType.GetEnum();
	if (!ensure(Enum))
	{
		return SNullWidget::NullWidget;
	}

	TArray<int32> Items;
	Items.Reserve(Enum->NumEnums());

	// NumEnums() - 1 is _MAX
	for (int32 Index = 0; Index < Enum->NumEnums() - 1; Index++)
	{
		if (Enum->HasMetaData(TEXT("Hidden"), Index))
		{
			continue;
		}

		Items.Add(Index);
	}

	float EnumSize = 150.f;
	if (Enum->HasMetaData(TEXT("OverrideEnumSize")))
	{
		EnumSize = Enum->GetFloatMetaData("OverrideEnumSize");
	}

	return
		SNew(SBox)
		.MinDesiredWidth(EnumSize)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		[
			SNew(SVoxelDetailComboBox<int32>)
			.NoRefreshDelegate()
			.Options(Items)
			.CurrentOption_Lambda([this, Enum]() -> int32
			{
				const FString SelectedValue = GraphPinObj->GetDefaultAsString();
				int32 ActiveValue = -1;

				// NumEnums() - 1 is _MAX
				for (int32 Index = 0; Index < Enum->NumEnums() - 1; Index++)
				{
					if (Enum->HasMetaData(TEXT("Hidden"), Index))
					{
						continue;
					}

					if (SelectedValue == Enum->GetNameStringByIndex(Index))
					{
						ActiveValue = Index;
					}
				}

				return ActiveValue;
			})
			.OptionText_Lambda([this, Enum](const int32 Index) -> FString
			{
				if (Index == -1)
				{
					return GraphPinObj->GetDefaultAsString() + " (INVALID)";
				}

				return Enum->GetDisplayNameTextByIndex(Index).ToString();
			})
			.OptionToolTip_Lambda([this, Enum](const int32 Index) -> FText
			{
				if (Index == -1)
				{
					return INVTEXT("");
				}

				return Enum->GetToolTipTextByIndex(Index);
			})
			.OnSelection_Lambda([this, Enum](const int32 NewSelection)
			{
				const FString NewValue = Enum->GetNameStringByIndex(NewSelection);
				if (GraphPinObj->GetDefaultAsString() == NewValue)
				{
					return;
				}

				const FVoxelTransaction Transaction(GraphPinObj, "Change Enum Pin Value");
				GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, NewValue);
			})
		];
}