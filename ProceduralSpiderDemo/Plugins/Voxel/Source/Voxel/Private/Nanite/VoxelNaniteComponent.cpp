// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nanite/VoxelNaniteComponent.h"
#include "Nanite/VoxelNaniteMesh.h"
#include "Nanite/VoxelNaniteMaterialRenderer.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "VoxelMesh.h"
#include "VoxelConfig.h"

#include "NaniteSceneProxy.h"
#include "StaticMeshResources.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

UVoxelNaniteComponent::UVoxelNaniteComponent()
{
	bCanEverAffectNavigation = false;

	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UVoxelNaniteComponent::SetMesh(
	const TSharedRef<const FVoxelNaniteMesh>& NewMesh,
	const FVoxelConfig& Config,
	const FVoxelNaniteMaterialRenderer& MaterialRenderer)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	ensure(Mesh != NewMesh);
	Mesh = NewMesh;

	for (int32 Index = 1; Index < OverrideMaterials.Num(); Index++)
	{
		SetMaterial(Index, nullptr);
	}

	ensure(Mesh->StaticMesh);
	SetStaticMesh(Mesh->StaticMesh);

	ensure(2 + Mesh->Mesh->UsedSurfaceTypes.Num() == GetNumMaterials());
	ensure(2 + Mesh->Mesh->UsedSurfaceTypes.Num() == Mesh->StaticMesh->GetRenderData()->LODResources[0].Sections.Num());

	int32 UsedMaterialIndex = 0;

	const auto AddUsedMaterial = [&](const FVoxelMaterialRenderIndex RenderIndex)
	{
		const TSharedPtr<FVoxelMaterialInstanceRef> Instance = MaterialRenderer.GetMaterialInstance(RenderIndex);
		if (!ensureVoxelSlow(Instance))
		{
			return;
		}

		UMaterialInterface* Material = Instance->GetInstance();
		if (!ensureVoxelSlow(Material))
		{
			return;
		}

		if (Material->GetBlendMode() == BLEND_Translucent)
		{
			// Otherwise the entire mesh won't render
			Material = nullptr;
		}

		SetMaterial(1 + UsedMaterialIndex, Material);

		UsedMaterialIndex++;
	};

	// Always mark the default material as used
	AddUsedMaterial({});

	for (const FVoxelSurfaceType SurfaceType : Mesh->Mesh->UsedSurfaceTypes)
	{
		AddUsedMaterial(Config.MegaMaterialProxy->GetRenderIndex(SurfaceType));
	}

	DisplacementFade = Config.DisplacementFade;
	bCacheTextureStreaming = Config.bCacheTextureStreaming;
}

void UVoxelNaniteComponent::ClearMesh()
{
	VOXEL_FUNCTION_COUNTER();

	Mesh.Reset();
	NaniteMaterial.Reset();
	UsedMaterials.Reset();

	for (int32 Index = 0; Index < OverrideMaterials.Num(); Index++)
	{
		SetMaterial(Index, nullptr);
	}

	SetStaticMesh(nullptr);
}

void UVoxelNaniteComponent::SetNaniteMaterial(const TSharedPtr<FVoxelMaterialInstanceRef>& Material)
{
	NaniteMaterial = Material;
	SetMaterial(0, Material->GetMaterial());
}

bool UVoxelNaniteComponent::ShouldCreatePhysicsState() const
{
	return false;
}

