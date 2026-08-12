// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterNodeRuntime.h"
#include "Scatter/VoxelNode_ScatterBase.h"
#include "VoxelSubsystem.h"
#include "VoxelGraphContext.h"

void FVoxelScatterNodeRuntime::Initialize(
	const FVoxelSubsystem& Subsystem,
	const FVoxelScatterNodeWeakRef& NodeRef,
	const TVoxelNodeEvaluator<FVoxelNode_ScatterBase>& Evaluator)
{
	VOXEL_FUNCTION_COUNTER();

	PrivateNodeRef = NodeRef;
	PrivateEvaluator = Evaluator;

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelScatterNodeRuntime"));

	FVoxelGraphContext Context = PrivateEvaluator.MakeContext(DependencyCollector);
	FVoxelGraphQueryImpl& Query = Context.MakeQuery();

	PrivateName = PrivateEvaluator->NamePin.GetSynchronous(Query);
	PrivateChunkSize = PrivateEvaluator->ChunkSizePin.GetSynchronous(Query) * 100.f;

	Initialize(Query);

	DependencyTracker = Subsystem.Finalize(DependencyCollector);
}