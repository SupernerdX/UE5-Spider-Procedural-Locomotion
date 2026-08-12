// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelTerminalBuffer.h"
#include "VoxelNameBuffer.generated.h"

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelNameBuffer, FVoxelNameWrapper);

USTRUCT(DisplayName = "Name Buffer")
struct VOXELGRAPH_API FVoxelNameBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelNameBuffer, FVoxelNameWrapper);
};