// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphThumbnailRenderer.h"
#include "VoxelGraph.h"

DEFINE_VOXEL_THUMBNAIL_RENDERER_RECURSIVE(UVoxelGraphThumbnailRenderer, UVoxelGraph);

FVoxelAssetIcon UVoxelGraphThumbnailRenderer::GetAssetIcon(UObject* Object) const
{
	return CastChecked<UVoxelGraph>(Object)->AssetIcon;
}