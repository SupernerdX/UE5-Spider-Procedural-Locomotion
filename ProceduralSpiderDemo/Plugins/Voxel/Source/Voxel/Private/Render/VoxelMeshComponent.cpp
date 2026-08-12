// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelMeshComponent.h"
#include "VoxelMesh.h"
#include "VoxelMeshSceneProxy.h"
#include "VoxelMeshRenderProxy.h"
#include "Materials/MaterialInterface.h"

UVoxelMeshComponent::UVoxelMeshComponent()
{
	CastShadow = true;
	bUseAsOccluder = true;
	bCanEverAffectNavigation = false;

	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UVoxelMeshComponent::SetRenderProxy(
	const TSharedRef<FVoxelMeshRenderProxy>& NewRenderProxy,
	const TSharedRef<FVoxelMaterialRef>& NewMaterialRef,
	const TSharedRef<FVoxelMaterialRef>& NewLumenMaterialRef,
	const bool bNewCacheTextureStreaming)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	ensureVoxelSlow(IsRegistered());

	RenderProxy = NewRenderProxy;
	MaterialRef = NewMaterialRef;
	LumenMaterialRef = NewLumenMaterialRef;

	bAffectDynamicIndirectLighting = NewRenderProxy->bEnableLumen;
	bVisibleInRayTracing = NewRenderProxy->bEnableRaytracing;
	bAffectDistanceFieldLighting = NewRenderProxy->bEnableMeshDistanceField;

	// Needed since our proxies are hidden when using Nanite
	bAffectIndirectLightingWhileHidden = true;

	RuntimeVirtualTextures.Reset();

	for (const TVoxelObjectPtr<URuntimeVirtualTexture>& RuntimeVirtualTexture : NewRenderProxy->RuntimeVirtualTextures)
	{
		RuntimeVirtualTextures.Add(RuntimeVirtualTexture.Resolve_Ensured());
	}

	bCacheTextureStreaming = bNewCacheTextureStreaming;

	MarkRenderStateDirty();

#if WITH_EDITOR
	FVoxelUtilities::EnsureViewportIsUpToDate();
#endif
}

void UVoxelMeshComponent::ClearRenderProxy()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	ensureVoxelSlow(IsRegistered());

	RenderProxy.Reset();
	MaterialRef.Reset();
	LumenMaterialRef.Reset();
	RuntimeVirtualTextures.Reset();

	MarkRenderStateDirty();

#if WITH_EDITOR
	FVoxelUtilities::EnsureViewportIsUpToDate();
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FPrimitiveSceneProxy* UVoxelMeshComponent::CreateSceneProxy()
{
	VOXEL_FUNCTION_COUNTER();

	if (!MaterialRef ||
		!RenderProxy)
	{
		return nullptr;
	}

	return new FVoxelMeshSceneProxy(*this);
}

void UVoxelMeshComponent::GetUsedMaterials(
	TArray<UMaterialInterface*>& OutMaterials,
	const bool bGetDebugMaterials) const
{
	Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);

	if (MaterialRef)
	{
		OutMaterials.Add(MaterialRef->GetMaterial());
	}

	if (LumenMaterialRef)
	{
		OutMaterials.Add(LumenMaterialRef->GetMaterial());
	}
}

FBoxSphereBounds UVoxelMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	const FVoxelBox LocalBounds = RenderProxy ? RenderProxy->Mesh->Bounds : FVoxelBox();
	ensure(LocalBounds.IsValid());
	return LocalBounds.TransformBy(LocalToWorld).ToFBox();
}

void UVoxelMeshComponent::OnComponentDestroyed(const bool bDestroyingHierarchy)
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnComponentDestroyed(bDestroyingHierarchy);

	// Clear memory
	RenderProxy.Reset();
}

void UVoxelMeshComponent::GetStreamingRenderAssetInfo(
	FStreamingTextureLevelContext& LevelContext,
	TArray<FStreamingRenderAssetPrimitiveInfo>& OutStreamingRenderAssets) const
{
	if (!bCacheTextureStreaming)
	{
		Super::GetStreamingRenderAssetInfo(LevelContext, OutStreamingRenderAssets);
		return;
	}

	if (CanSkipGetTextureStreamingRenderAssetInfo() ||
		CVarStreamingUseNewMetrics.GetValueOnGameThread() == 0)
	{
		return;
	}

	LevelContext.BindBuildData(nullptr);

	if (MaterialRef)
	{
		MaterialRef->GetStreamingRenderAssetInfo(
			LevelContext,
			Bounds,
			Bounds.SphereRadius,
			OutStreamingRenderAssets);
	}

	if (LumenMaterialRef)
	{
		LumenMaterialRef->GetStreamingRenderAssetInfo(
			LevelContext,
			Bounds,
			Bounds.SphereRadius,
			OutStreamingRenderAssets);
	}
}