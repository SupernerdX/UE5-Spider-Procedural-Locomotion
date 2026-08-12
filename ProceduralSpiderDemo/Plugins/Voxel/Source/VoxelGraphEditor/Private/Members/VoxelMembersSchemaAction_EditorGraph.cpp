// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMembersSchemaAction_EditorGraph.h"
#include "VoxelGraph.h"
#include "VoxelGraphToolkit.h"
#include "VoxelTerminalGraph.h"
#include "Selection/VoxelGraphSelection.h"

UObject* FVoxelMembersSchemaAction_EditorGraph::GetOuter() const
{
	return WeakGraph.Resolve();
}

TSharedRef<SWidget> FVoxelMembersSchemaAction_EditorGraph::CreatePaletteWidget(const FCreateWidgetForActionData& CreateData) const
{
	return SNew(SVoxelMembersPaletteItem_EditorGraph, CreateData);
}

void FVoxelMembersSchemaAction_EditorGraph::BuildContextMenu(FMenuBuilder& MenuBuilder)
{
	const UVoxelGraph* Graph = WeakGraph.Resolve();
	if (!ensure(Graph) ||
		!Graph->GetBaseGraph_Unsafe())
	{
		return;
	}

	MenuBuilder.AddMenuEntry(
		INVTEXT("Go to parent"),
		INVTEXT("Go to the parent graph"),
		{},
		FUIAction
		{
			MakeLambdaDelegate([this]
			{
				const UVoxelGraph* LocalGraph = WeakGraph.Resolve();
				if (!ensure(LocalGraph))
				{
					return;
				}

				UVoxelGraph* BaseGraph = LocalGraph->GetBaseGraph_Unsafe();
				if (!BaseGraph)
				{
					return;
				}

				FVoxelUtilities::FocusObject(BaseGraph->GetEditorTerminalGraph());
			})
		});
}

void FVoxelMembersSchemaAction_EditorGraph::ApplyNewGuids(const TArray<FGuid>& NewGuids) const
{
	ensure(NewGuids.Num() == 1);
}

void FVoxelMembersSchemaAction_EditorGraph::OnActionDoubleClick() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return;
	}

	FVoxelUtilities::FocusObject(Toolkit->Asset->GetEditorTerminalGraph());
}

void FVoxelMembersSchemaAction_EditorGraph::OnSelected() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return;
	}

	Toolkit->GetSelection().SelectEditorGraph();
}

FString FVoxelMembersSchemaAction_EditorGraph::GetName() const
{
	return "Editor Graph";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelMembersPaletteItem_EditorGraph::Construct(const FArguments& InArgs, const FCreateWidgetForActionData& CreateData)
{
	ActionPtr = CreateData.Action;

	ChildSlot
	[
		SNew(SBox)
		.Padding(FMargin(0.f, -2.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.f, 2.f, 4.f, 2.f))
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("ClassIcon.EditorUtilityBlueprint"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f)
			[
				CreateTextSlotWidget(CreateData)
			]
		]
	];
}