void UVoxelNaniteComponent::OnComponentDestroyed(const bool bDestroyingHierarchy)
{
	VOXEL_FUNCTION_COUNTER();

	Super::OnComponentDestroyed(bDestroyingHierarchy);

	// Clear memory
	Mesh.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelNaniteSceneProxy : public Nanite::FSceneProxy
{
public:
	// Make sure to keep the mesh alive while the proxy is alive
	TSharedPtr<const FVoxelNaniteMesh> Mesh;

	using FSceneProxy::FSceneProxy;
	UE_NONCOPYABLE(FVoxelNaniteSceneProxy);

	void UpdateDisplacement(
		const FDisplacementFadeRange& DisplacementFadeRange,
		const float Scale)
	{
		if (DisplacementFadeRange.IsValid())
		{
			const bool bUseTessellation = UseNaniteTessellation();
			for (FMaterialSection& MaterialSection : MaterialSections)
			{
				if (bUseTessellation &&
					MaterialSection.MaterialRelevance.bUsesDisplacement)
				{
					MaterialSection.DisplacementFadeRange = DisplacementFadeRange;
				}
			}

			MaterialDisplacementFadeOutSize = FMath::Min(DisplacementFadeRange.StartSizePixels, DisplacementFadeRange.EndSizePixels);
		}

		MinMaxMaterialDisplacement = FVector2f::Zero();

		for (FMaterialSection& MaterialSection : MaterialSections)
		{
			MaterialSection.DisplacementScaling.Magnitude /= Scale;

			const float MinDisplacement = (0.0f - MaterialSection.DisplacementScaling.Center) * MaterialSection.DisplacementScaling.Magnitude;
			const float MaxDisplacement = (1.0f - MaterialSection.DisplacementScaling.Center) * MaterialSection.DisplacementScaling.Magnitude;

			MinMaxMaterialDisplacement.X = FMath::Min(MinMaxMaterialDisplacement.X, MinDisplacement);
			MinMaxMaterialDisplacement.Y = FMath::Max(MinMaxMaterialDisplacement.Y, MaxDisplacement);
		}
	}
};

FPrimitiveSceneProxy* UVoxelNaniteComponent::CreateStaticMeshSceneProxy(
	Nanite::FMaterialAudit& NaniteMaterials,
	const bool bCreateNanite)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensureVoxelSlow(bCreateNanite))
	{
		return nullptr;
	}

	extern bool GVoxelShowRaytracedMeshes;
	extern bool GVoxelShowDistanceFieldMeshes;

	if (GVoxelShowRaytracedMeshes ||
		GVoxelShowDistanceFieldMeshes)
	{
		return nullptr;
	}

	const FVector Scale = GetRelativeScale3D();
	ensure(Scale.IsUniform());

	FVoxelNaniteSceneProxy* Proxy = new FVoxelNaniteSceneProxy(NaniteMaterials, this);
	Proxy->Mesh = Mesh;
	Proxy->UpdateDisplacement(DisplacementFade, Scale.X);
	return Proxy;
}

void UVoxelNaniteComponent::GetStreamingRenderAssetInfo(
	FStreamingTextureLevelContext& LevelContext,
	TArray<FStreamingRenderAssetPrimitiveInfo>& OutStreamingRenderAssets) const
{
	if (!bCacheTextureStreaming)
	{
		Super::GetStreamingRenderAssetInfo(LevelContext, OutStreamingRenderAssets);
		return;
	}

	if (bIgnoreInstanceForTextureStreaming ||
		!GetStaticMesh() ||
		GetStaticMesh()->IsCompiling() ||
		!(GetStaticMesh()->HasValidRenderData() || GetStaticMesh()->HasValidNaniteData()))
	{
		return;
	}

	if (CanSkipGetTextureStreamingRenderAssetInfo() ||
		CVarStreamingUseNewMetrics.GetValueOnGameThread() == 0)
	{
		return;
	}

	LevelContext.BindBuildData(nullptr);

	if (NaniteMaterial)
	{
		NaniteMaterial->GetStreamingRenderAssetInfo(
			LevelContext,
			Bounds,
			StreamingDistanceMultiplier,
			OutStreamingRenderAssets);
	}
	for (const TSharedPtr<FVoxelMaterialInstanceRef>& Material : UsedMaterials)
	{
		Material->GetStreamingRenderAssetInfo(
			LevelContext,
			Bounds,
			StreamingDistanceMultiplier,
			OutStreamingRenderAssets);
	}
}