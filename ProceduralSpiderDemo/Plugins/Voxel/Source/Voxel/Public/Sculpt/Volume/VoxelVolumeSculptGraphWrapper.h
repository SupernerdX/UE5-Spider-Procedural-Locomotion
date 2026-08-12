// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelParameterOverridesOwner.h"
#include "VoxelVolumeSculptGraphWrapper.generated.h"

class UVoxelVolumeSculptGraph;

USTRUCT(BlueprintType, meta = (HasNativeMake = "/Script/Voxel.VoxelVolumeSculptBlueprintLibrary.MakeVoxelVolumeSculptGraphWrapper"))
struct VOXEL_API FVoxelVolumeSculptGraphWrapper
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelVolumeSculptGraph> Graph;

	UPROPERTY()
	FVoxelParameterOverrides ParameterOverrides;

	bool IsValid() const
	{
		return Graph != nullptr;
	}
};