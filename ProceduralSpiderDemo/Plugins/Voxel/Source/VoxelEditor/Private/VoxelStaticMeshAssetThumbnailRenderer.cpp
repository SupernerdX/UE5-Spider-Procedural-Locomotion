// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStaticMeshAssetThumbnailRenderer.h"
#include "StaticMesh/VoxelStaticMesh.h"

DEFINE_VOXEL_THUMBNAIL_RENDERER(UVoxelStaticMeshAssetThumbnailRenderer, UVoxelStaticMesh);

UStaticMesh* UVoxelStaticMeshAssetThumbnailRenderer::GetStaticMesh(UObject* Object, TArray<UMaterialInterface*>& OutMaterialOverrides) const
{
	return CastChecked<UVoxelStaticMesh>(Object)->Mesh.LoadSynchronous();
}