// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelThumbnailRenderers.h"
#include "VoxelMetadataThumbnailRenderer.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelMetadataThumbnailRenderer : public UVoxelTextureWithBackgroundRenderer
{
	GENERATED_BODY()

	//~ Begin UVoxelTextureThumbnailRenderer Interface
	virtual FVoxelAssetIcon GetAssetIcon(UObject* Object) const override;
	//~ End UVoxelTextureThumbnailRenderer Interface
};