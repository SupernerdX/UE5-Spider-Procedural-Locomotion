// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLatentAction.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Texture/VoxelTextureBlueprintLibrary.h"
#include "VoxelTextureBlueprintLibrary_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelTextureBlueprintLibrary_BlueprintOnly : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Create a voxel texture to be used in voxel graphs from a render target
	 * @param RenderTarget Render target to read from
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Texture")
	static void CreateVoxelTextureFromRenderTarget(
		UVoxelTexture*& Texture,
		UTextureRenderTarget2D* RenderTarget)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelTextureBlueprintLibrary::K2_CreateVoxelTextureFromRenderTarget(
				Texture,
				RenderTarget);
		});
	}

	/**
	 * Create a voxel texture to be used in voxel graphs from a render target
	 * @param RenderTarget Render target to read from
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Texture", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void CreateVoxelTextureFromRenderTargetAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UVoxelTexture*& Texture,
		UTextureRenderTarget2D* RenderTarget,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelTextureBlueprintLibrary::K2_CreateVoxelTextureFromRenderTarget(
					Texture,
					RenderTarget);
			});
	}
};