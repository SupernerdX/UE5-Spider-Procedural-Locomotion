// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "VoxelRenderMaterial.h"
#include "Surface/VoxelSurfaceType.h"
#include "VoxelMegaMaterialTarget.h"

class AVoxelWorld;
class UVoxelMegaMaterial;
class UTexture2DArray;
class FVoxelMegaMaterialUsageTracker;
class UVoxelMegaMaterialGeneratedData;
struct FVoxelSurfaceTypeBlend;

class VOXEL_API FVoxelMegaMaterialProxy
{
public:
	const TVoxelObjectPtr<UVoxelMegaMaterial> WeakMegaMaterial;
	const bool bDetectNewSurfaces;

	static TSharedRef<FVoxelMegaMaterialProxy> Default();

	UE_NONCOPYABLE(FVoxelMegaMaterialProxy);

public:
	const TVoxelMap<FVoxelMaterialRenderIndex, TVoxelObjectPtr<UMaterialInterface>>& GetMaterialIndexToMaterial() const
	{
		return RenderIndexToMaterial;
	}
	TConstVoxelArrayView<FVoxelMetadataRef> GetMetadataIndexToMetadata() const
	{
		return MetadataIndexToMetadata;
	}
	TVoxelObjectPtr<UMaterialInterface> GetTargetMaterial(const EVoxelMegaMaterialTarget Target) const
	{
		return TargetToMaterial.FindRef(Target);
	}

public:
	FVoxelMaterialRenderIndex GetRenderIndex(FVoxelSurfaceType SurfaceType) const;
	TConstVoxelArrayView<FVoxelMetadataRef> GetUsedMetadatas(FVoxelMaterialRenderIndex RenderIndex) const;
	TVoxelArray<FVoxelRenderMaterial> GetRenderMaterials(TConstVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypes) const;

public:
	// Cubic surface type accessors
	const TVoxelMap<FName, TVoxelObjectPtr<UTexture2DArray>>& GetCubicTextureArrays() const
	{
		return CubicTextureArrays;
	}
	TVoxelObjectPtr<UMaterialInterface> GetCubicMaterial() const
	{
		return CubicMaterial;
	}
	TVoxelObjectPtr<UMaterialInterface> GetCubicMaterialTranslucent() const
	{
		return CubicMaterialTranslucent;
	}
	int32 GetCubicSliceIndex(FVoxelMaterialRenderIndex RenderIndex) const;
	const TVoxelMap<FVoxelMaterialRenderIndex, int32>& GetRenderIndexToCubicSliceIndex() const
	{
		return RenderIndexToCubicSliceIndex;
	}
	bool HasCubicSurfaces() const
	{
		return CubicTextureArrays.Num() > 0;
	}
	bool ShouldHideFacesAtNaNBoundaries() const
	{
		return bHideFacesAtNaNBoundaries;
	}

public:
#if WITH_EDITOR
	void LogErrors(
		ERHIFeatureLevel::Type FeatureLevel,
		const TSet<EVoxelMegaMaterialTarget>& Targets,
		AVoxelWorld* VoxelWorld) const;
#endif

private:
	TVoxelMap<FVoxelMaterialRenderIndex, TVoxelObjectPtr<UMaterialInterface>> RenderIndexToMaterial;
	TVoxelMap<FVoxelMaterialRenderIndex, TVoxelArray<FVoxelMetadataRef>> RenderIndexToUsedMetadatas;
	TVoxelMap<FVoxelSurfaceType, FVoxelMaterialRenderIndex> SurfaceTypeToRenderIndex;

	TVoxelArray<FVoxelMetadataRef> MetadataIndexToMetadata;
	TVoxelMap<EVoxelMegaMaterialTarget, TVoxelObjectPtr<UMaterialInterface>> TargetToMaterial;

	// Cubic surface data
	TVoxelMap<FName, TVoxelObjectPtr<UTexture2DArray>> CubicTextureArrays;
	TVoxelMap<FVoxelMaterialRenderIndex, int32> RenderIndexToCubicSliceIndex;
	// Base cubic material (opaque or masked based on surface types)
	TVoxelObjectPtr<UMaterialInterface> CubicMaterial;
	// Translucent cubic material variant (only set if translucent surfaces exist)
	TVoxelObjectPtr<UMaterialInterface> CubicMaterialTranslucent;
	bool bHideFacesAtNaNBoundaries = false;

#if WITH_EDITOR
	const TSharedRef<FVoxelMegaMaterialUsageTracker> UsageTracker;
#endif

	explicit FVoxelMegaMaterialProxy(UVoxelMegaMaterial& MegaMaterial);

	void Initialize(const FVoxelMegaMaterialProxy* OldProxy);
	bool Equals(const FVoxelMegaMaterialProxy& Other) const;

	friend UVoxelMegaMaterial;
	friend UVoxelMegaMaterialGeneratedData;
};