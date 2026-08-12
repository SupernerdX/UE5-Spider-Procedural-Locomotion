// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelStackLayer.h"
#include "VoxelPointSet.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "VoxelNode_RaymarchLayerDistanceField.generated.h"

class FVoxelQuery;

USTRUCT(Category = "Layers", meta = (AllowList = "PCG, Scatter"))
struct VOXELPCG_API FVoxelNode_RaymarchLayerDistanceField : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	// Layer to sample
	VOXEL_INPUT_PIN(FVoxelWeakStackLayer, Layer, nullptr);
	// If true, will set the new point rotation to match the distance field gradient at the new position
	VOXEL_INPUT_PIN(bool, UpdateRotation, false);
	// If true, will look for a new surface in the point downward directly, according to its rotation
	// Ignored if sampling height layer
	VOXEL_INPUT_PIN(bool, ForceDirection, true);
	// If a point is further away than this distance from the surface
	// at the end of the raymarching, its density will be set to 0
	// Ignored if sampling height layer
	VOXEL_INPUT_PIN(float, KillDistance, 1000.f);
	// If distance to surface is less than this, stop the raymarching
	// Ignored if sampling height layer
	VOXEL_INPUT_PIN(float, Tolerance, 10.f, AdvancedDisplay);
	// Max number of steps to do during raymarching
	// Ignored if sampling height layer
	VOXEL_INPUT_PIN(int32, MaxSteps, 10, AdvancedDisplay);
	// How "fast" to converge to the surface, between 0 and 1
	// NewPoint = OldPoint + DistanceToSurface * Direction * Speed
	// Decrease if the raymarching is imprecise
	// Ignored if sampling height layer
	VOXEL_INPUT_PIN(float, Speed, 0.8f, AdvancedDisplay);
	// Distance between points when sampling gradients
	VOXEL_INPUT_PIN(float, GradientStep, 100.f, AdvancedDisplay);

	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface

private:
	static TSharedRef<FVoxelPointSet> Project2D(
		const FVoxelPointSet& Points,
		FVoxelDoubleVectorBuffer Positions,
		const FVoxelQuery& Query,
		const FVoxelWeakStackLayer& WeakLayer);

	static TSharedRef<FVoxelPointSet> Project2DWithRotation(
		const FVoxelPointSet& Points,
		const FVoxelDoubleVectorBuffer& PositionBuffer,
		const FVoxelQuaternionBuffer& RotationBuffer,
		const FVoxelQuery& Query,
		const FVoxelWeakStackLayer& WeakLayer,
		float GradientStep);

	static TSharedRef<FVoxelPointSet> Project3D(
		const FVoxelPointSet& Points,
		const FVoxelDoubleVectorBuffer& PositionBuffer,
		const FVoxelQuaternionBuffer& RotationBuffer,
		const FVoxelQuery& Query,
		const FVoxelWeakStackLayer& WeakLayer,
		bool bUpdateRotation,
		bool bForceDirection,
		float KillDistance,
		float Tolerance,
		int32 MaxSteps,
		float Speed,
		float GradientStep);
};