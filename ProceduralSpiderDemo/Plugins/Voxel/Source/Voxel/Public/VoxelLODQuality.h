// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLODQuality.generated.h"

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelLODQuality
{
	GENERATED_BODY()

public:
	// Game Quality: quality used in game (including PIE)
	//
	// Higher quality = better looking voxel mesh, but higher memory & rendering cost
	// The voxel mesh will first compute at MinQuality, and slowly increase its resolution up to MaxQuality
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FFloatInterval GameQuality = FFloatInterval(1.f, 1.f);

	// Editor Quality: quality used in the editor (outside of PIE)
	//
	// Higher quality = better looking voxel mesh, but higher memory & rendering cost
	// The voxel mesh will first compute at MinQuality, and slowly increase its resolution up to MaxQuality
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (EditCondition = "!bAlwaysUseGameQuality", EditConditionHides))
	FFloatInterval EditorQuality = FFloatInterval(1.f, 1.f);

	// If true, will always use Game Quality, even in editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAlwaysUseGameQuality = false;

public:
	FORCEINLINE bool operator==(const FVoxelLODQuality& Other) const
	{
		return
			GameQuality.Min == Other.GameQuality.Min &&
			GameQuality.Max == Other.GameQuality.Max &&
			EditorQuality.Min == Other.EditorQuality.Min &&
			EditorQuality.Max == Other.EditorQuality.Max &&
			bAlwaysUseGameQuality == Other.bAlwaysUseGameQuality;
	}
};