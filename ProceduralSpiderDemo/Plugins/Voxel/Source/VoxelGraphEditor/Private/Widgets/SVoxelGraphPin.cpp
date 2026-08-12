// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphPin.h"
#include "VoxelEdGraph.h"
#include "SVoxelPinLODWidget.h"
#include "Nodes/VoxelGraphNode.h"

VOXEL_INITIALIZE_STYLE(GraphPinEditor)
{
	Set("Debug.Pin.Background", new CORE_IMAGE_BRUSH("Graph/RegularNode_heat_display", CoreStyleConstants::Icon24x24));
}

DEFINE_PRIVATE_ACCESS(SGraphPin, FullPinHorizontalRowWidget)
DEFINE_PRIVATE_ACCESS(SGraphPin, CachedImg_Pin_DiffOutline)
DEFINE_PRIVATE_ACCESS(SGraphPin, PinNameLODBranchNode)

DEFINE_PRIVATE_ACCESS_FUNCTION(SGraphPin, GetPinColor)

DEFINE_PRIVATE_ACCESS_FUNCTION(SGraphPin, GetPinLabelVisibility)
DEFINE_PRIVATE_ACCESS_FUNCTION(SGraphPin, GetPinLabel)
DEFINE_PRIVATE_ACCESS_FUNCTION(SGraphPin, GetPinTextColor)

DEFINE_PRIVATE_ACCESS_FUNCTION(SGraphPin, GetPinBorder)
DEFINE_PRIVATE_ACCESS_FUNCTION(SGraphPin, GetHighlightColor)
DEFINE_PRIVATE_ACCESS_FUNCTION(SGraphPin, GetPinDiffColor)
DEFINE_PRIVATE_ACCESS_FUNCTION(SGraphPin, UseLowDetailPinNames)

TSharedRef<SWidget> SVoxelGraphPin::InitializeEditableLabel(
	const TSharedPtr<SGraphPin>& PinWidget,
	const TSharedRef<SWidget>& DefaultWidget)
{
	if (!PinWidget->IsEditingEnabled())
	{
		return DefaultWidget;
	}

	const UEdGraphPin* PinObject = PinWidget->GetPinObj();
	if (!ensure(PinObject))
	{
		return DefaultWidget;
	}

	UVoxelGraphNode* OwnerNode = Cast<UVoxelGraphNode>(PinObject->GetOwningNode());
	if (!OwnerNode)
	{
		return DefaultWidget;
	}

	if (!OwnerNode->CanRenamePin(*PinObject))
	{
		return DefaultWidget;
	}

	TSharedRef<SInlineEditableTextBlock> EditableTextBoxWidget =
		SNew(SInlineEditableTextBlock)
		.Style(&FAppStyle::Get().GetWidgetStyle<FInlineEditableTextBlockStyle>("Graph.Node.InlineEditablePinName"))
		.Visibility_Lambda([WeakWidget = MakeWeakPtr(PinWidget)]() -> EVisibility
		{
			const TSharedPtr<SGraphPin> Widget = WeakWidget.Pin();
			if (!Widget)
			{
				return EVisibility::Collapsed;
			}

			return PrivateAccess::GetPinLabelVisibility(*Widget)();
		})
		.Text_Lambda([WeakWidget = MakeWeakPtr(PinWidget)]() -> FText
		{
			const TSharedPtr<SGraphPin> Widget = WeakWidget.Pin();
			if (!Widget)
			{
				return {};
			}

			return PrivateAccess::GetPinLabel(*Widget)();
		})
		.ColorAndOpacity_Lambda([WeakWidget = MakeWeakPtr(PinWidget)]() -> FSlateColor
		{
			const TSharedPtr<SGraphPin> Widget = WeakWidget.Pin();
			if (!Widget)
			{
				return {};
			}

			return PrivateAccess::GetPinTextColor(*Widget)();
		})
		.OnVerifyTextChanged(MakeWeakPtrDelegate(PinWidget, [&PinWidget = *PinWidget](const FText& InNewName, FText& OutErrorMessage)
		{
			UEdGraphPin* EdGraphPin = PinWidget.GetPinObj();
			if (!ensure(EdGraphPin))
			{
				return false;
			}

			const UVoxelGraphNode* Node = Cast<UVoxelGraphNode>(EdGraphPin->GetOwningNode());
			if (!ensure(Node))
			{
				return false;
			}

			return Node->IsNewPinNameValid(*EdGraphPin, FName(InNewName.ToString()));
		}))
		.OnTextCommitted(MakeWeakPtrDelegate(PinWidget, [&PinWidget = *PinWidget](const FText& InNewName, ETextCommit::Type CommitType)
		{
			UEdGraphPin* EdGraphPin = PinWidget.GetPinObj();
			if (!ensure(EdGraphPin))
			{
				return;
			}

			UVoxelGraphNode* Node = Cast<UVoxelGraphNode>(EdGraphPin->GetOwningNode());
			if (!ensure(Node))
			{
				return;
			}

			const FString NewPinName = InNewName.ToString().TrimStartAndEnd();
			if (NewPinName.IsEmpty())
			{
				return;
			}

			Node->RenamePin(*EdGraphPin, FName(NewPinName));
		}));

	OwnerNode->AddPinToRename(PinObject->PinName, MakeWeakPtrDelegate(EditableTextBoxWidget, [&EditableTextBoxWidget = *EditableTextBoxWidget]
	{
		EditableTextBoxWidget.EnterEditingMode();
	}));

	if (OwnerNode->ShouldPromptRenameOnSpawn(*PinObject))
	{
		FVoxelUtilities::DelayedCall(MakeWeakPtrLambda(EditableTextBoxWidget, [&EditableTextBoxWidget = *EditableTextBoxWidget]
		{
			EditableTextBoxWidget.EnterEditingMode();
		}));
	}

	return EditableTextBoxWidget;
}

