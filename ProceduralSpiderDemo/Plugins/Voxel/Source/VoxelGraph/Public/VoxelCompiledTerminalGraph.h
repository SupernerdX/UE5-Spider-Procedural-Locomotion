// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelNode;

namespace Voxel::Graph
{
	class FGraph;
}

class VOXELGRAPH_API FVoxelCompiledTerminalGraph
{
public:
	const FGuid TerminalGraphGuid;
	const int32 NumNodes;
	const int32 NumPins;

	FORCEINLINE bool OwnsNode(const FVoxelNode* Node) const
	{
		return OwnedNodes.Contains(Node);
	}

	template<typename T>
	requires std::derived_from<T, FVoxelNode>
	FORCEINLINE TConstVoxelArrayView<const T*> GetNodes() const
	{
#if VOXEL_DEBUG
		static const int32 _ = INLINE_LAMBDA
		{
			check(T().CanBeQueried());
			return 0;
		};
#endif

		const TVoxelArray<const FVoxelNode*>* Nodes = StructToNodes.Find(StaticStructFast<T>());
		if (!Nodes)
		{
			return {};
		}

		return Nodes->View<const T*>();
	}

	UE_NONCOPYABLE(FVoxelCompiledTerminalGraph);

private:
#if WITH_EDITOR
	TSharedPtr<const Voxel::Graph::FGraph> Graph_DiffingOnly;
#endif

	const TVoxelArray<TSharedPtr<FVoxelNode>> NodeRefs;
	const TVoxelSet<const FVoxelNode*> OwnedNodes;
	const TVoxelMap<UScriptStruct*, TVoxelArray<const FVoxelNode*>> StructToNodes;
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;

	FVoxelCompiledTerminalGraph(
		const FGuid TerminalGraphGuid,
		const int32 NumNodes,
		const int32 NumPins,
#if WITH_EDITOR
		const TSharedRef<const Voxel::Graph::FGraph>& Graph_DiffingOnly,
#endif
		TVoxelArray<TSharedPtr<FVoxelNode>>&& NodeRefs,
		TVoxelSet<const FVoxelNode*>&& OwnedNodes,
		TVoxelMap<UScriptStruct*, TVoxelArray<const FVoxelNode*>>&& StructToNodes)
		: TerminalGraphGuid(TerminalGraphGuid)
		, NumNodes(NumNodes)
		, NumPins(NumPins)
#if WITH_EDITOR
		, Graph_DiffingOnly(Graph_DiffingOnly)
#endif
		, NodeRefs(MoveTemp(NodeRefs))
		, OwnedNodes(MoveTemp(OwnedNodes))
		, StructToNodes(MoveTemp(StructToNodes))
	{
	}

	friend class UVoxelTerminalGraphRuntime;
	friend struct FVoxelCompiledTerminalGraphRef;
};