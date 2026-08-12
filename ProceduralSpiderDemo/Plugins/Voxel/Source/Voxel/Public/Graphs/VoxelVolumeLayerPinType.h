// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLayer.h"
#include "VoxelObjectPinType.h"
#include "VoxelTerminalBuffer.h"
#include "VoxelVolumeLayerPinType.generated.h"

USTRUCT(DisplayName = "Voxel Volume Layer")
struct VOXEL_API FVoxelVolumeLayerObject
{
	GENERATED_BODY()

	TVoxelObjectPtr<UVoxelVolumeLayer> Layer;
};

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelVolumeLayerObject);

USTRUCT()
struct VOXEL_API FVoxelVolumeLayerObjectPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelVolumeLayerObject, UVoxelVolumeLayer)
	{
		if (bSetObject)
		{
			OutObject = Struct.Layer;
		}
		else
		{
			Struct = FVoxelVolumeLayerObject{ &InObject };
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_TERMINAL_BUFFER(FVoxelVolumeLayerObjectBuffer, FVoxelVolumeLayerObject);

USTRUCT(DisplayName = "Voxel Volume Layer Buffer")
struct VOXEL_API FVoxelVolumeLayerObjectBuffer final : public FVoxelTerminalBuffer
{
	GENERATED_BODY()
	GENERATED_VOXEL_TERMINAL_BUFFER_BODY(FVoxelVolumeLayerObjectBuffer, FVoxelVolumeLayerObject);
};