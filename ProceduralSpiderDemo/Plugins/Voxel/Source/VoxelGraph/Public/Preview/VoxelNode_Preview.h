// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelNode_Preview.generated.h"

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_Preview : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(FVoxelWildcard, Value, nullptr);

public:
	//~ Begin FVoxelNode Interface
	virtual bool CanBeQueried() const override
	{
		return true;
	}
	//~ End FVoxelNode Interface
};