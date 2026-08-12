// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelThumbnailRenderers.h"
#include "VoxelLayerThumbnailRenderer.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelLayerThumbnailRenderer : public UVoxelTextureWithBackgroundRenderer
{
	GENERATED_BODY()

	//~ Begin UVoxelTextureThumbnailRenderer Interface
	virtual FVoxelAssetIcon GetAssetIcon(UObject* Object) const override;
	//~ End UVoxelTextureThumbnailRenderer Interface
};

UCLASS()
class VOXELEDITOR_API UVoxelLayerStackThumbnailRenderer : public UVoxelTextureWithBackgroundRenderer
{
	GENERATED_BODY()

	//~ Begin UVoxelTextureThumbnailRenderer Interface
	virtual FVoxelAssetIcon GetAssetIcon(UObject* Object) const override;
	//~ End UVoxelTextureThumbnailRenderer Interface
};