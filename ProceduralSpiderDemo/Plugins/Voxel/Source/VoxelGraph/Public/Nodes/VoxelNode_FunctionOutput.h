// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelNode_FunctionOutput.generated.h"

USTRUCT(meta = (Internal))
struct VOXELGRAPH_API FVoxelNode_FunctionOutput : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	VOXEL_TEMPLATE_INPUT_PIN(FVoxelWildcard, Value, nullptr);

public:
	UPROPERTY()
	FGuid Guid;

public:
	//~ Begin FVoxelNode Interface
	virtual bool CanBeQueried() const override
	{
		return true;
	}

#if WITH_EDITOR
	virtual FVoxelPinTypeSet GetPromotionTypes(const FVoxelPin& Pin) const override
	{
		return FVoxelPinTypeSet::All();
	}
#endif
	//~ End FVoxelNode Interface
};