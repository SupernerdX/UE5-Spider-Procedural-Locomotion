// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNodeEvaluator.h"
#include "VoxelGraph.h"
#include "VoxelGraphContext.h"
#include "VoxelGraphEnvironment.h"
#include "VoxelTerminalGraph.h"
#include "VoxelCompiledGraph.h"
#include "VoxelCompiledTerminalGraph.h"
#include "Nodes/VoxelOutputNode.h"

FVoxelGraphContext FVoxelNodeEvaluator::MakeContext(FVoxelDependencyCollector& DependencyCollector) const
{
	check(IsValid());

	return FVoxelGraphContext(
		*Environment,
		*TerminalGraph,
		DependencyCollector);
}

FVoxelNodeEvaluator FVoxelNodeEvaluator::Create(
	const UScriptStruct* Struct,
	const TSharedRef<const FVoxelGraphEnvironment>& Environment,
	const UVoxelTerminalGraph* TerminalGraph,
	const FVoxelNode* Node)
{
	VOXEL_FUNCTION_COUNTER()

	const UVoxelGraph* Graph = Environment->RootCompiledGraph->Graph.Resolve();
	if (!ensure(Graph))
	{
		return {};
	}

	if (!TerminalGraph)
	{
		TerminalGraph = Graph->GetMainTerminalGraph_CheckBaseGraphs();
	}
	if (!ensure(TerminalGraph))
	{
		return {};
	}

	const FVoxelCompiledTerminalGraph* CompiledTerminalGraph = Environment->RootCompiledGraph->FindTerminalGraph(TerminalGraph->GetGuid());
	if (!CompiledTerminalGraph)
	{
		VOXEL_MESSAGE(Error, "Failed to compile {0}", TerminalGraph);
		return {};
	}

	if (!Node)
	{
		if (!ensure(Struct->IsChildOf(StaticStructFast<FVoxelOutputNode>())))
		{
			return {};
		}

		const TConstVoxelArrayView<const FVoxelOutputNode*> Nodes = CompiledTerminalGraph->GetNodes<FVoxelOutputNode>();
		if (Nodes.Num() == 0)
		{
			VOXEL_MESSAGE(Error, "{0} is missing an output node of type {1}",
				TerminalGraph,
				Struct->GetName());

			return {};
		}

		if (Nodes.Num() > 1)
		{
			VOXEL_MESSAGE(Error, "{0} has more than one output node: {1}",
				TerminalGraph,
				Nodes);

			return {};
		}

		Node = Nodes[0];

		if (!Node->IsA(Struct))
		{
			VOXEL_MESSAGE(Error, "{0} has an output node of type {1}, but {2} was expected",
				TerminalGraph,
				Node->GetStruct()->GetName(),
				Struct->GetName());

			return {};
		}
	}

	if (!ensure(Node->IsA(Struct)))
	{
		return {};
	}

	FVoxelNodeEvaluator Result;
	Result.Node = Node;
	Result.Environment = Environment;
	Result.TerminalGraph = CompiledTerminalGraph;
	return Result;
}