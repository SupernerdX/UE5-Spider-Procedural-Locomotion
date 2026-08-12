// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelDependency.h"
#include "VoxelNodeEvaluator.h"
#include "Scatter/VoxelScatterNodeRef.h"

struct FVoxelNode_ScatterBase;

class VOXEL_API FVoxelScatterManager : public IVoxelWorldSubsystem
{
public:
	GENERATED_VOXEL_WORLD_SUBSYSTEM_BODY(FVoxelScatterManager);

	void AddNode(
		const FVoxelScatterNodeWeakRef& NodeRef,
		const TVoxelNodeEvaluator<FVoxelNode_ScatterBase>& Evaluator);

	void RemoveNode(const FVoxelScatterNodeWeakRef& NodeRef);

	const TVoxelMap<FVoxelScatterNodeWeakRef, TVoxelNodeEvaluator<FVoxelNode_ScatterBase>>& GetNodeRefToEvaluator(FVoxelDependencyCollector& DependencyCollector) const;

private:
	const TSharedRef<FVoxelDependency> Dependency = FVoxelDependency::Create("FVoxelScatterManager");

	TVoxelMap<FVoxelScatterNodeWeakRef, TVoxelNodeEvaluator<FVoxelNode_ScatterBase>> NodeRefToEvaluator;
};