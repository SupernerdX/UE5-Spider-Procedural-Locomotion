// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelGradientNodes.generated.h"

// Returns the unnormalized gradient of a value
// Will make any node before this 4x more expensive, use with caution
USTRUCT(Category = "Gradient")
struct VOXELGRAPH_API FVoxelNode_GetGradient2D : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Value, nullptr);
	// Sets how granular the result will be: if very small, will return the gradient very close to the surface
	// If bigger will give a broader gradient for that area
	// Get Gradient computes a delta between values at X + Granularity, X - Granularity, Y + Granularity etc
	VOXEL_INPUT_PIN(float, Granularity, 100.f);
	// Should normalize gradient output
	VOXEL_INPUT_PIN(bool, Normalize, true);
	VOXEL_OUTPUT_PIN(FVoxelVector2DBuffer, Gradient);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

// Returns the unnormalized gradient of a value
// Will make any node before this 6x more expensive, use with caution
USTRUCT(Category = "Gradient")
struct VOXELGRAPH_API FVoxelNode_GetGradient3D : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Value, nullptr);
	// Sets how granular the result will be: if very small, will return the gradient very close to the surface
	// If bigger will give a broader gradient for that area
	// Get Gradient computes a delta between values at X + Granularity, X - Granularity, Y + Granularity etc
	VOXEL_INPUT_PIN(float, Granularity, 100.f);
	// Should normalize gradient output
	VOXEL_INPUT_PIN(bool, Normalize, true);
	VOXEL_OUTPUT_PIN(FVoxelVectorBuffer, Gradient);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};