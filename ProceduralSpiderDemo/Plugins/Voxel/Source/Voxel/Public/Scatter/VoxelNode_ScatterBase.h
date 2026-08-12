// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelPointSet.h"
#include "VoxelNode_ScatterBase.generated.h"

class FVoxelScatterNodeRuntime;

USTRUCT(Category = "Scatter", meta = (Abstract, AllowList = "Scatter", NodeColor = "Red"))
struct VOXEL_API FVoxelNode_ScatterBase : public FVoxelNode
{
	GENERATED_BODY()
	GENERATED_VOXEL_NODE_BODY()

public:
	// Debug name for this node
	VOXEL_INPUT_PIN(FName, Name, nullptr);
	// Chunk size, in meters
	VOXEL_INPUT_PIN(float, ChunkSize, 64.f);

public:
	//~ Begin FVoxelNode Interface
	virtual bool HasGuid() const override
	{
		return false;
	}
	virtual bool CanBeQueried() const override
	{
		return true;
	}
	//~ End FVoxelNode Interface

	virtual TSharedRef<FVoxelScatterNodeRuntime> MakeRuntime() const VOXEL_PURE_VIRTUAL(SharedRef_Null);
};