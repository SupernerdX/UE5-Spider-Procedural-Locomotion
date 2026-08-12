// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterManager.h"
#include "VoxelInvalidationCallstack.h"

void FVoxelScatterManager::AddNode(
	const FVoxelScatterNodeWeakRef& NodeRef,
	const TVoxelNodeEvaluator<FVoxelNode_ScatterBase>& Evaluator)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());

	NodeRefToEvaluator.Add_EnsureNew(NodeRef, Evaluator);

	FVoxelInvalidationScope Scope("FVoxelScatterManager::UpdateNode");
	Dependency->Invalidate();
}

void FVoxelScatterManager::RemoveNode(const FVoxelScatterNodeWeakRef& NodeRef)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());

	ensure(NodeRefToEvaluator.Remove(NodeRef));

	FVoxelInvalidationScope Scope("FVoxelScatterManager::RemoveNode");
	Dependency->Invalidate();
}

const TVoxelMap<FVoxelScatterNodeWeakRef, TVoxelNodeEvaluator<FVoxelNode_ScatterBase>>& FVoxelScatterManager::GetNodeRefToEvaluator(FVoxelDependencyCollector& DependencyCollector) const
{
	ensure(IsInGameThread());
	DependencyCollector.AddDependency(*Dependency);
	return NodeRefToEvaluator;
}