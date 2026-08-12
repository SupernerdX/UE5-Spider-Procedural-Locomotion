// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelDomainWarpNodes.generated.h"

USTRUCT(Category = "Noise")
struct VOXELGRAPH_API FVoxelNode_DomainWarp2D : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelVector2DBuffer, Position, nullptr, PositionPin);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Amplitude, 10000.f);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, FeatureScale, 100000.f);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Lacunarity, 2.f);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Gain, 0.5f);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, WeightedStrength, 0.f);
	VOXEL_INPUT_PIN(int32, NumOctaves, 10);
	VOXEL_INPUT_PIN(FVoxelSeed, Seed, nullptr);

	VOXEL_OUTPUT_PIN(FVoxelVector2DBuffer, Value);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};

USTRUCT(Category = "Noise")
struct VOXELGRAPH_API FVoxelNode_DomainWarp3D : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelVectorBuffer, Position, nullptr, PositionPin);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Amplitude, 10000.f);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, FeatureScale, 100000.f);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Lacunarity, 2.f);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, Gain, 0.5f);
	VOXEL_INPUT_PIN(FVoxelFloatBuffer, WeightedStrength, 0.f);
	VOXEL_INPUT_PIN(int32, NumOctaves, 10);
	VOXEL_INPUT_PIN(FVoxelSeed, Seed, nullptr);

	VOXEL_OUTPUT_PIN(FVoxelVectorBuffer, Value);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};