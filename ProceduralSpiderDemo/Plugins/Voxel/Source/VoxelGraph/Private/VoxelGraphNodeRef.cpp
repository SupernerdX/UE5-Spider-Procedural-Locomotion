// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphNodeRef.h"
#include "VoxelTerminalGraph.h"
#include "VoxelTerminalGraphRuntime.h"
#include "VoxelGraphMessageTokens.h"
#if WITH_EDITOR
#include "EdGraph/EdGraph.h"
#endif

FVoxelGraphNodeRef::FVoxelGraphNodeRef(
	const UVoxelTerminalGraph& TerminalGraph,
	const FName NodeId,
	const FName EdGraphNodeTitle,
	const FName EdGraphNodeName,
	const bool bGeneratedNode)
	: TerminalGraph(&TerminalGraph)
	, NodeId(NodeId)
	, bGeneratedNode(bGeneratedNode)
	, EdGraphNodeTitle(EdGraphNodeTitle)
	, EdGraphNodeName(EdGraphNodeName)
{
}

#if WITH_EDITOR
UEdGraphNode* FVoxelGraphNodeRef::GetGraphNode_EditorOnly() const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());

	if (EdGraphNodeName.IsNone())
	{
		return nullptr;
	}

	const UVoxelTerminalGraph* TerminalGraphObject = TerminalGraph.Resolve();
	if (!TerminalGraphObject)
	{
		return nullptr;
	}

	for (UEdGraphNode* Node : TerminalGraphObject->GetEdGraph().Nodes)
	{
		if (ensure(Node) &&
			Node->GetFName() == EdGraphNodeName)
		{
			return Node;
		}
	}

	ensureVoxelSlow(false);
	return nullptr;
}
#endif

bool FVoxelGraphNodeRef::IsDeleted() const
{
	if (EdGraphNodeName.IsNone())
	{
		return false;
	}

	const UVoxelTerminalGraph* TerminalGraphObject = TerminalGraph.Resolve();
	if (!ensure(TerminalGraphObject))
	{
		return false;
	}

	return !TerminalGraphObject->GetRuntime().GetSerializedGraph().NodeNameToNode.Contains(EdGraphNodeName);
}

void FVoxelGraphNodeRef::AppendString(FWideStringBuilderBase& Out) const
{
	if (!EdGraphNodeTitle.IsNone())
	{
		EdGraphNodeTitle.AppendString(Out);
	}
	else
	{
		NodeId.AppendString(Out);
	}
}

FVoxelGraphNodeRef FVoxelGraphNodeRef::WithSuffix(const FString& Suffix) const
{
	FVoxelGraphNodeRef Result = *this;
	Result.NodeId += "_" + Suffix;
	Result.EdGraphNodeTitle += " (" + Suffix + ")";
	return Result;
}

TSharedRef<FVoxelMessageToken> FVoxelGraphNodeRef::CreateMessageToken() const
{
	const TSharedRef<FVoxelMessageToken_NodeRef> Result = MakeShared<FVoxelMessageToken_NodeRef>();
	Result->NodeRef = *this;
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelMessageToken> FVoxelGraphMergedNodeRef::CreateMessageToken() const
{
	const TSharedRef<FVoxelMessageToken_MergedNodeRef> Result = MakeShared<FVoxelMessageToken_MergedNodeRef>();
	Result->NodeRef = *this;
	return Result;
}

FString FVoxelGraphMergedNodeRef::ToString() const
{
	FString Title = Node.EdGraphNodeTitle.ToString();
	if (MergedNodes.Num() > 0)
	{
		Title += " [Merged " + LexToString(MergedNodes.Num() + 1) + " nodes]";
	}
	return Title;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FString FVoxelGraphPinRef::ToString() const
{
	ensure(IsInGameThread());

	return FString::Printf(TEXT("%s.%s.%s"),
		*NodeRef.TerminalGraph.GetPathName(),
		*NodeRef.EdGraphNodeTitle.ToString(),
		*PinName.ToString());
}

TSharedRef<FVoxelMessageToken> FVoxelGraphPinRef::CreateMessageToken() const
{
	const TSharedRef<FVoxelMessageToken_PinRef> Result = MakeShared<FVoxelMessageToken_PinRef>();
	Result->PinRef = *this;
	return Result;
}