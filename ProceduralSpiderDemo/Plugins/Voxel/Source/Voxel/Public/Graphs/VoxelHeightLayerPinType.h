// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLayer.h"
#include "VoxelObjectPinType.h"
#include "VoxelTerminalBuffer.h"
#include "VoxelHeightLayerPinType.generated.h"

USTRUCT(DisplayName = "Voxel Height Layer")
struct VOXEL_API FVoxelHeightLayerObject
{
	GENERATED_BODY()

	TVoxelObjectPtr<UVoxelHeightLayer> Layer;
};

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelHeightLayerObject);

USTRUCT()
struct VOXEL_API FVoxelHeightLayerObjectPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelHeightLayerObject, UVoxelHeightLayer)
	{
		if (bSetObject)
		{
			OutObject = Struct.Layer;
		}
		else
		{
			Struct = FVoxelHeightLayerObject{ &InObject };
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelHeightLayerObjectBuffer, FVoxelHeightLayerObject);

USTRUCT(DisplayName = "Voxel Height Layer Buffer")
struct VOXEL_API FVoxelHeightLayerObjectBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelHeightLayerObjectBuffer, FVoxelHeightLayerObject);
};