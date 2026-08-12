// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelComputeNode.h"

struct VOXELGRAPH_API FVoxelComputeNodeImpl
{
	static int32 GetRegisterWidth(const FVoxelPinType& Type);
	static FVoxelPinType GetRegisterType(const FVoxelPinType& Type);

	static void FormatCode(FString& Code);
	static FString GenerateFunction(const FVoxelComputeNode& Node, TSet<FString>& OutIncludes);
};