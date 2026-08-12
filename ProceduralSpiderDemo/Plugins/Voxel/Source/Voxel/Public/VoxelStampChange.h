// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelStampRuntime;

struct FVoxelStampChange
{
	TSharedPtr<const FVoxelStampRuntime> OldStamp;
	TSharedPtr<const FVoxelStampRuntime> NewStamp;
};