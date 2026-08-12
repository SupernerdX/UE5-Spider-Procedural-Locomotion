// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSurfaceTypePicker.generated.h"

class UVoxelSurfaceTypeInterface;

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelSurfaceTypePicker
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelSurfaceTypeInterface> SurfaceType;

	FVoxelSurfaceTypePicker() = default;
	FVoxelSurfaceTypePicker(UVoxelSurfaceTypeInterface* SurfaceType)
		: SurfaceType(SurfaceType)
	{
	}
};