void SVoxelGraphPin::ConstructDebugHighlight(SGraphPin* PinWidget)
{
	const SGraphPin::FArguments InArgs;

	const bool bIsInput = PinWidget->GetDirection() == EGPD_Input;
	TSharedPtr<SHorizontalBox> PinContent;
	if (bIsInput)
	{
		PrivateAccess::FullPinHorizontalRowWidget(*PinWidget) = PinContent = 
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, InArgs._SideToSideMargin, 0)
			[
				PinWidget->GetPinImageWidget().ToSharedRef()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				PinWidget->GetLabelAndValue()
			];
	}
	else
	{
		PrivateAccess::FullPinHorizontalRowWidget(*PinWidget) = PinContent = SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				PinWidget->GetLabelAndValue()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(InArgs._SideToSideMargin, 0, 0, 0)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.Padding(-4.f)
				[
					SNew(SImage)
					.Visibility_Lambda([WeakWidget = MakeWeakPtr(PinWidget)]() -> EVisibility
					{
						const TSharedPtr<SGraphPin> Widget = WeakWidget.Pin();
						if (!Widget)
						{
							return EVisibility::Collapsed;
						}

						const UEdGraphPin* EdPin = Widget->GetPinObj();
						if (!EdPin)
						{
							return EVisibility::Collapsed;
						}

						const UVoxelGraphNode* VoxelNode = Cast<UVoxelGraphNode>(EdPin->GetOwningNodeUnchecked());
						if (!VoxelNode)
						{
							return EVisibility::Collapsed;
						}

						const UVoxelEdGraph* Graph = Cast<UVoxelEdGraph>(VoxelNode->GetGraph());
						if (!Graph)
						{
							return EVisibility::Collapsed;
						}

						if (Graph->WeakDebugNode != VoxelNode ||
							VoxelNode->PreviewedPin != EdPin->PinName)
						{
							return EVisibility::Collapsed;
						}

						return EVisibility::Visible;
					})
					.Image(FVoxelEditorStyle::GetBrush("Debug.Pin.Background"))
					.ColorAndOpacity_Lambda([WeakWidget = MakeWeakPtr(PinWidget)]() -> FSlateColor
					{
						const TSharedPtr<SGraphPin> Widget = WeakWidget.Pin();
						if (!Widget)
						{
							return FLinearColor::Transparent;
						}

						FLinearColor Color = PrivateAccess::GetPinColor(*Widget)().GetSpecifiedColor();
						Color.A = 0.6f;
						return Color;
					})
					.DesiredSizeOverride(FVector2D(16.f))
				]
				+ SOverlay::Slot()
				[
					PinWidget->GetPinImageWidget().ToSharedRef()
				]
			];
	}

	TSharedPtr<SVoxelPinLODWidget> LODWidget;
	PinWidget->SBorder::Construct(SBorder::FArguments()
		.BorderImage_Lambda([WeakWidget = MakeWeakPtr(PinWidget)]() -> const FSlateBrush*
		{
			const TSharedPtr<SGraphPin> Widget = WeakWidget.Pin();
			if (!Widget)
			{
				return nullptr;
			}

			return PrivateAccess::GetPinBorder(*Widget)();
		})
		.BorderBackgroundColor_Lambda([WeakWidget = MakeWeakPtr(PinWidget)]() -> FSlateColor
		{
			const TSharedPtr<SGraphPin> Widget = WeakWidget.Pin();
			if (!Widget)
			{
				return {};
			}

			return PrivateAccess::GetHighlightColor(*Widget)();
		})
		.OnMouseButtonDown(PinWidget, &SGraphPin::OnPinNameMouseDown)
		[
			SNew(SBorder)
			.BorderImage(PrivateAccess::CachedImg_Pin_DiffOutline(*PinWidget))
			.BorderBackgroundColor_Lambda([WeakWidget = MakeWeakPtr(PinWidget)]() -> FSlateColor
			{
				const TSharedPtr<SGraphPin> Widget = WeakWidget.Pin();
				if (!Widget)
				{
					return {};
				}

				return PrivateAccess::GetPinDiffColor(*Widget)();
			})
			[
				SAssignNew(LODWidget, SVoxelPinLODWidget)
				.UseLowDetailSlot_Lambda([WeakWidget = MakeWeakPtr(PinWidget)]() -> bool
				{
					const TSharedPtr<SGraphPin> Widget = WeakWidget.Pin();
					if (!Widget)
					{
						return true;
					}

					return PrivateAccess::UseLowDetailPinNames(*Widget)();
				})
				.LowDetail()
				[
					SNew(SBox)
					.HAlign(bIsInput ? HAlign_Left : HAlign_Right)
					.VAlign(VAlign_Center)
					[
						PinWidget->GetPinImageWidget().ToSharedRef()
					]
				]
				.HighDetail()
				[
					PinContent.ToSharedRef()
				]
			]
		]
	);

	PrivateAccess::PinNameLODBranchNode(*PinWidget) = LODWidget;
}