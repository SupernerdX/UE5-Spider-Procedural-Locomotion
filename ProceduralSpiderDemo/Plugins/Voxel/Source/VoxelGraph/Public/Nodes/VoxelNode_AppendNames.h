// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelNode_AppendNames.generated.h"

USTRUCT(Category = "Name", meta = (ShowInPinShortList))
struct VOXELGRAPH_API FVoxelNode_AppendNames : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	VOXEL_VARIADIC_INPUT_PIN(FName, Names, nullptr, 2);
	VOXEL_OUTPUT_PIN(FName, Name);

	//~ Begin FVoxelNode Interface
	virtual bool IsPureNode() const override
	{
		return true;
	}

	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};