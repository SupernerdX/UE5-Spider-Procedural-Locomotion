// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelNode_Parameter.generated.h"

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_Parameter : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

	UPROPERTY()
	FGuid ParameterGuid;

	UPROPERTY()
	FName ParameterName;

	VOXEL_OUTPUT_PIN(FVoxelWildcard, Value);

	//~ Begin FVoxelNode Interface
	virtual void Compute(FVoxelGraphQuery Query) const override;
	//~ End FVoxelNode Interface
};