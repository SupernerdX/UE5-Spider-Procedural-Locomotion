// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodeEvaluator.h"
#include "Scatter/VoxelScatterNodeRef.h"

class AVoxelScatterActor;
struct FVoxelNode_ScatterBase;

class VOXEL_API FVoxelScatterActorRuntime
{
public:
	static TSharedPtr<FVoxelScatterActorRuntime> Create(AVoxelScatterActor& Actor);
	~FVoxelScatterActorRuntime();

	void Update() const;
	void Destroy();

private:
	const TVoxelObjectPtr<UWorld> WeakWorld;
	const TVoxelObjectPtr<AVoxelScatterActor> WeakActor;
	const TVoxelArray<TVoxelNodeEvaluator<FVoxelNode_ScatterBase>> Evaluators;
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;
	TVoxelArray<FVoxelScatterNodeWeakRef> NodeRefs;

	FVoxelScatterActorRuntime(
		AVoxelScatterActor& Actor,
		TVoxelArray<TVoxelNodeEvaluator<FVoxelNode_ScatterBase>>&& Evaluators);

	void Initialize();
};