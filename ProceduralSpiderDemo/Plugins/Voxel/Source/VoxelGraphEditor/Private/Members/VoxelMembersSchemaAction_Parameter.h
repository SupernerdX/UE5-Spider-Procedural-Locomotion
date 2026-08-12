// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Members/VoxelMembersDragDropAction_Base.h"
#include "VoxelMembersSchemaAction_VariableBase.h"

class UVoxelGraph;
struct FVoxelGraphToolkit;

struct FVoxelMembersSchemaAction_Parameter : public FVoxelMembersSchemaAction_VariableBase
{
public:
	TVoxelObjectPtr<UVoxelGraph> WeakGraph;

	using FVoxelMembersSchemaAction_VariableBase::FVoxelMembersSchemaAction_VariableBase;

	static void OnPaste(
		const FVoxelGraphToolkit& Toolkit,
		const FString& Text,
		const FString& Category);

	//~ Begin FVoxelMembersSchemaAction_Base Interface
	virtual UObject* GetOuter() const override;
	virtual void ApplyNewGuids(const TArray<FGuid>& NewGuids) const override;
	virtual FReply OnDragged(const FPointerEvent& MouseEvent) override;
	virtual void BuildContextMenu(FMenuBuilder& MenuBuilder) override;
	virtual void OnActionDoubleClick() const override;
	virtual void OnSelected() const override;
	virtual void OnDelete() const override;
	virtual void OnDuplicate() const override;
	virtual bool OnCopy(FString& OutText) const override;
	virtual FString GetName() const override;
	virtual void SetName(const FString& Name) const override;
	virtual void SetCategory(const FString& NewCategory) const override;
	virtual FString GetSearchString() const override;
	//~ End FVoxelMembersSchemaAction_Base Interface

	//~ Begin FVoxelMembersSchemaAction_VariableBase Interface
	virtual FVoxelPinType GetPinType() const override;
	virtual void SetPinType(const FVoxelPinType& NewPinType) const override;
	//~ End FVoxelMembersSchemaAction_VariableBase Interface
};

class FVoxelMembersDragDropAction_Parameter : public FVoxelMembersDragDropAction_Base
{
public:
	DRAG_DROP_OPERATOR_TYPE(FVoxelMembersDragDropAction_Parameter, FVoxelMembersDragDropAction_Base);

	const TVoxelObjectPtr<UVoxelGraph> WeakGraph;
	const FGuid ParameterGuid;
	const bool bAltDrag;
	const bool bControlDrag;

	static TSharedRef<FVoxelMembersDragDropAction_Parameter> New(
		const TSharedRef<FVoxelMembersSchemaAction_Base>& Action,
		const TVoxelObjectPtr<UVoxelGraph>& WeakGraph,
		const FGuid& ParameterGuid,
		const FPointerEvent& MouseEvent)
	{
		const TSharedRef<FVoxelMembersDragDropAction_Parameter> Operation = MakeShareable(new FVoxelMembersDragDropAction_Parameter(
			WeakGraph,
			ParameterGuid,
			MouseEvent.IsAltDown(),
			MouseEvent.IsControlDown()));
		Operation->Construct(Action);
		return Operation;
	}

	//~ Begin FGraphSchemaActionDragDropAction Interface
	virtual void HoverTargetChanged() override;

	virtual FReply DroppedOnPin(
		UE_506_SWITCH(FVector2D, const FVector2f&) ScreenPosition,
		UE_506_SWITCH(FVector2D, const FVector2f&) GraphPosition) override;
	virtual FReply DroppedOnNode(
		UE_506_SWITCH(FVector2D, const FVector2f&) ScreenPosition,
		UE_506_SWITCH(FVector2D, const FVector2f&) GraphPosition) override;
	virtual FReply DroppedOnPanel(
		const TSharedRef<SWidget>& Panel,
		UE_506_SWITCH(FVector2D, const FVector2f&) ScreenPosition,
		UE_506_SWITCH(FVector2D, const FVector2f&) GraphPosition,
		UEdGraph& EdGraph) override;

	virtual void GetDefaultStatusSymbol(
		const FSlateBrush*& PrimaryBrushOut,
		FSlateColor& IconColorOut,
		FSlateBrush const*& SecondaryBrushOut,
		FSlateColor& SecondaryColorOut) const override;
	//~ End FGraphSchemaActionDragDropAction Interface

private:
	FVoxelMembersDragDropAction_Parameter(
		const TVoxelObjectPtr<UVoxelGraph>& WeakGraph,
		const FGuid& ParameterGuid,
		const bool bAltDrag,
		const bool bControlDrag)
		: WeakGraph(WeakGraph)
		, ParameterGuid(ParameterGuid)
		, bAltDrag(bAltDrag)
		, bControlDrag(bControlDrag)
	{
	}
};