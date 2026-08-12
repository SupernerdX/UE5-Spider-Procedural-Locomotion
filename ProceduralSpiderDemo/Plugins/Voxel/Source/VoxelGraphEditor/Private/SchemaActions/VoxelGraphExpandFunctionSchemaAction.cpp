// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphExpandFunctionSchemaAction.h"

#include "VoxelGraphSchema.h"
#include "VoxelGraphToolkit.h"
#include "VoxelTerminalGraph.h"
#include "Nodes/VoxelGraphNode_FunctionInput.h"
#include "Nodes/VoxelGraphNode_FunctionOutput.h"
#include "Nodes/VoxelGraphNode_CallFunction.h"

UEdGraphNode* FVoxelGraphSchemaAction_ExpandFunction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = FVoxelGraphToolkit::Get(ParentGraph);
	if (!ensure(Toolkit))
	{
		return nullptr;
	}

	const TSharedPtr<SGraphEditor> GraphEditor = Toolkit->FindGraphEditor(*ParentGraph);
	if (!ensure(GraphEditor))
	{
		return nullptr;
	}

	if (!ensure(GraphEditor->GetNumberOfSelectedNodes() == 1))
	{
		return nullptr;
	}

	UVoxelGraphNode_CallMemberFunction* FunctionNode = Cast<UVoxelGraphNode_CallMemberFunction>(GraphEditor->GetSingleSelectedNode());
	if (!ensure(FunctionNode))
	{
		return nullptr;
	}

	if (!ensure(FunctionNode->Guid.IsValid()))
	{
		return nullptr;
	}

	const UVoxelTerminalGraph* FunctionGraph = Toolkit->Asset->FindTerminalGraph(FunctionNode->Guid);
	if (!ensure(FunctionGraph))
	{
		return nullptr;
	}

	const FVoxelTransaction Transaction(Toolkit->Asset, "Expand Function");

	FindNearestSuitableLocation(GraphEditor, FunctionNode, FunctionGraph);
	GroupNodesToCopy(FunctionGraph->GetEdGraph());

	// Copy and Paste
	{
		FString ExportedText;
		ExportNodes(ExportedText);
		ImportNodes(Toolkit->FindGraphEditor(*ParentGraph), ParentGraph, ExportedText);
	}

	ConnectNewNodes(FunctionNode);

	// Add comment to surround all function nodes
	{
		TSet<TVoxelObjectPtr<UObject>> SelectedNodes;
		for (UObject* Obj : GraphEditor->GetSelectedNodes())
		{
			SelectedNodes.Add(Obj);
		}

		FVoxelGraphSchemaAction_NewComment CommentAction;
		UEdGraphNode_Comment* NewComment = Cast<UEdGraphNode_Comment>(CommentAction.PerformAction(ParentGraph, nullptr, SuitablePosition));
		NewComment->NodeComment = FunctionGraph->GetDisplayName() + " expanded nodes";

		// We have to wait for slate to process newly created nodes desired size to wrap comment around them
		FVoxelUtilities::DelayedCall([WeakComment = MakeWeakObjectPtr(NewComment), WeakGraphEditor = MakeWeakPtr(GraphEditor), SelectedNodes]
		{
			UEdGraphNode_Comment* Comment = WeakComment.Get();
			const TSharedPtr<SGraphEditor> PinnedGraphEditor = WeakGraphEditor.Pin();
			if (!ensure(Comment) ||
				!ensure(PinnedGraphEditor))
			{
				return;
			}

			PinnedGraphEditor->ClearSelectionSet();
			for (const TVoxelObjectPtr<UObject>& WeakNode : SelectedNodes)
			{
				if (UEdGraphNode* Node = Cast<UEdGraphNode>(WeakNode.Resolve()))
				{
					PinnedGraphEditor->SetNodeSelection(Node, true);
				}
			}

			FSlateRect Bounds;
			if (PinnedGraphEditor->GetBoundsForSelectedNodes(Bounds, 50.f))
			{
				Comment->PreEditChange(nullptr);
				Comment->SetBounds(Bounds);
				Comment->PostEditChange();
			}

			PinnedGraphEditor->NotifyGraphChanged();
			PinnedGraphEditor->ZoomToFit(true);
		}, 0.1f);
	}

	FunctionNode->DestroyNode();

	GraphEditor->NotifyGraphChanged();

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphSchemaAction_ExpandFunction::FindNearestSuitableLocation(const TSharedPtr<SGraphEditor>& GraphEditor, const UVoxelGraphNode_CallMemberFunction* FunctionNode, const UVoxelTerminalGraph* FunctionGraph)
{
	FSlateRect Bounds(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);

	GraphEditor->SelectAllNodes();
	ensure(GraphEditor->GetBoundsForSelectedNodes(Bounds, 50.f));

	auto GetDiffValue = [](const int32 Value, const int32 MinValue, const int32 MaxValue, int32& OutDiffValue, int32& OutValue, int32& OutDirection)
	{
		if (FMath::Abs(Value - MinValue) > FMath::Abs(Value - MaxValue))
		{
			OutDiffValue = FMath::Abs(Value - MaxValue);
			OutValue = MaxValue;
			OutDirection = 1;
		}
		else
		{
			OutDiffValue = FMath::Abs(Value - MinValue);
			OutValue = MinValue;
			OutDirection = -1;
		}
	};

	FIntPoint Diff;
	FIntPoint Location;
	FIntPoint Direction;
	GetDiffValue(FunctionNode->NodePosX, Bounds.Left, Bounds.Right, Diff.X, Location.X, Direction.X);
	GetDiffValue(FunctionNode->NodePosY, Bounds.Top, Bounds.Bottom, Diff.Y, Location.Y, Direction.Y);

	FIntPoint AvgPosition = FIntPoint::ZeroValue;
	int32 NodesCount = 0;
	for (const UEdGraphNode* Node : FunctionGraph->GetEdGraph().Nodes)
	{
		if (!ensure(Node))
		{
			continue;
		}

		if (Node->IsA<UVoxelGraphNode_FunctionInput>() ||
			Node->IsA<UVoxelGraphNode_FunctionOutput>())
		{
			continue;
		}

		AvgPosition.X += Node->NodePosX;
		AvgPosition.Y += Node->NodePosY;
		NodesCount++;
	}

	AvgPosition /= FMath::Max(1, NodesCount);

	if (Diff.X < Diff.Y)
	{
		SuitablePosition = { Location.X + (AvgPosition.X + 350) * Direction.X, FunctionNode->NodePosY };
	}
	else
	{
		SuitablePosition = { FunctionNode->NodePosX, Location.Y + (AvgPosition.Y + 350) * Direction.Y };
	}
}

