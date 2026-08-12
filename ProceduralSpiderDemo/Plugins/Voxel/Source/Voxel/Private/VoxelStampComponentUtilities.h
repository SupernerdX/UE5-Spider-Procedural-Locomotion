// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelStampRuntime;

struct FVoxelStampComponentUtilities
{
	static bool ShouldRender(const USceneComponent* Component);
	static FVoxelBox GetLocalBounds(const FVoxelStampRuntime& Stamp);
	static void DispatchBeginPlay(const UWorld* World);
};