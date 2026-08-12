// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Materials/MaterialRelevance.h"
#include "VoxelMeshComponent.generated.h"

class FVoxelMeshRenderProxy;

UCLASS()
class VOXEL_API UVoxelMeshComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UVoxelMeshComponent();

	const TSharedPtr<FVoxelMeshRenderProxy>& GetRenderProxy() const
	{
		return RenderProxy;
	}

	void SetRenderProxy(
		const TSharedRef<FVoxelMeshRenderProxy>& NewRenderProxy,
		const TSharedRef<FVoxelMaterialRef>& NewMaterialRef,
		const TSharedRef<FVoxelMaterialRef>& NewLumenMaterialRef,
		bool bNewCacheTextureStreaming);

	void ClearRenderProxy();

public:
	//~ Begin UPrimitiveComponent Interface
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual bool ShouldCreatePhysicsState() const override { return false; }

	virtual void GetUsedMaterials(
		TArray<UMaterialInterface*>& OutMaterials,
		bool bGetDebugMaterials) const override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual void GetStreamingRenderAssetInfo(
		FStreamingTextureLevelContext& LevelContext,
		TArray<FStreamingRenderAssetPrimitiveInfo>& OutStreamingRenderAssets) const override;
	//~ End UPrimitiveComponent Interface

private:
	TSharedPtr<FVoxelMeshRenderProxy> RenderProxy;
	TSharedPtr<FVoxelMaterialRef> MaterialRef;
	TSharedPtr<FVoxelMaterialRef> LumenMaterialRef;

	bool bCacheTextureStreaming = false;

	friend class FVoxelMeshSceneProxy;
};