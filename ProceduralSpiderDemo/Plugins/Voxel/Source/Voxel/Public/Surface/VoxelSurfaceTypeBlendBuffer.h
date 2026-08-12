// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSurfaceTypeBlend.h"
#include "VoxelObjectPinType.h"
#include "VoxelTerminalBuffer.h"
#include "Surface/VoxelSurfaceTypeInterface.h"
#include "VoxelSurfaceTypeBlendBuffer.generated.h"

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelSurfaceTypeBlend);

USTRUCT()
struct VOXEL_API FVoxelSurfaceTypeBlendPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelSurfaceTypeBlend, UVoxelSurfaceTypeInterface);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelSurfaceTypeBlendBuffer, FVoxelSurfaceTypeBlend);

USTRUCT()
struct VOXEL_API FVoxelSurfaceTypeBlendBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelSurfaceTypeBlendBuffer, FVoxelSurfaceTypeBlend);

	// Override equality functions to be faster & because not all bytes might be initialized

	//~ Begin FVoxelTerminalBuffer Interface
	virtual void AllocateZeroed(int32 NewNum) override;
	virtual bool Equal(const FVoxelBuffer& Other) const override;
	virtual void BulkEqual(const FVoxelBuffer& Other, TVoxelArrayView<bool> Result, EBulkEqualFlags Flags) const override;
	//~ End FVoxelTerminalBuffer Interface
};