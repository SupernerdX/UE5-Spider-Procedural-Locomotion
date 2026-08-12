// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMembersSchemaAction_FunctionInput.h"
#include "SVoxelGraphMembers.h"
#include "VoxelExposedSeed.h"
#include "VoxelGraphToolkit.h"
#include "VoxelGraphVisuals.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphSchemaAction.h"
#include "Selection/VoxelGraphSelection.h"
#include "Nodes/VoxelGraphNode_FunctionInput.h"

void FVoxelMembersSchemaAction_FunctionInput::OnPaste(
	const FVoxelGraphToolkit& Toolkit,
	UVoxelTerminalGraph& TerminalGraph,
	const FString& Text,
	const FString& Category)
{
	FVoxelGraphFunctionInput NewInput;
	if (!FVoxelUtilities::TryImportText(Text, NewInput))
	{
		return;
	}

	NewInput.Category = Category;

	const FGuid NewGuid = FGuid::NewGuid();
	{
		const FVoxelTransaction Transaction(TerminalGraph, "Paste input");
		TerminalGraph.AddFunctionInput(NewGuid, NewInput);
	}

	Toolkit.GetSelection().SelectFunctionInput(TerminalGraph, NewGuid);
	Toolkit.GetSelection().RequestRename();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UObject* FVoxelMembersSchemaAction_FunctionInput::GetOuter() const
{
	return WeakTerminalGraph.Resolve();
}

void FVoxelMembersSchemaAction_FunctionInput::ApplyNewGuids(const TArray<FGuid>& NewGuids) const
{
	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return;
	}

	const FVoxelTransaction Transaction(TerminalGraph, "Reorder inputs");
	TerminalGraph->ReorderFunctionInputs(NewGuids);
}

FReply FVoxelMembersSchemaAction_FunctionInput::OnDragged(const FPointerEvent& MouseEvent)
{
	return FReply::Handled().BeginDragDrop(FVoxelMembersDragDropAction_Input::New(
		SharedThis(this),
		WeakTerminalGraph,
		MemberGuid,
		MouseEvent));
}

void FVoxelMembersSchemaAction_FunctionInput::BuildContextMenu(FMenuBuilder& MenuBuilder)
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph) ||
		!TerminalGraph->IsInheritedInput(MemberGuid))
	{
		return;
	}

	MenuBuilder.AddMenuEntry(
		INVTEXT("Go to definition"),
		INVTEXT("Go to the graph defining this input"),
		{},
		FUIAction
		{
			MakeLambdaDelegate([=, this]
			{
				const UVoxelTerminalGraph* LocalTerminalGraph = WeakTerminalGraph.Resolve();
				if (!ensure(LocalTerminalGraph))
				{
					return;
				}

				const UVoxelTerminalGraph* LastTerminalGraph = LocalTerminalGraph;
				for (const UVoxelTerminalGraph* BaseTerminalGraph : LocalTerminalGraph->GetBaseTerminalGraphs())
				{
					if (!BaseTerminalGraph->FindInput(MemberGuid))
					{
						break;
					}
					LastTerminalGraph = BaseTerminalGraph;
				}

				FVoxelUtilities::FocusObject(LastTerminalGraph);
			})
		});
}

void FVoxelMembersSchemaAction_FunctionInput::OnActionDoubleClick() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return;
	}

	const TSharedPtr<SGraphEditor> GraphEditor = Toolkit->GetActiveGraphEditor();
	if (!ensure(GraphEditor))
	{
		return;
	}

	GraphEditor->ClearSelectionSet();

	for (UEdGraphNode* Node : GraphEditor->GetCurrentGraph()->Nodes)
	{
		if (const UVoxelGraphNode_FunctionInput* InputNode = Cast<UVoxelGraphNode_FunctionInput>(Node))
		{
			if (InputNode->Guid == MemberGuid)
			{
				GraphEditor->SetNodeSelection(Node, true);
			}
		}
	}

	if (GraphEditor->GetSelectedNodes().Num() > 0)
	{
		GraphEditor->ZoomToFit(true);
	}
}

void FVoxelMembersSchemaAction_FunctionInput::OnSelected() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return;
	}

	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return;
	}

	Toolkit->GetSelection().SelectFunctionInput(*TerminalGraph, MemberGuid);
}

