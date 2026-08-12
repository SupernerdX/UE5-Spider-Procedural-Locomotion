// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "VoxelFloatMetadataRef.generated.h"

USTRUCT()
struct VOXEL_API FVoxelFloatMetadataRef : public FVoxelMetadataRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_METADATA_REF_BODY(FVoxelFloatMetadataRef, UVoxelFloatMetadata, FVoxelFloatBuffer);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelFloatMetadataRefBuffer, FVoxelFloatMetadataRef);

USTRUCT()
struct VOXEL_API FVoxelFloatMetadataRefBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelFloatMetadataRefBuffer, FVoxelFloatMetadataRef);
};