// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "SGraphActionMenu.h"
#include "VoxelNodeLibrary.h"
#include "VoxelGraphSchemaAction.h"
#include "VoxelGraphEditorSettings.h"
#include "VoxelFunctionLibraryAsset.h"
#include "Nodes/VoxelNode_UFunction.h"
#include "Widgets/SVoxelGraphEditorActionMenu.h"
#include "Widgets/SVoxelInputBindingEditBox.h"

class FVoxelGraphInputBindingCustomization : public IPropertyTypeCustomization
{
public:
	virtual void CustomizeHeader(
		const TSharedRef<IPropertyHandle> PropertyHandle,
		FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		BaseHandle = PropertyHandle;

		HeaderRow
		.NameContent()
		[
			SNew(SVoxelInputBindingEditBox)
			.InputChord_Lambda([this]
			{
				const TSharedRef<IPropertyHandle> InputChordHandle = BaseHandle->GetChildHandleStatic(FVoxelGraphEditorInputBinding, InputChord);
				return FVoxelEditorUtilities::GetStructPropertyValue<FInputChord>(InputChordHandle);
			})
			.OnUpdateInputChord_Lambda([this](const FInputChord& NewInputChord)
			{
				const int32 ArrayIndex = BaseHandle->GetIndexInArray();

				UVoxelGraphEditorSettings* Settings = GetMutableDefault<UVoxelGraphEditorSettings>();
				if (INLINE_LAMBDA
					{
						for (int32 Index = 0; Index < Settings->Hotkeys.Num(); Index++)
						{
							if (Index != ArrayIndex &&
								Settings->Hotkeys[Index].InputChord == NewInputChord)
							{
								return true;
							}
						}
						return false;
					})
				{
					FVoxelTransaction Transaction(Settings, "Remove conflicting chords");
					Transaction.SetProperty(FindFPropertyChecked(UVoxelGraphEditorSettings, Hotkeys));
					for (int32 Index = 0; Index < Settings->Hotkeys.Num(); Index++)
					{
						FVoxelGraphEditorInputBinding& Input = Settings->Hotkeys[Index];
						if (Index != ArrayIndex &&
							Input.InputChord == NewInputChord)
						{
							Input.InputChord = {};
						}
					}
				}

				const TSharedRef<IPropertyHandle> InputChordHandle = BaseHandle->GetChildHandleStatic(FVoxelGraphEditorInputBinding, InputChord);
				FVoxelEditorUtilities::SetStructPropertyValue(InputChordHandle, NewInputChord);
			})
			.OnGetChordConflictMessage_Lambda([this](const FInputChord& NewInputChord) -> FText
			{
				const int32 InputChordIndex = BaseHandle->GetIndexInArray();
				const UVoxelGraphEditorSettings* Settings = GetDefault<UVoxelGraphEditorSettings>();
				for (int32 Index = 0; Index < Settings->Hotkeys.Num(); Index++)
				{
					const FVoxelGraphEditorInputBinding& Input = Settings->Hotkeys[Index];
					if (Index != InputChordIndex &&
						Input.InputChord == NewInputChord)
					{
						return FText::FromString(NewInputChord.GetInputText().ToString() + " is already bound to " + Input.GetActionName());
					}
				}

				return {};
			})
		]
		.ValueContent()
		[
			SNew(SBox)
			.MinDesiredWidth(125.f)
			[
				SAssignNew(SelectedComboButton, SComboButton)
				.ComboButtonStyle(FAppStyle::Get(), "ComboButton")
				.OnGetMenuContent(this, &FVoxelGraphInputBindingCustomization::GetMenuContent)
				.ContentPadding(0)
				.ForegroundColor(FSlateColor::UseForeground())
				.ButtonContent()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image_Lambda([this]() -> const FSlateBrush*
						{
							FSlateIcon Icon;
							FLinearColor Color;
							GetActionColorIcon(Icon, Color);
							return Icon.GetIcon();
						})
						.ColorAndOpacity_Lambda([this]() -> FSlateColor
						{
							FSlateIcon Icon;
							FLinearColor Color;
							GetActionColorIcon(Icon, Color);
							return Color;
						})
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(6.0f, 0.0f, 3.0f, 0.0f)
					[
						SNew(STextBlock)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
						.Text_Lambda([this]
						{
							const FVoxelGraphEditorInputBinding& InputBinding = FVoxelEditorUtilities::GetStructPropertyValue<FVoxelGraphEditorInputBinding>(BaseHandle);
							return FText::FromString(InputBinding.GetActionName());
						})
						.ColorAndOpacity_Lambda([this]
						{
							const FVoxelGraphEditorInputBinding& InputBinding = FVoxelEditorUtilities::GetStructPropertyValue<FVoxelGraphEditorInputBinding>(BaseHandle);
							return InputBinding.IsActionValid(true) ? FSlateColor::UseForeground() : FStyleColors::Error;
						})
					]
				]
			]
		];
	}

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
	}

