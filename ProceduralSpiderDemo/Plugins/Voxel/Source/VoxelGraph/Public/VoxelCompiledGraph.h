// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class UVoxelGraph;
class FVoxelCompiledTerminalGraph;

class VOXELGRAPH_API FVoxelCompiledGraph
{
public:
	const TVoxelObjectPtr<const UVoxelGraph> Graph;

	FORCEINLINE const FVoxelCompiledTerminalGraph* FindTerminalGraph(const FGuid& Guid) const
	{
		return GuidToTerminalGraph.FindRef(Guid);
	}

	UE_NONCOPYABLE(FVoxelCompiledGraph);

private:
	const TVoxelArray<TSharedPtr<FVoxelCompiledTerminalGraph>> TerminalGraphRefs;
	const TVoxelMap<FGuid, const FVoxelCompiledTerminalGraph*> GuidToTerminalGraph;

	FVoxelCompiledGraph(
		TVoxelObjectPtr<const UVoxelGraph> Graph,
		TVoxelArray<TSharedPtr<FVoxelCompiledTerminalGraph>>&& TerminalGraphRefs,
		TVoxelMap<FGuid, const FVoxelCompiledTerminalGraph*>&& GuidToTerminalGraph);

	friend class UVoxelGraph;
};