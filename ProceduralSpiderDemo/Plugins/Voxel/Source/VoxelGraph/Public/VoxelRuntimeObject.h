// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelObjectPinType.h"
#include "VoxelRuntimeObject.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelRuntimeObject
{
	GENERATED_BODY()

	TVoxelObjectPtr<UObject> Object;
};

DECLARE_VOXEL_OBJECT_PIN_TYPE(FVoxelRuntimeObject);

USTRUCT()
struct VOXELGRAPH_API FVoxelRuntimeObjectPinType : public FVoxelObjectPinType
{
	GENERATED_BODY()

	DEFINE_VOXEL_OBJECT_PIN_TYPE(FVoxelRuntimeObject, UObject)
	{
		if (bSetObject)
		{
			OutObject = Struct.Object;
		}
		else
		{
			Struct.Object = InObject;
		}
	}
};