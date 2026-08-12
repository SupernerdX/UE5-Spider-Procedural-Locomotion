// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMetadataThumbnailRenderer.h"
#include "VoxelMetadata.h"

DEFINE_VOXEL_THUMBNAIL_RENDERER_RECURSIVE(UVoxelMetadataThumbnailRenderer, UVoxelMetadata);

FVoxelAssetIcon UVoxelMetadataThumbnailRenderer::GetAssetIcon(UObject* Object) const
{
	return CastChecked<UVoxelMetadata>(Object)->AssetIcon;
}