private:
	TSharedRef<SWidget> GetMenuContent()
	{
		return
			SAssignNew(MenuContent, SMenuOwner)
			[
				SNew(SBorder)
				.BorderImage( FAppStyle::GetBrush("Menu.Background") )
				.Padding(5)
				[
					// Achieving fixed width by nesting items within a fixed width box.
					SNew(SBox)
					.WidthOverride(400)
					.HeightOverride(400)
					[
						SNew(SVoxelGraphEditorActionMenu)
						.OnActionSelected(this, &FVoxelGraphInputBindingCustomization::OnActionSelected)
						.NoEdGraph(true)
					]
				]
			];
	}

	void OnActionSelected(const TSharedPtr<FEdGraphSchemaAction>& Action, UEdGraph* Graph, TArray<UEdGraphPin*>& DraggedFromPins, const UE_506_SWITCH(FVector2D, FVector2f)& NewNodePosition) const
	{
		FSlateApplication::Get().DismissAllMenus();

		if (!ensure(Action->GetTypeId() == FVoxelGraphSchemaAction::StaticGetTypeId()))
		{
			return;
		}

		FVoxelGraphEditorInputBinding InputBinding = FVoxelEditorUtilities::GetStructPropertyValue<FVoxelGraphEditorInputBinding>(BaseHandle);

		const TSharedPtr<FVoxelGraphSchemaAction> VoxelAction = StaticCastSharedPtr<FVoxelGraphSchemaAction>(Action);
		if (VoxelAction->GetVoxelTypeId() == FVoxelGraphSchemaAction_NewStructNode::StaticGetVoxelTypeId())
		{
			const TSharedPtr<FVoxelGraphSchemaAction_NewStructNode> StructAction = StaticCastSharedPtr<FVoxelGraphSchemaAction_NewStructNode>(Action);
			if (!StructAction->Node)
			{
				return;
			}

			if (const FVoxelNode_UFunction* FunctionNode = StructAction->Node->As<FVoxelNode_UFunction>())
			{
				InputBinding.Type = EVoxelGraphEditorInputType::Function;
				InputBinding.Struct = nullptr;
				InputBinding.Function = FunctionNode->GetFunction();
			}
			else
			{
				InputBinding.Type = EVoxelGraphEditorInputType::Struct;
				InputBinding.Struct = StructAction->Node->GetStruct();
				InputBinding.Function = nullptr;
			}
			InputBinding.FunctionLibrary = nullptr;
			InputBinding.FunctionGuid = {};
		}
		else if (VoxelAction->GetVoxelTypeId() == FVoxelGraphSchemaAction_NewCallExternalFunctionNode::StaticGetVoxelTypeId())
		{
			const TSharedPtr<FVoxelGraphSchemaAction_NewCallExternalFunctionNode> ExternalFunctionAction = StaticCastSharedPtr<FVoxelGraphSchemaAction_NewCallExternalFunctionNode>(Action);
			InputBinding.Type = EVoxelGraphEditorInputType::FunctionLibrary;
			InputBinding.Struct = nullptr;
			InputBinding.Function = nullptr;
			InputBinding.FunctionLibrary = ExternalFunctionAction->FunctionLibrary.Resolve();
			InputBinding.FunctionGuid = ExternalFunctionAction->Guid;
		}
		else
		{
			ensure(false);
			return;
		}

		FVoxelEditorUtilities::SetStructPropertyValue(BaseHandle, InputBinding);
	}

private:
	void GetActionColorIcon(FSlateIcon& OutIcon, FLinearColor& OutColor)
	{
		const FVoxelGraphEditorInputBinding& InputBinding = FVoxelEditorUtilities::GetStructPropertyValue<FVoxelGraphEditorInputBinding>(BaseHandle);
		switch (InputBinding.Type)
		{
		case EVoxelGraphEditorInputType::Struct:
		{
			if (InputBinding.Struct.IsNull())
			{
				break;
			}

			const TSharedPtr<const FVoxelNode> Node = GVoxelNodeLibrary->FindNode(InputBinding.Struct.LoadSynchronous());
			StructNodeAction.Node = Node;

			if (!StructNodeAction.Node)
			{
				break;
			}

			StructNodeAction.GetIcon(OutIcon, OutColor);
			break;
		}
		case EVoxelGraphEditorInputType::Function:
		{
			if (InputBinding.Function.IsNull())
			{
				break;
			}

			const TSharedPtr<const FVoxelNode> Node = GVoxelNodeLibrary->FindNode(InputBinding.Function.LoadSynchronous());
			StructNodeAction.Node = Node;

			if (!StructNodeAction.Node)
			{
				break;
			}

			StructNodeAction.GetIcon(OutIcon, OutColor);
			break;
		}
		case EVoxelGraphEditorInputType::FunctionLibrary:
		{
			static const FSlateIcon FunctionIcon = FSlateIcon("EditorStyle", "GraphEditor.Function_16x");
			OutColor = FLinearColor::White;
			OutIcon = FunctionIcon;
			break;
		}
		case EVoxelGraphEditorInputType::None: break;
		default: ensure(false); break;
		}
	}

private:
	TSharedPtr<IPropertyHandle> BaseHandle;
	TSharedPtr<SMenuOwner> MenuContent;
	TSharedPtr<SComboButton> SelectedComboButton;

	FVoxelGraphSchemaAction_NewStructNode StructNodeAction;
};

DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelGraphEditorInputBinding, FVoxelGraphInputBindingCustomization);