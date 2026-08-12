// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTextureBlueprintLibrary.generated.h"

class UTextureRenderTarget2D;
class UVoxelTexture;

UCLASS()
class VOXEL_API UVoxelTextureBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TVoxelFuture<UVoxelTexture*> CreateVoxelTextureFromRenderTarget(UTextureRenderTarget2D* RenderTarget);

	/**
	 * Create a voxel texture to be used in voxel graphs from a render target
	 * @param RenderTarget	Render target to read from
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Texture")
	static FVoxelFuture K2_CreateVoxelTextureFromRenderTarget(
		UVoxelTexture*& Texture,
		UTextureRenderTarget2D* RenderTarget);
};