// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelThumbnailRenderers.h"
#include "VoxelStaticMeshAssetThumbnailRenderer.generated.h"

UCLASS()
class UVoxelStaticMeshAssetThumbnailRenderer : public UVoxelStaticMeshThumbnailRenderer
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelStaticMeshThumbnailRenderer Interface
	virtual UStaticMesh* GetStaticMesh(UObject* Object, TArray<UMaterialInterface*>& OutMaterialOverrides) const override;
	//~ End UVoxelStaticMeshThumbnailRenderer Interface
};