void FVoxelMembersSchemaAction_FunctionInput::OnDelete() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(Toolkit) ||
		!ensure(TerminalGraph))
	{
		return;
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(MemberGuid);
	if (!ensure(Input))
	{
		return;
	}

	UVoxelGraph::LoadAllGraphs();

	TArray<UVoxelGraphNode_FunctionInput*> InputNodes;
	for (UVoxelTerminalGraph* ChildTerminalGraph : TerminalGraph->GetChildTerminalGraphs_LoadedOnly())
	{
		for (UEdGraphNode* Node : ChildTerminalGraph->GetEdGraph().Nodes)
		{
			if (UVoxelGraphNode_FunctionInput* InputNode = Cast<UVoxelGraphNode_FunctionInput>(Node))
			{
				if (InputNode->Guid == MemberGuid)
				{
					InputNodes.Add(InputNode);
				}
			}
		}
	}

	EResult DeleteNodes = EResult::No;
	if (InputNodes.Num() > 0)
	{
		DeleteNodes = CreateDeletePopups("Delete input", Input->Name.ToString());

		if (DeleteNodes == EResult::Cancel)
		{
			return;
		}
	}

	{
		const FVoxelTransaction Transaction(TerminalGraph, "Delete input");

		if (DeleteNodes == EResult::Yes)
		{
			for (UVoxelGraphNode_FunctionInput* Node : InputNodes)
			{
				Node->GetGraph()->RemoveNode(Node);
			}
		}
		else
		{
			// Reset GUIDs to avoid invalid references
			for (UVoxelGraphNode_FunctionInput* Node : InputNodes)
			{
				Node->Guid = {};
			}
		}

		TerminalGraph->RemoveFunctionInput(MemberGuid);
	}

	MarkAsDeleted();

	if (const TSharedPtr<SVoxelGraphMembers> Members = GetMembers())
	{
		Members->SelectClosestAction(GetSectionID(), MemberGuid);
	}
}

void FVoxelMembersSchemaAction_FunctionInput::OnDuplicate() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(Toolkit) ||
		!ensure(TerminalGraph))
	{
		return;
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(MemberGuid);
	if (!ensure(Input))
	{
		return;
	}

	const FGuid NewGuid = FGuid::NewGuid();

	{
		const FVoxelTransaction Transaction(TerminalGraph, "Duplicate input");
		TerminalGraph->AddFunctionInput(NewGuid, MakeCopy(*Input));
	}

	Toolkit->GetSelection().SelectFunctionInput(*TerminalGraph, NewGuid);
	Toolkit->GetSelection().RequestRename();
}

bool FVoxelMembersSchemaAction_FunctionInput::OnCopy(FString& OutText) const
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return false;
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(MemberGuid);
	if (!ensure(Input))
	{
		return false;
	}

	const FVoxelGraphFunctionInput Defaults;

	FVoxelGraphFunctionInput::StaticStruct()->ExportText(OutText, Input, &Defaults, nullptr, 0, nullptr);
	return true;
}

FString FVoxelMembersSchemaAction_FunctionInput::GetName() const
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return {};
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(MemberGuid);
	if (!ensure(Input))
	{
		return {};
	}

	return Input->Name.ToString();
}

void FVoxelMembersSchemaAction_FunctionInput::SetName(const FString& Name) const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(Toolkit) ||
		!ensure(TerminalGraph))
	{
		return;
	}

	const FVoxelTransaction Transaction(TerminalGraph, "Rename input");

	TerminalGraph->UpdateFunctionInput(MemberGuid, [&](FVoxelGraphFunctionInput& Input)
	{
		Input.Name = *Name;
	});
}

void FVoxelMembersSchemaAction_FunctionInput::SetCategory(const FString& NewCategory) const
{
	if (bIsInherited)
	{
		return;
	}

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(Toolkit) ||
		!ensure(TerminalGraph))
	{
		return;
	}

	const FVoxelTransaction Transaction(TerminalGraph, "Set input category");

	TerminalGraph->UpdateFunctionInput(MemberGuid, [&](FVoxelGraphFunctionInput& Input)
	{
		Input.Category = *NewCategory;
	});
}

FString FVoxelMembersSchemaAction_FunctionInput::GetSearchString() const
{
	return MemberGuid.ToString();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPinType FVoxelMembersSchemaAction_FunctionInput::GetPinType() const
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return {};
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(MemberGuid);
	if (!ensure(Input))
	{
		return {};
	}

	return Input->Type;
}

