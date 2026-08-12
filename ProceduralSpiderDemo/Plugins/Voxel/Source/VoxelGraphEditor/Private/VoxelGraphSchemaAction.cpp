// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphSchemaAction.h"
#include "VoxelGraphVisuals.h"
#include "VoxelGraphToolkit.h"
#include "Selection/VoxelGraphSelection.h"
#include "Nodes/VoxelGraphNode_Knot.h"
#include "Nodes/VoxelGraphNode_FunctionInput.h"
#include "Nodes/VoxelGraphNode_FunctionOutput.h"
#include "Nodes/VoxelGraphNode_Struct.h"
#include "Nodes/VoxelGraphNode_Parameter.h"
#include "Nodes/VoxelGraphNode_CallFunction.h"
#include "Nodes/VoxelGraphNode_LocalVariable.h"
#include "Nodes/VoxelGraphNode_CustomizeParameter.h"
#include "Nodes/VoxelGraphNode_CallParentMainGraph.h"
#include "VoxelGraphCommandManager.h"
#include "EdGraphNode_Comment.h"

UVoxelGraphNode* FVoxelGraphSchemaAction::Apply(UEdGraph& ParentGraph, const UE_506_SWITCH(FVector2D, FVector2f)& Location, const bool bSelectNewNode)
{
	return CastEnsured<UVoxelGraphNode>(PerformAction(&ParentGraph, nullptr, Location, bSelectNewNode));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	const FVoxelTransaction Transaction(ParentGraph, "Add comment");

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(ParentGraph);
	if (!ensure(Toolkit))
	{
		return nullptr;
	}

	const TSharedPtr<SGraphEditor> GraphEditor = Toolkit->GetActiveGraphEditor();
	if (!ensure(GraphEditor))
	{
		return nullptr;
	}

	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();

	UE_506_SWITCH(FVector2D, FVector2f) SpawnLocation = Location;

	FSlateRect Bounds;
	if (GraphEditor->GetBoundsForSelectedNodes(Bounds, 50.f))
	{
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
	}

	return FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation);
}

