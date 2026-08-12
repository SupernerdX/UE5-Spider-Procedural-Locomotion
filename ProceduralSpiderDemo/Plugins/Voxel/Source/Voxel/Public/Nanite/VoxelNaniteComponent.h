// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "VoxelNaniteComponent.generated.h"

class FVoxelNaniteMesh;
class FVoxelMegaMaterialProxy;
class FVoxelNaniteMaterialRenderer;
struct FVoxelConfig;

UCLASS(NotBlueprintable)
class VOXEL_API UVoxelNaniteComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UVoxelNaniteComponent();

	const TSharedPtr<const FVoxelNaniteMesh>& GetMesh() const
	{
		return Mesh;
	}

	void SetMesh(
		const TSharedRef<const FVoxelNaniteMesh>& NewMesh,
		const FVoxelConfig& Config,
		const FVoxelNaniteMaterialRenderer& MaterialRenderer);

	void ClearMesh();
	void SetNaniteMaterial(const TSharedPtr<FVoxelMaterialInstanceRef>& Material);

	//~ Begin UStaticMeshComponent Interface
	virtual bool ShouldCreatePhysicsState() const override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	virtual FPrimitiveSceneProxy* CreateStaticMeshSceneProxy(
		Nanite::FMaterialAudit& NaniteMaterials,
		bool bCreateNanite) override;
	virtual void GetStreamingRenderAssetInfo(
		FStreamingTextureLevelContext& LevelContext,
		TArray<FStreamingRenderAssetPrimitiveInfo>& OutStreamingRenderAssets) const override;
	//~ End UStaticMeshComponent Interface

private:
	TSharedPtr<const FVoxelNaniteMesh> Mesh;
	TSharedPtr<FVoxelMaterialInstanceRef> NaniteMaterial;
	TArray<TSharedPtr<FVoxelMaterialInstanceRef>> UsedMaterials;
	FDisplacementFadeRange DisplacementFade;

	bool bCacheTextureStreaming = false;
};