void FVoxelMembersSchemaAction_FunctionInput::SetPinType(const FVoxelPinType& NewPinType) const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return;
	}

	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return;
	}

	const FVoxelTransaction Transaction(TerminalGraph, "Change input type");

	TerminalGraph->UpdateFunctionInput(MemberGuid, [&](FVoxelGraphFunctionInput& Input)
	{
		Input.Type = NewPinType;
		Input.Fixup();

		if (Input.DefaultPinValue.Is<FVoxelExposedSeed>())
		{
			Input.DefaultPinValue.Get<FVoxelExposedSeed>().Randomize();
		}
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMembersDragDropAction_Input::HoverTargetChanged()
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return;
	}

	const UEdGraph* EdGraph = GetHoveredGraph();
	if (!EdGraph)
	{
		FVoxelMembersDragDropAction_Base::HoverTargetChanged();
		return;
	}

	if (!IsCompatibleEdGraph(*EdGraph))
	{
		SetFeedbackMessageError("Cannot use in a different graph");
		return;
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(InputGuid);
	if (!ensure(Input))
	{
		return;
	}

	if (const UEdGraphPin* Pin = GetHoveredPin())
	{
		if (!CheckPin(Pin, { EGPD_Input }))
		{
			return;
		}

		if (!Input->Type.CanBeCastedTo_Schema(Pin->PinType))
		{
			SetFeedbackMessageError("The type of '" + Input->Name.ToString() + "' is not compatible with " + Pin->PinName.ToString());
			return;
		}

		SetFeedbackMessageOK("Make " + Pin->PinName.ToString() + " = " + Input->Name.ToString() + "");
		return;
	}

	if (Cast<UVoxelGraphNode_FunctionInput>(GetHoveredNode()))
	{
		SetFeedbackMessageOK("Change node input");
		return;
	}

	FVoxelMembersDragDropAction_Base::HoverTargetChanged();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FReply FVoxelMembersDragDropAction_Input::DroppedOnPin(
	const UE_506_SWITCH(FVector2D, FVector2f&) ScreenPosition,
	const UE_506_SWITCH(FVector2D, FVector2f&) GraphPosition)
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return FReply::Unhandled();
	}

	const UEdGraph* EdGraph = GetHoveredGraph();
	if (!EdGraph)
	{
		return FReply::Unhandled();
	}

	if (!IsCompatibleEdGraph(*EdGraph))
	{
		return FReply::Unhandled();
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(InputGuid);
	if (!ensure(Input))
	{
		return FReply::Unhandled();
	}

	UEdGraphPin* Pin = GetHoveredPin();
	if (!ensure(Pin))
	{
		return FReply::Unhandled();
	}

	if (Pin->bOrphanedPin ||
		Pin->bNotConnectable ||
		Pin->Direction != EGPD_Input ||
		!Input->Type.CanBeCastedTo_Schema(Pin->PinType))
	{
		return FReply::Unhandled();
	}

	FVoxelGraphSchemaAction_NewFunctionInputUsage Action;
	Action.Guid = InputGuid;
	Action.PerformAction(Pin->GetOwningNode()->GetGraph(), Pin, GraphPosition, true);

	return FReply::Handled();
}

FReply FVoxelMembersDragDropAction_Input::DroppedOnNode(
	const UE_506_SWITCH(FVector2D, FVector2f&) ScreenPosition,
	const UE_506_SWITCH(FVector2D, FVector2f&) GraphPosition)
{
	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return FReply::Unhandled();
	}

	const UEdGraph* EdGraph = GetHoveredGraph();
	if (!EdGraph)
	{
		return FReply::Unhandled();
	}

	if (!IsCompatibleEdGraph(*EdGraph))
	{
		return FReply::Unhandled();
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(InputGuid);
	if (!ensure(Input))
	{
		return FReply::Unhandled();
	}

	UEdGraphNode* Node = GetHoveredNode();
	if (!ensure(Node))
	{
		return FReply::Unhandled();
	}

	UVoxelGraphNode_FunctionInput* InputNode = Cast<UVoxelGraphNode_FunctionInput>(Node);
	if (!InputNode)
	{
		return FReply::Unhandled();
	}

	const FVoxelTransaction Transaction(TerminalGraph, "Replace input");
	InputNode->Guid = InputGuid;
	InputNode->CachedInput = *Input;
	InputNode->ReconstructNode();

	return FReply::Handled();
}

FReply FVoxelMembersDragDropAction_Input::DroppedOnPanel(
	const TSharedRef<SWidget>& Panel,
	const UE_506_SWITCH(FVector2D, FVector2f&) ScreenPosition,
	const UE_506_SWITCH(FVector2D, FVector2f&) GraphPosition,
	UEdGraph& EdGraph)
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return FReply::Unhandled();
	}

	if (!IsCompatibleEdGraph(EdGraph))
	{
		return FReply::Unhandled();
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(InputGuid);
	if (!ensure(Input))
	{
		return FReply::Unhandled();
	}

	const FModifierKeysState ModifierKeys = FSlateApplication::Get().GetModifierKeys();
	const bool bModifiedKeysActive = ModifierKeys.IsControlDown() || ModifierKeys.IsAltDown();
	const bool bAutoCreateGetter = bModifiedKeysActive ? ModifierKeys.IsControlDown() : bControlDrag;
	const bool bAutoCreateSetter = bModifiedKeysActive ? ModifierKeys.IsAltDown() : bAltDrag;

	if (bAutoCreateGetter)
	{
		FVoxelGraphSchemaAction_NewFunctionInputUsage Action;
		Action.Guid = InputGuid;
		Action.PerformAction(&EdGraph, nullptr, GraphPosition, true);
		return FReply::Handled();
	}

	if (bAutoCreateSetter)
	{
		FVoxelGraphSchemaAction_NewFunctionInputDefaultUsage Action;
		Action.Guid = InputGuid;
		Action.PerformAction(&EdGraph, nullptr, GraphPosition, true);
		return FReply::Handled();
	}

	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("VariableDroppedOn", FText::FromName(Input->Name));
	{
		MenuBuilder.AddMenuEntry(
			FText::FromString("Get " + Input->Name.ToString()),
			FText::FromString("Create Getter for input '" + Input->Name.ToString() + "'\n(Ctrl-drag to automatically create a getter)"),
			FSlateIcon(),
			FUIAction(MakeLambdaDelegate([Guid = InputGuid, Position = GraphPosition, WeakGraph = MakeWeakObjectPtr(&EdGraph)]
			{
				UEdGraph* PinnedEdGraph = WeakGraph.Get();
				if (!ensure(PinnedEdGraph))
				{
					return;
				}

				FVoxelGraphSchemaAction_NewFunctionInputUsage Action;
				Action.Guid = Guid;
				Action.PerformAction(PinnedEdGraph, nullptr, Position, true);
			}))
		);

		MenuBuilder.AddMenuEntry(
			FText::FromString("Set " + Input->Name.ToString() + " Default Value"),
			FText::FromString("Create default value Setter for input '" + Input->Name.ToString() + "'\n(Alt-drag to automatically create a customization node)"),
			FSlateIcon(),
			FUIAction(MakeLambdaDelegate([Guid = InputGuid, Position = GraphPosition, WeakGraph = MakeWeakObjectPtr(&EdGraph)]
			{
				UEdGraph* PinnedEdGraph = WeakGraph.Get();
				if (!ensure(PinnedEdGraph))
				{
					return;
				}

				FVoxelGraphSchemaAction_NewFunctionInputDefaultUsage Action;
				Action.Guid = Guid;
				Action.PerformAction(PinnedEdGraph, nullptr, Position, true);
			}))
			);

		MenuBuilder.AddMenuEntry(
			FText::FromString("Set " + Input->Name.ToString() + " Preview Value"),
			FText::FromString("Create preview value Setter for input '" + Input->Name.ToString() + "'"),
			FSlateIcon(),
			FUIAction(MakeLambdaDelegate([Guid = InputGuid, Position = GraphPosition, WeakGraph = MakeWeakObjectPtr(&EdGraph)]
			{
				UEdGraph* PinnedEdGraph = WeakGraph.Get();
				if (!ensure(PinnedEdGraph))
				{
					return;
				}

				FVoxelGraphSchemaAction_NewFunctionInputPreviewUsage Action;
				Action.Guid = Guid;
				Action.PerformAction(PinnedEdGraph, nullptr, Position, true);
			}))
		);
	}
	MenuBuilder.EndSection();

	FSlateApplication::Get().PushMenu(
		Panel,
		FWidgetPath(),
		MenuBuilder.MakeWidget(),
		ScreenPosition,
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
	);

	return FReply::Handled();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMembersDragDropAction_Input::GetDefaultStatusSymbol(
	const FSlateBrush*& PrimaryBrushOut,
	FSlateColor& IconColorOut,
	FSlateBrush const*& SecondaryBrushOut,
	FSlateColor& SecondaryColorOut) const
{
	FGraphSchemaActionDragDropAction::GetDefaultStatusSymbol(PrimaryBrushOut, IconColorOut, SecondaryBrushOut, SecondaryColorOut);

	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return;
	}

	const FVoxelGraphFunctionInput* Input = TerminalGraph->FindInput(InputGuid);
	if (!ensure(Input))
	{
		return;
	}

	PrimaryBrushOut = FVoxelGraphVisuals::GetPinIcon(Input->Type).GetIcon();
	IconColorOut = FVoxelGraphVisuals::GetPinColor(Input->Type);
}

bool FVoxelMembersDragDropAction_Input::IsCompatibleEdGraph(const UEdGraph& EdGraph) const
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!ensure(TerminalGraph))
	{
		return false;
	}

	return EdGraph.GetTypedOuter<UVoxelTerminalGraph>() == TerminalGraph;
}