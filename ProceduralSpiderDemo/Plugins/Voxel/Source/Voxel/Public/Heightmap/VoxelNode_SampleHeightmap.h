// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelHeightmapFunctionLibrary.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Graphs/VoxelSampleStampNodes.h"
#include "VoxelNode_SampleHeightmap.generated.h"

// Sample a heightmap asset
// Will return NaN if Position is outside the heightmap bounds
USTRUCT(Category = "Heightmap")
struct VOXEL_API FVoxelNode_SampleHeightmap : public FVoxelNode_MetadataPinsBase
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	// The heightmap to sample
	VOXEL_INPUT_PIN(FVoxelHeightmapRef, Heightmap, nullptr);
	// Position, note that the heightmap ScaleXY is applied
	VOXEL_INPUT_PIN(FVoxelVector2DBuffer, Position, nullptr, PositionPin);
	// If true will use bicubic sampling, which is slower but better looking
	VOXEL_INPUT_PIN(bool, UseBicubic, true);

	VOXEL_OUTPUT_PIN(FVoxelFloatBuffer, Height);
	VOXEL_OUTPUT_PIN(FVoxelSurfaceTypeBlendBuffer, SurfaceType);

public:
	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};