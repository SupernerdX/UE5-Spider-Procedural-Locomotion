// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSurfaceThumbnails.h"
#include "Surface/VoxelSurfaceTypeAsset.h"
#include "SceneView.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"

DEFINE_VOXEL_THUMBNAIL_RENDERER(UVoxelSurfaceAssetThumbnailRenderer, UVoxelSurfaceTypeAsset);

void UVoxelSurfaceAssetThumbnailRenderer::BeginDestroy()
{
	VOXEL_FUNCTION_COUNTER();

	Super::BeginDestroy();

	ThumbnailScene.Reset();
}

void UVoxelSurfaceAssetThumbnailRenderer::Draw(
	UObject* Object,
	const int32 X,
	const int32 Y,
	const uint32 Width,
	const uint32 Height,
	FRenderTarget* RenderTarget,
	FCanvas* Canvas,
	const bool bAdditionalViewFamily)
{
	VOXEL_FUNCTION_COUNTER();

	const UVoxelSurfaceTypeAsset* Asset = Cast<UVoxelSurfaceTypeAsset>(Object);
	if (!ensureVoxelSlow(Asset))
	{
		return;
	}

	UMaterialInterface* Material = Asset->Material;
	if (!Material)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
	if (!ensure(Material))
	{
		return;
	}

	if (!ThumbnailScene)
	{
		ThumbnailScene = MakeShared<FMaterialThumbnailScene>();
	}

	ThumbnailScene->SetMaterialInterface(Material);

	FSceneViewFamilyContext ViewFamily(
		FSceneViewFamily::ConstructionValues(
			RenderTarget,
			ThumbnailScene->GetScene(),
			FEngineShowFlags(ESFIM_Game))
		.SetTime(GetTime())
		.SetAdditionalViewFamily(bAdditionalViewFamily));

	ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
	ViewFamily.EngineShowFlags.SetSeparateTranslucency(ThumbnailScene->ShouldSetSeparateTranslucency(Material));
	ViewFamily.EngineShowFlags.MotionBlur = 0;
	ViewFamily.EngineShowFlags.AntiAliasing = 0;

	FSceneView* View = ThumbnailScene->CreateView(
		&ViewFamily,
		X,
		Y,
		Width,
		Height);

	RenderViewFamily(
		Canvas,
		&ViewFamily,
		View);

	ThumbnailScene->SetMaterialInterface(nullptr);
}

bool UVoxelSurfaceAssetThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	const UVoxelSurfaceTypeAsset* Asset = Cast<UVoxelSurfaceTypeAsset>(Object);
	if (!ensureVoxelSlow(Asset))
	{
		return false;
	}

	if (Asset->Material &&
		Asset->Material->IsCompiling())
	{
		return false;
	}

	return true;
}