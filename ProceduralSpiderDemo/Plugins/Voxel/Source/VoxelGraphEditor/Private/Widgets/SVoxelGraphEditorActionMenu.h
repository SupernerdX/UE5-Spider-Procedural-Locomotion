// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "SGraphActionMenu.h"

class FVoxelGraphContextActionsBuilder;
struct FCustomExpanderData;
struct FEdGraphSchemaAction;
struct FCreateWidgetForActionData;
struct FGraphActionListBuilderBase;

class SVoxelGraphActionWidget : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ATTRIBUTE(FText, HighlightText)
	};

	void Construct(const FArguments& InArgs, const FCreateWidgetForActionData* InCreateData);

	//~ Begin SCompoundWidget Interface
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	//~ End SCompoundWidget Interface

private:
	TWeakPtr<FEdGraphSchemaAction> ActionPtr;
	FCreateWidgetMouseButtonDown MouseButtonDownDelegate;
};

class SVoxelGraphEditorActionMenu : public SBorder
{
public:
	VOXEL_SLATE_ARGS()
	{
		FArguments()
			: _GraphObj(nullptr)
			, _NewNodePosition()
			, _AutoExpandActionMenu(false)
			, _NoEdGraph(false)
		{
		}

		SLATE_ARGUMENT(UEdGraph*, GraphObj)
		SLATE_ARGUMENT(UE_506_SWITCH(FVector2D, FVector2f), NewNodePosition)
		SLATE_ARGUMENT(TArray<UEdGraphPin*>, DraggedFromPins)
		SLATE_ARGUMENT(SGraphEditor::FActionMenuClosed, OnClosedCallback)
		SLATE_ARGUMENT(bool, AutoExpandActionMenu)
		SLATE_ARGUMENT(bool, NoEdGraph)
		SLATE_EVENT(TDelegate<void(const TSharedPtr<FEdGraphSchemaAction>&, UEdGraph*, TArray<UEdGraphPin*>&, const UE_506_SWITCH(FVector2D, FVector2f)&)>, OnActionSelected)
	};

	void Construct(const FArguments& InArgs);

	//~ Begin SBorder Interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual ~SVoxelGraphEditorActionMenu() override;
	//~ End SBorder Interface

	TSharedRef<SEditableTextBox> GetFilterTextBox() const;

private:
	TSharedRef<FGraphActionListBuilderBase> GetActionList();
	TSharedRef<SExpanderArrow> CreateActionExpander(const FCustomExpanderData& ActionMenuData) const;
	TSharedRef<SWidget> OnCreateWidgetForAction(FCreateWidgetForActionData* CreateData) const;

private:
	FText GetSearchContextDesc() const;
	void OnGraphToggleChanged(ECheckBoxState NewState) const;
	ECheckBoxState GraphToggleIsChecked() const;

protected:
	TVoxelObjectPtr<UEdGraph> WeakEdGraph = nullptr;
	TArray<UEdGraphPin*> DraggedFromPins;
	UE_506_SWITCH(FVector2D, FVector2f) NewNodePosition = UE_506_SWITCH(FVector2D, FVector2f)::ZeroVector;
	bool bAutoExpandActionMenu = false;
	bool bNoEdGraph = false;

	TSharedPtr<FVoxelGraphContextActionsBuilder> ActionsBuilder;
	SGraphEditor::FActionMenuClosed OnClosedCallback;

	TSharedPtr<SGraphActionMenu> GraphActionMenu;
	TSharedPtr<FGraphContextMenuBuilder> RootActionList;

	TDelegate<void(const TSharedPtr<FEdGraphSchemaAction>&, UEdGraph*, TArray<UEdGraphPin*>&, const UE_506_SWITCH(FVector2D, FVector2f)&)> OnActionSelected;
};