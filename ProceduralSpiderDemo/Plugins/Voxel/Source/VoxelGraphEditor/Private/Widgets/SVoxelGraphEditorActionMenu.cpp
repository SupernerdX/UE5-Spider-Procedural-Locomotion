// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphEditorActionMenu.h"
#include "SGraphActionMenu.h"
#include "VoxelGraphToolkit.h"
#include "VoxelGraphVisuals.h"
#include "VoxelGraphSchemaAction.h"
#include "VoxelGraphContextActionsBuilder.h"
#include "Widgets/Notifications/SProgressBar.h"

void SVoxelGraphActionWidget::Construct(const FArguments& InArgs, const FCreateWidgetForActionData* InCreateData)
{
	ActionPtr = InCreateData->Action;
	MouseButtonDownDelegate = InCreateData->MouseButtonDownDelegate;

	FSlateIcon Icon("EditorStyle", "NoBrush");
	FLinearColor Color = FLinearColor::White;

	if (InCreateData->Action->GetTypeId() == FVoxelGraphSchemaAction::StaticGetTypeId())
	{
		static_cast<FVoxelGraphSchemaAction&>(*InCreateData->Action).GetIcon(Icon, Color);
	}

	FString ToolTip = InCreateData->Action->GetTooltipDescription().ToString();
	if (ToolTip != InCreateData->Action->GetMenuDescription().ToString())
	{
		TArray<FString> ToolTipLines;
		ToolTip.ParseIntoArrayLines(ToolTipLines, true);

		ToolTip = {};
		for (FString Line : ToolTipLines)
		{
			Line.TrimStartAndEndInline();
			ToolTip += Line + " ";
		}
	}
	else
	{
		ToolTip = {};
	}

	this->ChildSlot
	[
		SNew(SHorizontalBox)
		.ToolTipText(InCreateData->Action->GetTooltipDescription())
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(Icon.GetIcon())
			.ColorAndOpacity(Color)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(6.0f, 0.0f, 3.0f, 0.0f)
		[
			SNew(STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.Text(InCreateData->Action->GetMenuDescription())
			.HighlightText(InArgs._HighlightText)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(6.0f, 0.0f, 3.0f, 0.0f)
		[
			SNew(STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			.Text(FText::FromString(ToolTip))
			.HighlightText(InArgs._HighlightText)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		]
	];
}

FReply SVoxelGraphActionWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseButtonDownDelegate.Execute(ActionPtr))
	{
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelGraphEditorActionMenu::Construct(const FArguments& InArgs)
{
	WeakEdGraph = InArgs._GraphObj;
	DraggedFromPins = InArgs._DraggedFromPins;
	NewNodePosition = InArgs._NewNodePosition;
	OnClosedCallback = InArgs._OnClosedCallback;
	bAutoExpandActionMenu = InArgs._AutoExpandActionMenu;
	OnActionSelected = InArgs._OnActionSelected;
	bNoEdGraph = InArgs._NoEdGraph;

	ensure(bNoEdGraph || InArgs._GraphObj);

	FSlateColor TypeColor;
	FSlateIcon ContextIcon;
	if (DraggedFromPins.Num() == 1)
	{
		if (const UEdGraphPin* Pin = DraggedFromPins[0])
		{
			TypeColor = Pin->GetSchema()->GetPinTypeColor(Pin->PinType);
			ContextIcon = FVoxelGraphVisuals::GetPinIcon(Pin->PinType);
		}
	}

	SBorder::Construct(
		SBorder::FArguments()
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		.Padding(5.f)
		[
			SNew(SBox)
			.WidthOverride(400.f)
			.HeightOverride(400.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2.f, 2.f, 2.f, 5.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, ContextIcon.IsSet() ? 5.f : 0.f, 0.f)
					[
						SNew(SImage)
						.ColorAndOpacity(TypeColor)
						.Visibility(ContextIcon.IsSet() ? EVisibility::Visible : EVisibility::Collapsed)
						.Image(ContextIcon.GetIcon())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SVoxelGraphEditorActionMenu::GetSearchContextDesc)
						.Font(FAppStyle::GetFontStyle(FName("BlueprintEditor.ActionMenu.ContextDescriptionFont")))
						.ToolTipText(INVTEXT("Describes the current context of the action list"))
						.AutoWrapText(true)
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SCheckBox)
						.OnCheckStateChanged(this, &SVoxelGraphEditorActionMenu::OnGraphToggleChanged)
						.IsChecked(this, &SVoxelGraphEditorActionMenu::GraphToggleIsChecked)
						.ToolTipText(INVTEXT("Should the list be filtered to only actions that make sense in the current graph?"))
						[
							SNew(STextBlock)
							.Text(INVTEXT("Graph Sensitive"))
						]
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SAssignNew(GraphActionMenu, SGraphActionMenu)
					.OnActionSelected_Lambda([this](const TArray<TSharedPtr<FEdGraphSchemaAction>>& SelectedActions, const ESelectInfo::Type SelectionType)
					{
						if (SelectionType != ESelectInfo::OnMouseClick &&
							SelectionType != ESelectInfo::OnKeyPress)
						{
							return;
						}

						if (SelectedActions.Num() == 0)
						{
							return;
						}
						ensure(SelectedActions.Num() == 1);

						UEdGraph* EdGraph = WeakEdGraph.Resolve();
						for (const TSharedPtr<FEdGraphSchemaAction>& CurrentAction : SelectedActions)
						{
							if (!CurrentAction.IsValid())
							{
								continue;
							}

							OnActionSelected.ExecuteIfBound(CurrentAction, EdGraph, DraggedFromPins, NewNodePosition);
						}
					})
					.OnGetActionList(this, &SVoxelGraphEditorActionMenu::GetActionList)
					.AutoExpandActionMenu(bAutoExpandActionMenu)
					.DraggedFromPins(DraggedFromPins)
					.GraphObj(WeakEdGraph.Resolve())
					.OnCreateCustomRowExpander(this, &SVoxelGraphEditorActionMenu::CreateActionExpander)
					.OnCreateWidgetForAction(this, &SVoxelGraphEditorActionMenu::OnCreateWidgetForAction)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(2.f)
					.Visibility_Lambda([this]
					{
						return
							ActionsBuilder &&
							!ActionsBuilder->IsCompleted()
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
					[
						SNew(SProgressBar)
						.BorderPadding(FVector2D::ZeroVector)
						.Percent_Lambda([this]()
						{
							return ActionsBuilder ? ActionsBuilder->GetProgress() : 0.0f;
						})
					]
				]
			]
		]
	);
}

void SVoxelGraphEditorActionMenu::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SBorder::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (ActionsBuilder)
	{
		const int32 NewIdxStart = RootActionList->GetNumActions();
		if (ActionsBuilder->Build())
		{
			GraphActionMenu->UpdateForNewActions(NewIdxStart);
		}
	}
}

SVoxelGraphEditorActionMenu::~SVoxelGraphEditorActionMenu()
{
	OnClosedCallback.ExecuteIfBound();
}

TSharedRef<SEditableTextBox> SVoxelGraphEditorActionMenu::GetFilterTextBox() const
{
	return GraphActionMenu->GetFilterTextBox();
}

TSharedRef<FGraphActionListBuilderBase> SVoxelGraphEditorActionMenu::GetActionList()
{
	RootActionList = MakeShared<FGraphContextMenuBuilder>(WeakEdGraph.Resolve());
	if (DraggedFromPins.Num() != 0)
	{
		RootActionList->FromPin = DraggedFromPins[0];
	}

	if (bNoEdGraph)
	{
		ActionsBuilder = FVoxelGraphContextActionsBuilder::BuildNoGraph(RootActionList);
	}
	else
	{
		ActionsBuilder = FVoxelGraphContextActionsBuilder::Build(RootActionList);
	}

	return RootActionList.ToSharedRef();
}

TSharedRef<SExpanderArrow> SVoxelGraphEditorActionMenu::CreateActionExpander(const FCustomExpanderData& ActionMenuData) const
{
	return SNew(SExpanderArrow, ActionMenuData.TableRow);
}

TSharedRef<SWidget> SVoxelGraphEditorActionMenu::OnCreateWidgetForAction(FCreateWidgetForActionData* CreateData) const
{
	return SNew(SVoxelGraphActionWidget, CreateData);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FText SVoxelGraphEditorActionMenu::GetSearchContextDesc() const
{
	if (bNoEdGraph)
	{
		return INVTEXT("All Possible Actions");
	}

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(WeakEdGraph.Resolve());
	if (!Toolkit)
	{
		return INVTEXT("All Possible Actions");
	}

	FString Description;
	if (DraggedFromPins.Num() == 1)
	{
		const UEdGraphPin* Pin = DraggedFromPins[0];
		if (ensure(Pin))
		{
			Description = "\n" + FString(Pin->Direction == EGPD_Input ? "providing" : "taking");
			Description += " a(n) " + FVoxelPinType(Pin->PinType).ToString();
		}
	}
	else if (DraggedFromPins.Num() > 1)
	{
		Description = "\nfor " + LexToString(DraggedFromPins.Num()) + " pins";
	}

	if (Toolkit->IsGraphSensitive())
	{
		FString TypeName = Toolkit->Asset->GetClass()->GetDisplayNameText().ToString();
		TypeName.RemoveFromStart("Voxel ");

		Description = "All " + TypeName + " Actions " + Description;
	}
	else
	{
		Description = "All Actions " + Description;
	}
	
	return FText::FromString(Description);
}

void SVoxelGraphEditorActionMenu::OnGraphToggleChanged(const ECheckBoxState NewState) const
{
	if (bNoEdGraph)
	{
		return;
	}

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(WeakEdGraph.Resolve());
	if (!Toolkit)
	{
		return;
	}

	Toolkit->SetIsGraphSensitive(NewState == ECheckBoxState::Checked);
	GraphActionMenu->RefreshAllActions(true, false);
}

ECheckBoxState SVoxelGraphEditorActionMenu::GraphToggleIsChecked() const
{
	if (bNoEdGraph)
	{
		return ECheckBoxState::Unchecked;
	}

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(WeakEdGraph.Resolve());
	if (!Toolkit)
	{
		return ECheckBoxState::Unchecked;
	}

	return Toolkit->IsGraphSensitive() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}