// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMembersSchemaAction_LibraryFunction.h"
#include "VoxelGraph.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphSearchManager.h"

TSharedRef<SWidget> FVoxelMembersSchemaAction_LibraryFunction::CreatePaletteWidget(const FCreateWidgetForActionData& CreateData) const
{
	return SNew(SVoxelMembersPaletteItem_LibraryFunction, CreateData);
}

void FVoxelMembersSchemaAction_LibraryFunction::OnDelete() const
{
	FVoxelMembersSchemaAction_Function::OnDelete();

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = WeakToolkit.Pin();
	if (!Toolkit)
	{
		return;
	}

	// Opens an empty tab if there's no function left
	for (const FGuid& Guid : Toolkit->Asset->GetTerminalGraphs())
	{
		UVoxelTerminalGraph& TerminalGraph = Toolkit->Asset->FindTerminalGraphChecked(Guid);
		if (TerminalGraph.IsMainTerminalGraph() ||
			TerminalGraph.IsEditorTerminalGraph())
		{
			continue;
		}

		return;
	}

	Toolkit->OpenEmptyTab();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelMembersSchemaAction_LibraryFunction::IsFunctionExposed() const
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return false;
	}

	return TerminalGraph->bExposeToLibrary;
}

void FVoxelMembersSchemaAction_LibraryFunction::ToggleFunctionExposure() const
{
	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return;
	}

	const FVoxelTransaction Transaction(TerminalGraph, "Toggle function visibility");
	TerminalGraph->bExposeToLibrary = !TerminalGraph->bExposeToLibrary;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelMembersPaletteItem_LibraryFunction::Construct(const FArguments& InArgs, const FCreateWidgetForActionData& CreateData)
{
	ActionPtr = CreateData.Action;

	ChildSlot
	[
		SNew(SBox)
		.Padding(0.f, -2.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(0.f, 2.f, 4.f, 2.f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("GraphEditor.Function_16x")))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f)
			[
				CreateTextSlotWidget(CreateData)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f)
			[
				CreateVisibilityWidget()
			]
		]
	];
}

TSharedRef<SWidget> SVoxelMembersPaletteItem_LibraryFunction::CreateVisibilityWidget() const
{
	TSharedRef<SWidget> Button = PropertyCustomizationHelpers::MakeVisibilityButton(
		FOnClicked::CreateLambda([this]() -> FReply
		{
			const TSharedPtr<FVoxelMembersSchemaAction_LibraryFunction> Action = GetAction();
			if (!ensure(Action))
			{
				return FReply::Handled();
			}

			Action->ToggleFunctionExposure();
			return FReply::Handled();
		}),
		{},
		MakeAttributeLambda([this]
		{
			const TSharedPtr<FVoxelMembersSchemaAction_LibraryFunction> Action = GetAction();
			if (!ensure(Action))
			{
				return false;
			}

			return Action->IsFunctionExposed();
		}));

	Button->SetToolTipText(INVTEXT("Expose Macro"));

	return Button;
}