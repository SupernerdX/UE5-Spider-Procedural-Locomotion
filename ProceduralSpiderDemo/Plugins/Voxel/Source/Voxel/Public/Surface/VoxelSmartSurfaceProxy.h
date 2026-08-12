// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodeEvaluator.h"

struct FVoxelOutputNode_OutputSurface;

class VOXEL_API FVoxelSmartSurfaceProxy
{
public:
	const FName Name;
	const TSharedRef<FVoxelDependency> Dependency;
	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSurface> Evaluator;

private:
	const TSharedRef<FVoxelDependencyTracker> DependencyTracker;

	FVoxelSmartSurfaceProxy(
		const FName Name,
		const TSharedRef<FVoxelDependency>& Dependency,
		const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSurface>& Evaluator,
		const TSharedRef<FVoxelDependencyTracker>& DependencyTracker)
		: Name(Name)
		, Dependency(Dependency)
		, Evaluator(Evaluator)
		, DependencyTracker(DependencyTracker)
	{
	}

	friend class FVoxelSurfaceTypeTableManager;
};