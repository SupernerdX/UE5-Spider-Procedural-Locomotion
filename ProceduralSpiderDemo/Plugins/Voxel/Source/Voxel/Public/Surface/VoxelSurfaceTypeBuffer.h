// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSurfaceType.h"
#include "VoxelObjectPinType.h"
#include "VoxelTerminalBuffer.h"
#include "Surface/VoxelSurfaceTypeInterface.h"
#include "VoxelSurfaceTypeBuffer.generated.h"

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelSurfaceType);

USTRUCT()
struct VOXEL_API FVoxelSurfaceTypePinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelSurfaceType, UVoxelSurfaceTypeInterface);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelSurfaceTypeBuffer, FVoxelSurfaceType);

USTRUCT()
struct VOXEL_API FVoxelSurfaceTypeBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelSurfaceTypeBuffer, FVoxelSurfaceType);
};