void FVoxelGraphSchemaAction_ExpandFunction::GroupNodesToCopy(const UEdGraph& FunctionEdGraph)
{
	for (UEdGraphNode* Node : FunctionEdGraph.Nodes)
	{
		if (!ensure(Node))
		{
			continue;
		}

		if (Node->IsA<UVoxelGraphNode_FunctionInput>() ||
			Node->IsA<UVoxelGraphNode_FunctionOutput>())
		{
			continue;
		}

		TSharedRef<FCopiedNode> NodeToCopy = MakeShared<FCopiedNode>();
		NodeToCopy->OriginalNode = Node;

		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (!Pin->HasAnyConnections())
			{
				continue;
			}

			for (const UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				if (!ensure(LinkedPin))
				{
					continue;
				}

				if (const UVoxelGraphNode_FunctionInput* InputNode = Cast<UVoxelGraphNode_FunctionInput>(LinkedPin->GetOwningNode()))
				{
					NodeToCopy->MappedOriginalPinsToInputsOutputs.FindOrAdd(Pin, {}).Add(InputNode->Guid);
				}
				else if (const UVoxelGraphNode_FunctionOutput* OutputNode = Cast<UVoxelGraphNode_FunctionOutput>(LinkedPin->GetOwningNode()))
				{
					NodeToCopy->MappedOriginalPinsToInputsOutputs.FindOrAdd(Pin, {}).Add(OutputNode->Guid);
				}
			}
		}

		CopiedNodes.Add(Node->NodeGuid, NodeToCopy);
	}
}

