// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelCompiledGraph.h"
#include "VoxelGraph.h"

FVoxelCompiledGraph::FVoxelCompiledGraph(
	const TVoxelObjectPtr<const UVoxelGraph> Graph,
	TVoxelArray<TSharedPtr<FVoxelCompiledTerminalGraph>>&& TerminalGraphRefs,
	TVoxelMap<FGuid, const FVoxelCompiledTerminalGraph*>&& GuidToTerminalGraph)
	: Graph(Graph)
	, TerminalGraphRefs(MoveTemp(TerminalGraphRefs))
	, GuidToTerminalGraph(MoveTemp(GuidToTerminalGraph))
{
}