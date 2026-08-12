// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class UVoxelTerminalGraph;
struct FOnVoxelGraphChanged;

struct VOXELGRAPH_API FVoxelGraphNodeRef
{
public:
	TVoxelObjectPtr<const UVoxelTerminalGraph> TerminalGraph;
	FName NodeId;
	int32 TemplateInstance = 0;
	bool bGeneratedNode = false;

	// Debug only
	FName EdGraphNodeTitle;
	// Debug only
	FName EdGraphNodeName;

public:
	FVoxelGraphNodeRef() = default;

	FVoxelGraphNodeRef(
		const UVoxelTerminalGraph& TerminalGraph,
		FName NodeId,
		FName EdGraphNodeTitle,
		FName EdGraphNodeName,
		bool bGeneratedNode = false);

#if WITH_EDITOR
	UEdGraphNode* GetGraphNode_EditorOnly() const;
#endif

	bool IsDeleted() const;
	void AppendString(FWideStringBuilderBase& Out) const;
	FVoxelGraphNodeRef WithSuffix(const FString& Suffix) const;
	TSharedRef<FVoxelMessageToken> CreateMessageToken() const;

public:
	FORCEINLINE bool IsExplicitlyNull() const
	{
		return
			TerminalGraph.IsExplicitlyNull() &&
			*this == FVoxelGraphNodeRef();
	}

	FORCEINLINE bool operator==(const FVoxelGraphNodeRef& Other) const
	{
		return
			TerminalGraph == Other.TerminalGraph &&
			NodeId == Other.NodeId &&
			TemplateInstance == Other.TemplateInstance;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelGraphNodeRef& Key)
	{
		return FVoxelUtilities::MurmurHashMulti(
			GetTypeHash(Key.TerminalGraph),
			GetTypeHash(Key.NodeId),
			GetTypeHash(Key.TemplateInstance));
	}
};

struct VOXELGRAPH_API FVoxelGraphMergedNodeRef
{
public:
	FVoxelGraphMergedNodeRef() = default;

	FVoxelGraphMergedNodeRef(
		const FVoxelGraphNodeRef& NodeRef,
		const TVoxelArray<FVoxelGraphNodeRef>& MergedNodeRefs)
		: Node(NodeRef)
	{
		TVoxelSet<FVoxelGraphNodeRef> Nodes(MergedNodeRefs);
		for (const FVoxelGraphNodeRef& OtherNode : Nodes)
		{
			if (Node == OtherNode)
			{
				continue;
			}

			MergedNodes.Add(OtherNode);
		}
	}

public:
	TSharedRef<FVoxelMessageToken> CreateMessageToken() const;

	FString ToString() const;

	FORCEINLINE bool operator==(const FVoxelGraphMergedNodeRef& Other) const
	{
		return
			Node == Other.Node &&
			MergedNodes == Other.MergedNodes;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelGraphMergedNodeRef& Key)
	{
		return FVoxelUtilities::MurmurHashMulti(
			GetTypeHash(Key.Node),
			GetTypeHash(Key.MergedNodes));
	}

public:
	FVoxelGraphNodeRef Node;
	TVoxelArray<FVoxelGraphNodeRef> MergedNodes;
};

struct VOXELGRAPH_API FVoxelGraphPinRef
{
public:
	FVoxelGraphNodeRef NodeRef;
	FName PinName;

public:
	FString ToString() const;
	TSharedRef<FVoxelMessageToken> CreateMessageToken() const;

	FORCEINLINE bool operator==(const FVoxelGraphPinRef& Other) const
	{
		return
			NodeRef == Other.NodeRef &&
			PinName == Other.PinName;
	}

	FORCEINLINE friend uint32 GetTypeHash(const FVoxelGraphPinRef& Key)
	{
		return FVoxelUtilities::MurmurHashMulti(
			GetTypeHash(Key.NodeRef),
			GetTypeHash(Key.PinName));
	}
};