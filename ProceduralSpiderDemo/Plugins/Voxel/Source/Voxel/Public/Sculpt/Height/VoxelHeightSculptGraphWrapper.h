// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelParameterOverridesOwner.h"
#include "VoxelHeightSculptGraphWrapper.generated.h"

class UVoxelHeightSculptGraph;

USTRUCT(BlueprintType, meta = (HasNativeMake = "/Script/Voxel.VoxelHeightSculptBlueprintLibrary.MakeVoxelHeightSculptGraphWrapper"))
struct VOXEL_API FVoxelHeightSculptGraphWrapper
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelHeightSculptGraph> Graph;

	UPROPERTY()
	FVoxelParameterOverrides ParameterOverrides;

	bool IsValid() const
	{
		return Graph != nullptr;
	}
};