void FVoxelGraphSchemaAction_NewComment::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	static const FSlateIcon CommentIcon("EditorStyle", "Icons.Comment");
	Icon = CommentIcon;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_Paste::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	const FVoxelTransaction Transaction(ParentGraph, "Paste nodes");

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(ParentGraph);
	if (!ensure(Toolkit))
	{
		return nullptr;
	}

	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	Toolkit->GetCommandManager().PasteNodes(FVector2D(Location), TextToImport);
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewCallFunctionNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	const FVoxelTransaction Transaction(ParentGraph, "New function node");

	FGraphNodeCreator<UVoxelGraphNode_CallMemberFunction> NodeCreator(*ParentGraph);
	UVoxelGraphNode_CallMemberFunction* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->Guid = Guid;
	Node->bCallParent = bCallParent;
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewCallFunctionNode::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FSlateIcon("EditorStyle", "GraphEditor.Function_16x");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewCallExternalFunctionNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	const FVoxelTransaction Transaction(ParentGraph, "New function node");

	FGraphNodeCreator<UVoxelGraphNode_CallExternalFunction> NodeCreator(*ParentGraph);
	UVoxelGraphNode_CallExternalFunction* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->FunctionLibrary = FunctionLibrary.Resolve();
	Node->Guid = Guid;
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewCallExternalFunctionNode::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FSlateIcon("EditorStyle", "GraphEditor.Function_16x");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewCallParentMainGraphNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	const FVoxelTransaction Transaction(ParentGraph, "New call parent main graph node");

	FGraphNodeCreator<UVoxelGraphNode_CallParentMainGraph> NodeCreator(*ParentGraph);
	UVoxelGraphNode_CallParentMainGraph* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewCallParentMainGraphNode::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FSlateIcon("EditorStyle", "GraphEditor.Function_16x");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewParameterUsage::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	check(Guid.IsValid());
	const FVoxelTransaction Transaction(ParentGraph, "Create new parameter usage");

	FGraphNodeCreator<UVoxelGraphNode_Parameter> NodeCreator(*ParentGraph);
	UVoxelGraphNode_Parameter* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->Guid = Guid;
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewParameterUsage::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FVoxelGraphVisuals::GetPinIcon(Type);
	Color = FVoxelGraphVisuals::GetPinColor(Type);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewFunctionInputUsage::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	check(Guid.IsValid());
	const FVoxelTransaction Transaction(ParentGraph, "Create new function input usage");

	FGraphNodeCreator<UVoxelGraphNode_FunctionInput> NodeCreator(*ParentGraph);
	UVoxelGraphNode_FunctionInput* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->Guid = Guid;
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewFunctionInputUsage::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FVoxelGraphVisuals::GetPinIcon(Type);
	Color = FVoxelGraphVisuals::GetPinColor(Type);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewFunctionInputDefaultUsage::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	check(Guid.IsValid());
	const FVoxelTransaction Transaction(ParentGraph, "Create new function input default usage");

	FGraphNodeCreator<UVoxelGraphNode_FunctionInputDefault> NodeCreator(*ParentGraph);
	UVoxelGraphNode_FunctionInputDefault* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->Guid = Guid;
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewFunctionInputDefaultUsage::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FVoxelGraphVisuals::GetPinIcon(Type);
	Color = FVoxelGraphVisuals::GetPinColor(Type);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewFunctionInputPreviewUsage::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	check(Guid.IsValid());
	const FVoxelTransaction Transaction(ParentGraph, "Create new function input preview usage");

	FGraphNodeCreator<UVoxelGraphNode_FunctionInputPreview> NodeCreator(*ParentGraph);
	UVoxelGraphNode_FunctionInputPreview* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->Guid = Guid;
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewFunctionInputPreviewUsage::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FVoxelGraphVisuals::GetPinIcon(Type);
	Color = FVoxelGraphVisuals::GetPinColor(Type);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewOutputUsage::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	check(Guid.IsValid());
	const FVoxelTransaction Transaction(ParentGraph, "Create new variable usage");

	FGraphNodeCreator<UVoxelGraphNode_FunctionOutput> NodeCreator(*ParentGraph);
	UVoxelGraphNode_FunctionOutput* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->Guid = Guid;
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewOutputUsage::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FVoxelGraphVisuals::GetPinIcon(Type);
	Color = FVoxelGraphVisuals::GetPinColor(Type);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewLocalVariableUsage::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	check(Guid.IsValid());
	const FVoxelTransaction Transaction(ParentGraph, "Create new variable usage");

	UVoxelGraphNode_LocalVariable* Node;
	if (bIsDeclaration)
	{
		FGraphNodeCreator<UVoxelGraphNode_LocalVariableDeclaration> NodeCreator(*ParentGraph);
		Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
		Node->Guid = Guid;
		Node->NodePosX = Location.X;
		Node->NodePosY = Location.Y;
		NodeCreator.Finalize();
	}
	else
	{
		FGraphNodeCreator<UVoxelGraphNode_LocalVariableUsage> NodeCreator(*ParentGraph);
		Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
		Node->Guid = Guid;
		Node->NodePosX = Location.X;
		Node->NodePosY = Location.Y;
		NodeCreator.Finalize();
	}

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewLocalVariableUsage::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FVoxelGraphVisuals::GetPinIcon(Type);
	Color = FVoxelGraphVisuals::GetPinColor(Type);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewParameter::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = ParentGraph ? FVoxelGraphToolkit::Get(ParentGraph) : WeakToolkit.Pin();
	if (!ensure(Toolkit))
	{
		return nullptr;
	}

	UVoxelGraph* Graph = Toolkit->Asset;
	if (!ensure(Graph))
	{
		return nullptr;
	}

	const FVoxelTransaction Transaction(Graph, "Create new parameter");

	const FGuid Guid = FGuid::NewGuid();

	FVoxelParameter NewParameter;
	NewParameter.Category = Category;

	if (FromPin)
	{
		const FString PinName = FromPin->GetDisplayName().ToString();
		if (!PinName.TrimStartAndEnd().IsEmpty())
		{
			NewParameter.Name = *PinName;
		}
		NewParameter.Type = FromPin->PinType;

		if (!NewParameter.Type.IsBufferArray())
		{
			NewParameter.Type = NewParameter.Type.GetInnerType();
		}
	}
	else
	{
		NewParameter.Type = TypeOverride.Get(FVoxelPinType::Make<float>());
		NewParameter.Fixup();
	}

	if (!NameOverride.IsNone())
	{
		NewParameter.Name = NameOverride;
	}

	UVoxelGraphNode_Parameter* Node = nullptr;
	{
		Graph->AddParameter(Guid, NewParameter);
		Graph->Fixup();

		if (FromPin &&
			FVoxelPinType(FromPin->PinType).HasPinDefaultValue())
		{
			// Name might have been updated by Fixup
			const FName NewName = Graph->FindParameterChecked(Guid).Name;
			const FVoxelPinValue NewValue = FVoxelPinValue::MakeFromPinDefaultValue(*FromPin);

			ensure(Graph->SetParameter(NewName, NewValue));
		}

		if (ParentGraph)
		{
			const FVoxelTransaction GraphTransaction(ParentGraph);

			FGraphNodeCreator<UVoxelGraphNode_Parameter> NodeCreator(*ParentGraph);
			Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
			Node->Guid = Guid;
			Node->NodePosX = Location.X;
			Node->NodePosY = Location.Y;
			NodeCreator.Finalize();

			if (FromPin &&
				FromPin->GetOwningNode()->GetGraph() == ParentGraph)
			{
				Node->AutowireNewNode(FromPin);
			}
		}
	}

	Toolkit->GetSelection().SelectParameter(Guid);
	Toolkit->GetSelection().RequestRename();

	return Node;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewFunctionInput::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	if (!ensure(ParentGraph))
	{
		return nullptr;
	}

	const FVoxelTransaction Transaction(ParentGraph, "Create new function input");

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(ParentGraph);
	if (!ensure(Toolkit))
	{
		return nullptr;
	}

	UVoxelTerminalGraph* TerminalGraph = ParentGraph->GetTypedOuter<UVoxelTerminalGraph>();
	if (!ensure(TerminalGraph))
	{
		return nullptr;
	}

	const FGuid Guid = FGuid::NewGuid();

	FVoxelGraphFunctionInput NewInput;
	NewInput.Category = Category;

	if (FromPin)
	{
		const FString PinName = FromPin->GetDisplayName().ToString();
		if (!PinName.TrimStartAndEnd().IsEmpty())
		{
			NewInput.Name = *PinName;
		}
		NewInput.Type = FromPin->PinType;

		if (NewInput.Type.HasPinDefaultValue())
		{
			NewInput.DefaultPinValue = FVoxelPinValue::MakeFromPinDefaultValue(*FromPin);
		}
	}
	else
	{
		NewInput.Type = TypeOverride.Get(FVoxelPinType::Make<float>());
		NewInput.Fixup();
	}

	if (!NameOverride.IsNone())
	{
		NewInput.Name = NameOverride;
	}

	UVoxelGraphNode_FunctionInput* Node;
	{
		const FVoxelTransaction GraphTransaction(TerminalGraph);

		TerminalGraph->AddFunctionInput(Guid, NewInput);

		FGraphNodeCreator<UVoxelGraphNode_FunctionInput> NodeCreator(*ParentGraph);
		Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
		Node->Guid = Guid;
		Node->NodePosX = Location.X;
		Node->NodePosY = Location.Y;
		NodeCreator.Finalize();

		if (FromPin)
		{
			if (FromPin->GetOwningNode()->GetGraph() == ParentGraph)
			{
				Node->AutowireNewNode(FromPin);
			}
		}
	}

	Toolkit->GetSelection().SelectFunctionInput(*TerminalGraph, Guid);
	Toolkit->GetSelection().RequestRename();

	return Node;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewFunctionOutput::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	if (!ensure(ParentGraph))
	{
		return nullptr;
	}

	const FVoxelTransaction Transaction(ParentGraph, "Create new output");

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(ParentGraph);
	if (!ensure(Toolkit))
	{
		return nullptr;
	}

	UVoxelTerminalGraph* TerminalGraph = ParentGraph->GetTypedOuter<UVoxelTerminalGraph>();
	if (!ensure(TerminalGraph))
	{
		return nullptr;
	}

	const FGuid Guid = FGuid::NewGuid();

	FVoxelGraphFunctionOutput NewOutput;
	NewOutput.Category = Category;

	if (FromPin)
	{
		const FString PinName = FromPin->GetDisplayName().ToString();
		if (!PinName.TrimStartAndEnd().IsEmpty())
		{
			NewOutput.Name = *PinName;
		}
		NewOutput.Type = FromPin->PinType;
	}
	else
	{
		NewOutput.Type = TypeOverride.Get(FVoxelPinType::Make<float>());
	}

	if (!NameOverride.IsNone())
	{
		NewOutput.Name = NameOverride;
	}

	UVoxelGraphNode_FunctionOutput* Node;
	{
		const FVoxelTransaction GraphTransaction(TerminalGraph);

		TerminalGraph->AddFunctionOutput(Guid, NewOutput);

		FGraphNodeCreator<UVoxelGraphNode_FunctionOutput> NodeCreator(*ParentGraph);
		Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
		Node->Guid = Guid;
		Node->NodePosX = Location.X;
		Node->NodePosY = Location.Y;
		NodeCreator.Finalize();

		if (FromPin)
		{
			if (FromPin->GetOwningNode()->GetGraph() == ParentGraph)
			{
				Node->AutowireNewNode(FromPin);
			}
		}
	}

	Toolkit->GetSelection().SelectOutput(*TerminalGraph, Guid);
	Toolkit->GetSelection().RequestRename();

	return Node;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewLocalVariable::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	if (!ensure(ParentGraph))
	{
		return nullptr;
	}

	const FVoxelTransaction Transaction(ParentGraph, "Create new local variable");

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(ParentGraph);
	if (!ensure(Toolkit))
	{
		return nullptr;
	}

	UVoxelTerminalGraph* TerminalGraph = ParentGraph->GetTypedOuter<UVoxelTerminalGraph>();
	if (!ensure(TerminalGraph))
	{
		return nullptr;
	}

	const FGuid Guid = FGuid::NewGuid();

	FVoxelGraphLocalVariable NewLocalVariable;
	NewLocalVariable.Category = Category;

	if (FromPin)
	{
		const FString PinName = FromPin->GetDisplayName().ToString();
		if (!PinName.TrimStartAndEnd().IsEmpty())
		{
			NewLocalVariable.Name = *PinName;
		}
		NewLocalVariable.Type = FromPin->PinType;
	}
	else
	{
		NewLocalVariable.Type = TypeOverride.Get(FVoxelPinType::Make<float>());
	}

	if (!NameOverride.IsNone())
	{
		NewLocalVariable.Name = NameOverride;
	}

	UVoxelGraphNode_LocalVariableDeclaration* Node;
	{
		const FVoxelTransaction GraphTransaction(TerminalGraph);

		TerminalGraph->AddLocalVariable(Guid, NewLocalVariable);

		FGraphNodeCreator<UVoxelGraphNode_LocalVariableDeclaration> NodeCreator(*ParentGraph);
		Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
		Node->Guid = Guid;
		Node->NodePosX = Location.X;
		Node->NodePosY = Location.Y;
		NodeCreator.Finalize();

		if (FromPin)
		{
			if (FromPin->GetOwningNode()->GetGraph() == ParentGraph)
			{
				Node->AutowireNewNode(FromPin);
			}

			// Copy default value over
			if (NewLocalVariable.Type.HasPinDefaultValue())
			{
				UEdGraphPin& InputPin = *Node->GetInputPin(0);
				FVoxelPinValue::MakeFromPinDefaultValue(*FromPin).ApplyToPinDefaultValue(InputPin);
			}
		}
	}

	Toolkit->GetSelection().SelectLocalVariable(*TerminalGraph, Guid);
	Toolkit->GetSelection().RequestRename();

	return Node;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewFunction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = ParentGraph ? FVoxelGraphToolkit::Get(ParentGraph) : WeakToolkit.Pin();
	if (!ensure(Toolkit))
	{
		return nullptr;
	}

	UVoxelGraph* Graph = Toolkit->Asset;
	if (!ensure(Graph))
	{
		return nullptr;
	}

	const FGuid Guid = FGuid::NewGuid();

	TSet<FName> GraphNames;
	Graph->ForeachTerminalGraph_NoInheritance([&](const UVoxelTerminalGraph& TerminalGraph)
	{
		if (TerminalGraph.IsMainTerminalGraph() ||
			TerminalGraph.IsEditorTerminalGraph())
		{
			return;
		}

		GraphNames.Add(FName(TerminalGraph.GetDisplayName()));
	});

	FName NewName = "NewFunction";
	while (GraphNames.Contains(NewName))
	{
		NewName.SetNumber(NewName.GetNumber() + 1);
	}

	{
		const FVoxelTransaction Transaction(Graph, "Create new function");

		UVoxelTerminalGraph& TerminalGraph = Graph->AddTerminalGraph(Guid);
		TerminalGraph.UpdateMetadata([&](FVoxelGraphMetadata& Metadata)
		{
			Metadata.DisplayName = NewName.ToString();
			Metadata.Category = Category;
		});

		OutNewFunction = &TerminalGraph;
	}

	if (bOpenNewGraph)
	{
		Toolkit->OpenGraphAndBringToFront(OutNewFunction.Resolve()->GetEdGraph(), false);
	}

	if (Toolkit->Asset->IsFunctionLibrary())
	{
		Toolkit->CloseEmptyTab();
	}

	Toolkit->GetSelection().SelectFunction(Guid);
	Toolkit->GetSelection().RequestRename();

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewStructNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	const FVoxelTransaction Transaction(ParentGraph, "New graph node");

	FGraphNodeCreator<UVoxelGraphNode_Struct> NodeCreator(*ParentGraph);
	UVoxelGraphNode_Struct* StructNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	StructNode->Struct = *Node;
	StructNode->NodePosX = Location.X;
	StructNode->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (!FromPin)
	{
		return StructNode;
	}

	const FVoxelPinType PinType(FromPin->PinType);
	StructNode->AutowireNewNode(FromPin);

	UEdGraphPin* LinkedPin = nullptr;
	for (UEdGraphPin* Pin : FromPin->LinkedTo)
	{
		if (Pin->GetOwningNode() == StructNode)
		{
			LinkedPin = Pin;
			break;
		}
	}

	if (!LinkedPin)
	{
		return StructNode;
	}

	FVoxelPinTypeSet PromotionTypes;
	if (!ensure(LinkedPin) ||
		!StructNode->CanPromotePin(*LinkedPin, PromotionTypes))
	{
		return StructNode;
	}

	if (!PromotionTypes.Contains(PinType))
	{
		return StructNode;
	}

	StructNode->PromotePin(*LinkedPin, PinType);

	return StructNode;
}

