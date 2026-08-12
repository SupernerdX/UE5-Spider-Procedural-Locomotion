// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class FVoxelMesh;
class FVoxelTexturePoolRef;
class FVoxelTextureManager;
class FVoxelMegaMaterialProxy;

struct FVoxelMegaMaterialRenderData
{
	TSharedPtr<FVoxelTextureManager> TextureManager;
	int32 ChunkIndicesIndex = 0;
	TVoxelArray<TSharedRef<FVoxelTexturePoolRef>> TextureRefs;
};

struct FVoxelMegaMaterialRenderUtilities
{
	static TSharedRef<const FVoxelMegaMaterialRenderData> BuildRenderData(
		const FVoxelMegaMaterialProxy& MegaMaterialProxy,
		const TSharedRef<FVoxelTextureManager>& TextureManager,
		const FVoxelMesh& Mesh);
};