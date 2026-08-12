// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelExposedSeed.generated.h"

USTRUCT(BlueprintType, DisplayName = "Seed", meta = (TypeCategory = "Default"))
struct VOXELGRAPH_API FVoxelExposedSeed
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	FString Seed;

	int32 GetSeed() const;
	void Randomize(int32 RandSeed = FMath::Rand());
};