void FVoxelGraphSchemaAction_NewStructNode::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	if (!ensure(Node))
	{
		return;
	}

	static FSlateIcon StructIcon("EditorStyle", "Kismet.AllClasses.FunctionIcon");

	Icon = StructIcon;
	Color = GetDefault<UGraphEditorSettings>()->FunctionCallNodeTitleColor;

	if (Node->IsPureNode())
	{
		Color = GetDefault<UGraphEditorSettings>()->PureFunctionCallNodeTitleColor;
	}

	if (Node->GetMetadataContainer().HasMetaDataHierarchical("NativeMakeFunc"))
	{
		Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.MakeStruct_16x");
		Color = FLinearColor::White;
	}

	if (Node->GetMetadataContainer().HasMetaDataHierarchical("NativeBreakFunc"))
	{
		Icon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.BreakStruct_16x");
		Color = FLinearColor::White;
	}

	if (Node->GetMetadataContainer().HasMetaDataHierarchical("NodeIcon"))
	{
		Icon = FVoxelGraphVisuals::GetNodeIcon(GetStringMetaDataHierarchical(&Node->GetMetadataContainer(), "NodeIcon"));
	}

	if (Node->GetMetadataContainer().HasMetaDataHierarchical("NodeIconColor"))
	{
		Color = FVoxelGraphVisuals::GetNodeColor(GetStringMetaDataHierarchical(&Node->GetMetadataContainer(), "NodeIconColor"));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewPromotableStructNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	const FVoxelTransaction Transaction(ParentGraph, "New graph node");

	FGraphNodeCreator<UVoxelGraphNode_Struct> NodeCreator(*ParentGraph);
	UVoxelGraphNode_Struct* StructNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	StructNode->Struct = *Node;
	StructNode->NodePosX = Location.X;
	StructNode->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		for (UEdGraphPin* InputPin : StructNode->GetInputPins())
		{
			if (PinTypes.Num() == 0)
			{
				break;
			}

			StructNode->PromotePin(*InputPin, PinTypes.Pop());
		}

		StructNode->AutowireNewNode(FromPin);
	}

	return StructNode;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewKnotNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	const FVoxelTransaction Transaction(ParentGraph, "New reroute node");

	FGraphNodeCreator<UVoxelGraphNode_Knot> NodeCreator(*ParentGraph);
	UVoxelGraphNode_Knot* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	Node->PropagatePinType();

	return Node;
}

void FVoxelGraphSchemaAction_NewKnotNode::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	static const FSlateIcon KnotIcon = FSlateIcon("EditorStyle", "GraphEditor.Default_16x");
	Icon = KnotIcon;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewCustomizeParameter::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, const bool bSelectNewNode)
{
	check(Guid.IsValid());
	const FVoxelTransaction Transaction(ParentGraph, "Create new customize parameter");

	FGraphNodeCreator<UVoxelGraphNode_CustomizeParameter> NodeCreator(*ParentGraph);
	UVoxelGraphNode_CustomizeParameter* Node = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	Node->Guid = Guid;
	Node->NodePosX = Location.X;
	Node->NodePosY = Location.Y;
	NodeCreator.Finalize();

	if (FromPin)
	{
		Node->AutowireNewNode(FromPin);
	}

	return Node;
}

void FVoxelGraphSchemaAction_NewCustomizeParameter::GetIcon(FSlateIcon& Icon, FLinearColor& Color)
{
	Icon = FVoxelGraphVisuals::GetPinIcon(Type);
	Color = FVoxelGraphVisuals::GetPinColor(Type);
}