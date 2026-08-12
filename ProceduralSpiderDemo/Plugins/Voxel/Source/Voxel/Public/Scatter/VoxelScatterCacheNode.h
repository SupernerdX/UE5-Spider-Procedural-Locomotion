// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelPointSet.h"
#include "VoxelScatterCacheNode.generated.h"

USTRUCT(Category = "Point", meta = (AllowList = "Scatter"))
struct VOXEL_API FVoxelNode_CachePoints : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_INPUT_PIN(FVoxelPointSet, In, nullptr);
	// Cache chunk size, in meters.
	// When a stamp moves, the whole cache chunk it touches is regenerated, and every
	// render chunk overlapping that cache chunk is also redrawn - so larger values
	// mean more work per edit.
	// For runtime editing, prefer cache chunks roughly the same size as your render
	// chunks, or smaller. For static content, larger is fine.
	VOXEL_INPUT_PIN(float, ChunkSize, 64.f);
	// Cache size in MB
	VOXEL_INPUT_PIN(float, CacheSize, 1.f);
	VOXEL_INPUT_PIN(float, BoundsExtension, 0.f);
	VOXEL_OUTPUT_PIN(FVoxelPointSet, Out);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FName> AttributesToCache = { "Rotation" };
};