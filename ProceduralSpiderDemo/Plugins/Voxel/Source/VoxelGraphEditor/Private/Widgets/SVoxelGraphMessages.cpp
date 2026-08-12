// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphMessages.h"
#include "VoxelGraph.h"
#include "VoxelMessage.h"
#include "VoxelTerminalGraph.h"
#include "SVoxelNotification.h"
#include "VoxelMessageTokens.h"
#include "Nodes/VoxelGraphNode.h"
#include "VoxelGraphMessageTokens.h"
#include "VoxelTerminalGraphRuntime.h"
#include "Logging/TokenizedMessage.h"

void SVoxelGraphMessages::Construct(const FArguments& Args)
{
	ensure(Args._Graph);
	WeakGraph = Args._Graph;

	ChildSlot
	[
		SNew(SScrollBox)
		.Orientation(Orient_Horizontal)
		+ SScrollBox::Slot()
		.FillSize(1.f)
		[
			SAssignNew(Tree, STree)
			.TreeItemsSource(&Nodes)
			.OnGenerateRow_Lambda([=](const TSharedPtr<INode>& Node, const TSharedRef<STableViewBase>& OwnerTable)
			{
				return
					SNew(STableRow<TSharedPtr<INode>>, OwnerTable)
					[
						Node->GetWidget()
					];
			})
			.OnGetChildren_Lambda([=](const TSharedPtr<INode>& Node, TArray<TSharedPtr<INode>>& OutChildren)
			{
				OutChildren = Node->GetChildren();
			})
			.SelectionMode(ESelectionMode::None)
		]
	];

	Tree->SetItemExpansion(CompileNode, true);
	Tree->SetItemExpansion(RuntimeNode, true);

	UpdateNodes();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelGraphMessages::UpdateNodes()
{
	VOXEL_FUNCTION_COUNTER();

	UVoxelGraph* RootGraph = WeakGraph.Resolve();
	if (!ensure(RootGraph))
	{
		return;
	}

	MessageToCanonMessage.Reset();
	HashToCanonMessage.Reset();

	TSet<TSharedPtr<INode>> VisitedNodes;
	TSet<TSharedPtr<INode>> NodesToExpand;

	RootGraph->ForeachTerminalGraph_NoInheritance([&](UVoxelTerminalGraph& TerminalGraph)
	{
		for (UEdGraphNode* GraphNode : TerminalGraph.GetEdGraph().Nodes)
		{
			if (!GraphNode->bHasCompilerMessage)
			{
				continue;
			}

			GraphNode->bHasCompilerMessage = false;
			GraphNode->ErrorType = EMessageSeverity::Info;
			GraphNode->ErrorMsg.Reset();

			UVoxelGraphNode* VoxelNode = Cast<UVoxelGraphNode>(GraphNode);
			if (!ensure(VoxelNode))
			{
				continue;
			}

			VoxelNode->RefreshNode();
		}

		const UVoxelTerminalGraphRuntime& Runtime = TerminalGraph.GetRuntime();

		TSharedPtr<FGraphNode>& CompileGraphNode = CompileNode->GraphToNode.FindOrAdd(&TerminalGraph);
		if (!CompileGraphNode)
		{
			CompileGraphNode = MakeShared<FGraphNode>(&TerminalGraph);
			NodesToExpand.Add(CompileGraphNode);
		}

		TSharedPtr<FGraphNode>& RuntimeGraphNode = RuntimeNode->GraphToNode.FindOrAdd(&TerminalGraph);
		if (!RuntimeGraphNode)
		{
			RuntimeGraphNode = MakeShared<FGraphNode>(&TerminalGraph);
			NodesToExpand.Add(RuntimeGraphNode);
		}

		for (const TSharedRef<FVoxelMessage>& Message : Runtime.GetCompileMessages())
		{
			VisitedNodes.Add(CompileGraphNode);

			for (const TSharedRef<FVoxelMessageToken>& Token : Message->GetTokens())
			{
				UVoxelGraphNode* GraphNode = nullptr;
				if (const TSharedPtr<FVoxelMessageToken_NodeRef> TypedToken = CastStruct<FVoxelMessageToken_NodeRef>(Token))
				{
					GraphNode = Cast<UVoxelGraphNode>(TypedToken->NodeRef.GetGraphNode_EditorOnly());
				}
				if (const TSharedPtr<FVoxelMessageToken_PinRef> TypedToken = CastStruct<FVoxelMessageToken_PinRef>(Token))
				{
					GraphNode = Cast<UVoxelGraphNode>(TypedToken->PinRef.NodeRef.GetGraphNode_EditorOnly());
				}
				if (const TSharedPtr<FVoxelMessageToken_Pin> TypedToken = CastStruct<FVoxelMessageToken_Pin>(Token))
				{
					if (const UEdGraphPin* Pin = TypedToken->PinReference.Get())
					{
						GraphNode = Cast<UVoxelGraphNode>(Pin->GetOwningNodeUnchecked());
					}
				}

				if (!GraphNode ||
					// Try not to leak errors
					GraphNode->GetTypedOuter<UVoxelTerminalGraph>() != &TerminalGraph)
				{
					continue;
				}

				GraphNode->bHasCompilerMessage = true;
				GraphNode->ErrorType = FMath::Min<int32>(GraphNode->ErrorType, Message->GetMessageSeverity());

				if (!GraphNode->ErrorMsg.IsEmpty())
				{
					GraphNode->ErrorMsg += "\n";
				}
				GraphNode->ErrorMsg += Message->ToString();

				GraphNode->RefreshNode();
			}

			TSharedPtr<FMessageNode>& MessageNode = CompileGraphNode->MessageToNode.FindOrAdd(Message);
			if (!MessageNode)
			{
				MessageNode = MakeShared<FMessageNode>(Message);
				NodesToExpand.Add(MessageNode);
			}
			VisitedNodes.Add(MessageNode);
		}

		for (const TSharedRef<FVoxelMessage>& Message : Runtime.GetRuntimeMessages())
		{
			VisitedNodes.Add(RuntimeGraphNode);

			const TSharedRef<FVoxelMessage> CanonMessage = GetCanonMessage(Message);

			TSharedPtr<FMessageNode>& MessageNode = RuntimeGraphNode->MessageToNode.FindOrAdd(CanonMessage);
			if (!MessageNode)
			{
				MessageNode = MakeShared<FMessageNode>(CanonMessage);
				NodesToExpand.Add(MessageNode);
			}
			VisitedNodes.Add(MessageNode);
		}
	});

	const auto Cleanup = [&](FRootNode& RootNode)
	{
		for (auto GraphNodeIt = RootNode.GraphToNode.CreateIterator(); GraphNodeIt; ++GraphNodeIt)
		{
			if (!VisitedNodes.Contains(GraphNodeIt.Value()))
			{
				GraphNodeIt.RemoveCurrent();
				continue;
			}

			// Put errors on top
			const auto SortPredicate = [](const TSharedPtr<FVoxelMessage>& A, const TSharedPtr<FVoxelMessage>& B)
			{
				return A->GetMessageSeverity() < B->GetMessageSeverity();
			};

			GraphNodeIt.Value()->MessageToNode.KeySort(SortPredicate);

			for (auto MessageIt = GraphNodeIt.Value()->MessageToNode.CreateIterator(); MessageIt; ++MessageIt)
			{
				if (!VisitedNodes.Contains(MessageIt.Value()))
				{
					MessageIt.RemoveCurrent();
				}
			}
		}
	};

	Cleanup(*CompileNode);
	Cleanup(*RuntimeNode);

	// Always expand new items
	if (!Nodes.Contains(CompileNode))
	{
		NodesToExpand.Add(CompileNode);
	}
	if (!Nodes.Contains(RuntimeNode))
	{
		NodesToExpand.Add(RuntimeNode);
	}

	Nodes.Reset();

	if (CompileNode->GraphToNode.Num() > 0)
	{
		Nodes.Add(CompileNode);
	}
	if (RuntimeNode->GraphToNode.Num() > 0)
	{
		Nodes.Add(RuntimeNode);
	}

	for (const TSharedPtr<INode>& Node : NodesToExpand)
	{
		Tree->SetItemExpansion(Node, true);
	}

	Tree->RequestTreeRefresh();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> SVoxelGraphMessages::FRootNode::GetWidget() const
{
	return SNew(STextBlock).Text(FText::FromString(Text));
}

TArray<TSharedPtr<SVoxelGraphMessages::INode>> SVoxelGraphMessages::FRootNode::GetChildren() const
{
	TArray<TSharedPtr<FGraphNode>> Children;
	GraphToNode.GenerateValueArray(Children);
	return TArray<TSharedPtr<INode>>(Children);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> SVoxelGraphMessages::FGraphNode::GetWidget() const
{
	return SNew(STextBlock).Text_Lambda([WeakGraph = WeakGraph]
	{
		const UVoxelTerminalGraph* Graph = WeakGraph.Resolve();
		if (!Graph)
		{
			return INVTEXT("Invalid");
		}

		if (Graph->IsMainTerminalGraph())
		{
			return INVTEXT("Main Graph");
		}

		if (Graph->IsEditorTerminalGraph())
		{
			return INVTEXT("Editor Graph");
		}

		return FText::FromString(Graph->GetDisplayName());
	});
}

TArray<TSharedPtr<SVoxelGraphMessages::INode>> SVoxelGraphMessages::FGraphNode::GetChildren() const
{
	TArray<TSharedPtr<INode>> Children;
	for (const auto& It : MessageToNode)
	{
		Children.Add(It.Value);
	}
	return Children;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> SVoxelGraphMessages::FMessageNode::GetWidget() const
{
	return SNew(SVoxelNotification, Message);
}

TArray<TSharedPtr<SVoxelGraphMessages::INode>> SVoxelGraphMessages::FMessageNode::GetChildren() const
{
	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelMessage> SVoxelGraphMessages::GetCanonMessage(const TSharedRef<FVoxelMessage>& Message)
{
	if (const TSharedPtr<FVoxelMessage> CanonMessage = MessageToCanonMessage.FindRef(Message).Pin())
	{
		return CanonMessage.ToSharedRef();
	}

	TWeakPtr<FVoxelMessage>& WeakCanonMessage = HashToCanonMessage.FindOrAdd(Message->GetHash());

	if (const TSharedPtr<FVoxelMessage> CanonMessage = WeakCanonMessage.Pin())
	{
		MessageToCanonMessage.Add_EnsureNew(Message, CanonMessage);
		return CanonMessage.ToSharedRef();
	}

	WeakCanonMessage = Message;
	MessageToCanonMessage.Add_EnsureNew(Message, Message);
	return Message;
}