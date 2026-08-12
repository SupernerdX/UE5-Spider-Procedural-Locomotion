// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMessageToken_GraphCallstack.h"
#include "VoxelNode.h"
#include "VoxelGraph.h"
#include "VoxelGraphContext.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphEditorUtilities.h"
#include "Nodes/VoxelNode_RangeDebug.h"
#include "Nodes/VoxelNode_ValueDebug.h"
#if WITH_EDITOR
#include "SVoxelCallstack.h"
#include "EdGraph/EdGraph.h"
#include "Logging/TokenizedMessage.h"
#include "Widgets/Layout/SScrollBox.h"
#endif

#if WITH_EDITOR
struct FVoxelCallstackNodeEntry : public FVoxelCallstackEntry
{
	FVoxelGraphMergedNodeRef NodeRef;

	FVoxelCallstackNodeEntry(
		const FVoxelGraphMergedNodeRef& NodeRef,
		const FString& Name,
		const FString& Prefix,
		const EType Type)
		: FVoxelCallstackEntry(Name, Prefix, Type)
		, NodeRef(NodeRef)
	{
	}

	virtual void OnClick() const override
	{
		TVoxelSet<const UEdGraphNode*> GraphNodes;
		if (const UEdGraphNode* GraphNode = NodeRef.Node.GetGraphNode_EditorOnly())
		{
			GraphNodes.Add(GraphNode);
		}
		for (const FVoxelGraphNodeRef& Node : NodeRef.MergedNodes)
		{
			if (const UEdGraphNode* GraphNode = Node.GetGraphNode_EditorOnly())
			{
				GraphNodes.Add(GraphNode);
			}
		}

		FVoxelGraphEditorUtilities::OnSelectNodes.ExecuteIfBound(GraphNodes);
	}
};
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelGraphSharedCallstack::FVoxelGraphSharedCallstack(const FVoxelGraphCallstack& Callstack)
	: NodeRef(Callstack.Node->GetNodeRef())
	, bDebugNode(
		Callstack.Node->IsA<FVoxelNode_ValueDebug>() ||
		Callstack.Node->IsA<FVoxelNode_RangeDebug>())
{
	if (Callstack.Parent)
	{
		Parent = MakeShared<FVoxelGraphSharedCallstack>(*Callstack.Parent);
	}
}

uint32 FVoxelGraphSharedCallstack::GetHash() const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelInlineArray<uint32, 64> Hashes;
	for (const FVoxelGraphSharedCallstack* Callstack = this; Callstack; Callstack = Callstack->Parent.Get())
	{
		Hashes.Add(GetTypeHash(Callstack->GetNodeRef()));
	}
	return FVoxelUtilities::MurmurHashView(Hashes);
}

FString FVoxelGraphSharedCallstack::ToDebugString() const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<const FVoxelGraphSharedCallstack*> Callstacks;
	for (const FVoxelGraphSharedCallstack* Callstack = this; Callstack; Callstack = Callstack->Parent.Get())
	{
		Callstacks.Add(Callstack);
	}

	FString Result;
	FVoxelGraphMergedNodeRef LastNodeRef;
	for (int32 Index = Callstacks.Num() - 1; Index >= 0; Index--)
	{
		const FVoxelGraphSharedCallstack* Callstack = Callstacks[Index];
		if (Callstack->NodeRef.Node.IsExplicitlyNull() ||
			Callstack->NodeRef == LastNodeRef)
		{
			continue;
		}
		LastNodeRef = Callstack->NodeRef;

		if (Callstack->NodeRef.Node.EdGraphNodeTitle.IsNone())
		{
			ensure(!Callstack->NodeRef.Node.NodeId.IsNone());
			Callstack->NodeRef.Node.NodeId.AppendString(Result);
		}
		else
		{
			Callstack->NodeRef.Node.EdGraphNodeTitle.AppendString(Result);
		}

		if (Index > 0)
		{
			Result += " -> ";
		}
	}
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint32 FVoxelMessageToken_GraphCallstack::GetHash() const
{
	return Callstack->GetHash();
}

FString FVoxelMessageToken_GraphCallstack::ToString() const
{
	return "\nCallstack: " + Callstack->ToDebugString();
}

TSharedRef<IMessageToken> FVoxelMessageToken_GraphCallstack::GetMessageToken() const
{
#if WITH_EDITOR
	return FActionToken::Create(
		INVTEXT("View Callstack"),
		INVTEXT("View callstack"),
		MakeLambdaDelegate([RootCallstack = Callstack, Title = Message->ToString()]
		{
			SVoxelCallstack::CreatePopup(
				Title,
				[RootCallstack]
				{
					TArray<TSharedPtr<FVoxelCallstackEntry>> Result;

					TVoxelArray<const FVoxelGraphSharedCallstack*> Callstacks;
					for (const FVoxelGraphSharedCallstack* Callstack = RootCallstack.Get(); Callstack; Callstack = Callstack->GetParent().Get())
					{
						const FVoxelGraphMergedNodeRef NodeRef = Callstack->GetNodeRef();
						if (NodeRef.Node.IsExplicitlyNull() ||
							Callstack->IsDebugNode() ||
							NodeRef.Node.bGeneratedNode)
						{
							continue;
						}

						if (Callstacks.Num() > 0 &&
							Callstacks.Last()->GetNodeRef() == NodeRef)
						{
							continue;
						}

						Callstacks.Add(Callstack);
					}
					int32 Num = Callstacks.Num();

					const UVoxelTerminalGraph* LastTerminalGraph = nullptr;

					TVoxelArray<FVoxelGraphMergedNodeRef> PendingNodes;

					const auto FlushNodes = [&]
					{
						if (PendingNodes.Num() == 0)
						{
							return;
						}

						FString GraphName = LastTerminalGraph->GetGraph().GetName();
						if (LastTerminalGraph->IsMainTerminalGraph())
						{
							// No suffix
						}
						else if (LastTerminalGraph->IsEditorTerminalGraph())
						{
							GraphName += ".Editor";
						}
						else
						{
							GraphName += "." + LastTerminalGraph->GetDisplayName();
						}

						const TSharedRef<FVoxelCallstackEntry> GraphEntry = MakeShared<FVoxelCallstackObjectEntry>(
							LastTerminalGraph,
							GraphName,
							"Graph: ",
							FVoxelCallstackEntry::EType::Subdued);
						Result.Add(GraphEntry);

						for (const FVoxelGraphMergedNodeRef& Node : PendingNodes)
						{
							TSharedRef<FVoxelCallstackEntry> NodeEntry = MakeShared<FVoxelCallstackNodeEntry>(
									Node,
									Node.ToString(),
									LexToString(Num) + ". ",
									Num == Callstacks.Num()
									? FVoxelCallstackEntry::EType::Marked
									: FVoxelCallstackEntry::EType::Default);
							GraphEntry->Children.Add(NodeEntry);
							Num--;
						}

						LastTerminalGraph = nullptr;
						PendingNodes.Reset();
					};

					for (const FVoxelGraphSharedCallstack* Callstack : Callstacks)
					{
						const UVoxelTerminalGraph* TerminalGraph = Callstack->GetNodeRef().Node.TerminalGraph.Resolve();
						if (!ensureVoxelSlow(TerminalGraph))
						{
							continue;
						}

						if (LastTerminalGraph != TerminalGraph)
						{
							FlushNodes();

							LastTerminalGraph = TerminalGraph;
						}

						PendingNodes.Add(Callstack->GetNodeRef());
					}

					FlushNodes();

					return Result;
				});
		}));
#else
	return Super::GetMessageToken();
#endif
}