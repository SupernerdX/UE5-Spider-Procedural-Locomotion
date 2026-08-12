// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelNode_CustomizeParameter.generated.h"

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_CustomizeParameter : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_INPUT_PIN(bool, IsVisible, true);
	VOXEL_INPUT_PIN(bool, IsReadOnly, false);
	VOXEL_INPUT_PIN(FName, DisplayName, nullptr);

public:
	UPROPERTY()
	FGuid ParameterGuid;

	UPROPERTY()
	FName ParameterName;

public:
	//~ Begin FVoxelNode Interface
	virtual bool CanBeQueried() const override
	{
		return true;
	}
	//~ End FVoxelNode Interface
};