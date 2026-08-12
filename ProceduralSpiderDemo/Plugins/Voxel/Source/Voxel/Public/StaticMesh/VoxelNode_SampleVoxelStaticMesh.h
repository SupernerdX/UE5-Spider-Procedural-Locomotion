// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelStaticMeshFunctionLibrary.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Graphs/VoxelSampleStampNodes.h"
#include "VoxelNode_SampleVoxelStaticMesh.generated.h"

// Get the distance to a voxel mesh asset
USTRUCT(Category = "Static Mesh")
struct VOXEL_API FVoxelNode_SampleVoxelStaticMesh : public FVoxelNode_MetadataPinsBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	// The mesh to sample
	VOXEL_INPUT_PIN(FVoxelStaticMeshRef, Mesh, nullptr);
	VOXEL_INPUT_PIN(FVoxelVectorBuffer, Position, nullptr, PositionPin);
	VOXEL_INPUT_PIN(float, Scale, 1.f);
	// If true will use tricubic sampling, which is slower but better looking
	VOXEL_INPUT_PIN(bool, UseTricubic, true);

	VOXEL_OUTPUT_PIN(FVoxelFloatBuffer, Distance);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};