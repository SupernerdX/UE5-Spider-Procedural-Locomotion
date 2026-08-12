// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelTextureThumbnailRenderer.h"
#include "Texture/VoxelTexture.h"
#include "Engine/Texture2D.h"

DEFINE_VOXEL_THUMBNAIL_RENDERER_RECURSIVE(UVoxelTextureAssetThumbnailRenderer, UVoxelTexture);

bool UVoxelTextureAssetThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	const UVoxelTexture* Texture = Cast<UVoxelTexture>(Object);
	if (!ensure(Texture) ||
		!Texture->Texture.LoadSynchronous())
	{
		return false;
	}

	return true;
}

UTexture* UVoxelTextureAssetThumbnailRenderer::GetTexture(UObject* Object) const
{
	const UVoxelTexture* Texture = Cast<UVoxelTexture>(Object);
	if (!ensure(Texture))
	{
		return nullptr;
	}

	return Texture->Texture.LoadSynchronous();
}