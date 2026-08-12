// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "VoxelStampMeshPreviewComponent.generated.h"

UCLASS(NotBlueprintable, meta = (VoxelPreviewComponent))
class VOXEL_API UVoxelStampMeshPreviewComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UVoxelStampMeshPreviewComponent();
};