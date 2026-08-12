// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFunctionLibrary.h"
#include "VoxelPCGFunctionLibrary.generated.h"

UCLASS()
class VOXELPCG_API UVoxelPCGFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "PCG", DisplayName = "Get PCG Bounds", meta = (AllowList = "PCG"))
	FVoxelBox GetPCGBounds() const;
};