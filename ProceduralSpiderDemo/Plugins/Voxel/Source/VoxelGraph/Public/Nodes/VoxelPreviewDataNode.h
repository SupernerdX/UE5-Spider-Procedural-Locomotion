// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelPreviewDataNode.generated.h"

USTRUCT(meta = (Abstract, NodeColor = "Red", NodeIconColor = "LightRed", Category = "Preview", EditorGraphOnly))
struct VOXELGRAPH_API FVoxelPreviewDataNode : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	//~ Begin FVoxelNode Interface
	virtual bool CanBeQueried() const override
	{
		return true;
	}
	//~ End FVoxelNode Interface

public:
	virtual void Query_WithPosition(
		const FVoxelGraphQueryImpl& Query,
		FVoxelGraphQueryImpl& OutQuery) const
	{
	}
};