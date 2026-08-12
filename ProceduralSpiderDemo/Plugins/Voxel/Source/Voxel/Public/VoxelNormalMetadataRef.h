// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "VoxelNormalMetadataRef.generated.h"

USTRUCT()
struct VOXEL_API FVoxelNormalMetadataRef : public FVoxelMetadataRef
{
	GENERATED_BODY()
	GENERATED_VOXEL_METADATA_REF_BODY(FVoxelNormalMetadataRef, UVoxelNormalMetadata, FVoxelNormalBuffer);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelNormalMetadataRefBuffer, FVoxelNormalMetadataRef);

USTRUCT()
struct VOXEL_API FVoxelNormalMetadataRefBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelNormalMetadataRefBuffer, FVoxelNormalMetadataRef);
};