void FVoxelGraphSchemaAction_ExpandFunction::ConnectNewNodes(const UVoxelGraphNode_CallMemberFunction* FunctionNode)
{
	for (const auto& It : CopiedNodes)
	{
		const UEdGraphNode* NewNode = It.Value->NewNode.Resolve();
		if (!ensure(NewNode))
		{
			continue;
		}

		for (const auto& MappedIt : It.Value->MappedOriginalPinsToInputsOutputs)
		{
			const UEdGraphPin* OriginalPin = MappedIt.Key.Get();
			if (!ensure(OriginalPin))
			{
				continue;
			}

			UEdGraphPin* NewPin = NewNode->FindPin(OriginalPin->PinName, OriginalPin->Direction);
			if (!ensure(NewPin))
			{
				continue;
			}

			for (FGuid InputOutputGuid : MappedIt.Value)
			{
				UEdGraphPin* FunctionPin = FunctionNode->FindPin(InputOutputGuid.ToString());
				if (!ensure(FunctionPin))
				{
					continue;
				}

				TArray<UEdGraphPin*> LinkedPins = FunctionPin->LinkedTo;
				FunctionPin->BreakAllPinLinks();

				for (UEdGraphPin* LinkedPin : LinkedPins)
				{
					NewPin->MakeLinkTo(LinkedPin);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphSchemaAction_ExpandFunction::ExportNodes(FString& ExportText) const
{
	TSet<UObject*> Nodes;
	for (const auto& It : CopiedNodes)
	{
		UEdGraphNode* Node = It.Value->OriginalNode.Resolve();
		if (!ensure(Node) ||
			!Node->CanDuplicateNode())
		{
			continue;
		}

		Node->PrepareForCopying();
		Nodes.Add(Node);
	}

	FEdGraphUtilities::ExportNodesToText(Nodes, ExportText);
}

void FVoxelGraphSchemaAction_ExpandFunction::ImportNodes(const TSharedPtr<SGraphEditor>& GraphEditor, UEdGraph* DestGraph, const FString& ExportText)
{
	FVoxelTransaction Transaction(DestGraph);

	// Clear the selection set
	GraphEditor->ClearSelectionSet();

	TSet<UEdGraphNode*> PastedNodes;

	// Import the nodes
	FEdGraphUtilities::ImportNodesFromText(DestGraph, ExportText, PastedNodes);

	for (auto It = PastedNodes.CreateIterator(); It; ++It)
	{
		UEdGraphNode* Node = *It;
		if (!ensure(Node))
		{
			It.RemoveCurrent();
			continue;
		}

		if (!Node->CanDuplicateNode())
		{
			Node->DestroyNode();
			It.RemoveCurrent();
		}
	}

	TMap<FGuid, FGuid> UpdatedLocalVariables;
	for (UEdGraphNode* Node : PastedNodes)
	{
		Node->NodePosX += SuitablePosition.X;
		Node->NodePosY += SuitablePosition.Y;

		Node->SnapToGrid(SNodePanel::GetSnapGridSize());

		const TSharedPtr<FCopiedNode> CopiedNode = CopiedNodes.FindRef(Node->NodeGuid);
		if (ensure(CopiedNode))
		{
			CopiedNode->NewNode = Node;
		}

		PastedNodesBounds.Left = FMath::Min(Node->NodePosX, PastedNodesBounds.Left);
		PastedNodesBounds.Top = FMath::Min(Node->NodePosY, PastedNodesBounds.Top);

		PastedNodesBounds.Right = FMath::Max(Node->NodePosX, PastedNodesBounds.Right);
		PastedNodesBounds.Bottom = FMath::Max(Node->NodePosY, PastedNodesBounds.Bottom);

		// Give new node a different GUID from the old one
		Node->CreateNewGuid();
	}

	for (UEdGraphNode* Node : PastedNodes)
	{
		GraphEditor->SetNodeSelection(Node, true);
	}
}