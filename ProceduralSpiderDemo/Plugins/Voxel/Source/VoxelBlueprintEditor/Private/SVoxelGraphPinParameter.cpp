// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphPinParameter.h"
#include "VoxelParameterView.h"
#include "VoxelGraphParametersView.h"
#include "SVoxelGraphParameterComboBox.h"
#include "K2Node_VoxelGraphParameterBase.h"

void SVoxelGraphPinParameter::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	if (const UK2Node_VoxelGraphParameterBase* Node = Cast<UK2Node_VoxelGraphParameterBase>(InGraphPinObj->GetOwningNode()))
	{
		if (UEdGraphPin* AssetPin = Node->FindPin(Node->GetGraphPinName()))
		{
			ensure(!AssetPin->DefaultObject || AssetPin->DefaultObject->IsA<UVoxelGraph>());

			WeakGraph = Cast<UVoxelGraph>(AssetPin->DefaultObject);
			GraphPinReference = AssetPin;
		}
	}

	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

void SVoxelGraphPinParameter::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SGraphPin::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bGraphDataInvalid)
	{
		WeakGraph = nullptr;
		GraphPinReference = {};
		return;
	}

	const UEdGraphPin* Pin = GraphPinReference.Get();
	if (!Pin)
	{
		return;
	}

	if (WeakGraph == Pin->DefaultObject)
	{
		return;
	}

	if (Pin->DefaultObject &&
		!Pin->DefaultObject->IsA<UVoxelGraph>())
	{
		return;
	}

	WeakGraph = Cast<UVoxelGraph>(Pin->DefaultObject);
}

TSharedRef<SWidget>	SVoxelGraphPinParameter::GetDefaultValueWidget()
{
	return
		SNew(SBox)
		.MinDesiredWidth(18)
		.MaxDesiredWidth(400)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(TextContainer, SBox)
				.Visibility_Lambda([this]
				{
					if (!WeakGraph.IsValid_Slow())
					{
						return GetDefaultValueVisibility();
					}
					return EVisibility::Collapsed;
				})
				[
					SNew(SEditableTextBox)
					.Style(FAppStyle::Get(), "Graph.EditableTextBox")
					.Text_Lambda([this]
					{
						return FText::FromString(GraphPinObj->DefaultValue);
					})
					.SelectAllTextWhenFocused(true)
					.IsReadOnly_Lambda([this]() -> bool
					{
						return GraphPinObj->bDefaultValueIsReadOnly;
					})
					.OnTextCommitted_Lambda([this](const FText& NewValue, ETextCommit::Type)
					{
						if (!ensure(!GraphPinObj->IsPendingKill()))
						{
							return;
						}

						FString NewName = NewValue.ToString();
						if (NewName.IsEmpty())
						{
							NewName = "None";
						}

						const FVoxelTransaction Transaction(GraphPinObj, "Change Parameter Pin Value");
						GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, *NewName);
					})
					.ForegroundColor(FSlateColor::UseForeground())
				]
			]
			+ SOverlay::Slot()
			[
				SAssignNew(ParameterSelectorContainer, SBox)
				.Visibility_Lambda([this]
				{
					if (WeakGraph.IsValid_Slow())
					{
						return GetDefaultValueVisibility();
					}
					return EVisibility::Collapsed;
				})
				[
					SAssignNew(ParameterComboBox, SVoxelGraphParameterComboBox)
					.Items_Lambda(MakeWeakPtrLambda(this, [this]() -> TArray<FVoxelSelectorItem>
					{
						UVoxelGraph* Graph = WeakGraph.Resolve();
						if (!Graph)
						{
							return {};
						}

						const TSharedPtr<FVoxelGraphParametersView> ParametersView = Graph->GetParametersView();
						if (!ParametersView)
						{
							return {};
						}

						TArray<FVoxelSelectorItem> Items;
						for (const FVoxelParameterView* ParameterView : ParametersView->GetChildren())
						{
							const FVoxelParameter Parameter = ParameterView->GetParameter();
							Items.Add(FVoxelSelectorItem(ParameterView->Guid, Parameter.Name, Parameter.Type, Parameter.Category));
						}
						return Items;
					}))
					.CurrentItem_Lambda(MakeWeakPtrLambda(this, [this]() -> FVoxelSelectorItem
					{
						const FVoxelGraphBlueprintParameter& CurrentParameter = Cast<UK2Node_VoxelGraphParameterBase>(GraphPinObj->GetOwningNode())->CachedParameter;
						return FVoxelSelectorItem(CurrentParameter.Guid, CurrentParameter.Name, CurrentParameter.ExposedType);
					}))
					.IsValidItem_Lambda(MakeWeakPtrLambda(this, [this]
					{
						const FVoxelGraphBlueprintParameter& CurrentParameter = Cast<UK2Node_VoxelGraphParameterBase>(GraphPinObj->GetOwningNode())->CachedParameter;
						return CurrentParameter.bIsValid;
					}))
					.OnItemChanged(MakeWeakPtrDelegate(this, [this](const FVoxelSelectorItem& NewItem)
					{
						if (!ensure(!GraphPinObj->IsPendingKill()) ||
							!GraphPinObj->GetOwningNode())
						{
							return;
						}

						const FVoxelTransaction Transaction(GraphPinObj, "Change Parameter Pin Value");
						GraphPinObj->GetSchema()->TrySetDefaultValue(*GraphPinObj, NewItem.Guid.ToString());
					}))
				]
			]
		];
}