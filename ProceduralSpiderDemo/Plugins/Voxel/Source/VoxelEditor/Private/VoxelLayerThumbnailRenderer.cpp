// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelLayerThumbnailRenderer.h"
#include "VoxelLayer.h"
#include "VoxelLayerStack.h"

DEFINE_VOXEL_THUMBNAIL_RENDERER_RECURSIVE(UVoxelLayerThumbnailRenderer, UVoxelLayer);
DEFINE_VOXEL_THUMBNAIL_RENDERER(UVoxelLayerStackThumbnailRenderer, UVoxelLayerStack);

FVoxelAssetIcon UVoxelLayerThumbnailRenderer::GetAssetIcon(UObject* Object) const
{
	return CastChecked<UVoxelLayer>(Object)->AssetIcon;
}

FVoxelAssetIcon UVoxelLayerStackThumbnailRenderer::GetAssetIcon(UObject* Object) const
{
	return CastChecked<UVoxelLayerStack>(Object)->AssetIcon;
}