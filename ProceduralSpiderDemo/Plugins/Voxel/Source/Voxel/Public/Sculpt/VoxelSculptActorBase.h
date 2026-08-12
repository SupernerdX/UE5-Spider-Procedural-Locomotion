// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSculptActorBase.generated.h"

UCLASS(NotBlueprintable)
class VOXEL_API AVoxelSculptActorBase : public AActor
{
	GENERATED_BODY()

public:
	virtual void ClearSculptCache() VOXEL_PURE_VIRTUAL();
};