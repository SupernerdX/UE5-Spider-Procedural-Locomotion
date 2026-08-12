// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMegaMaterialTarget.h"
#include "VoxelMegaMaterial.generated.h"

class UMaterial;
class UMaterialInstanceConstant;
class UVoxelSurfaceTypeAsset;
class UVoxelCubicSurfaceTypeAsset;
class UVoxelCubicTextureTemplate;
class FVoxelMegaMaterialProxy;
class FVoxelMegaMaterialUsageTracker;
class UVoxelMegaMaterialGeneratedData;
struct FVoxelCubicTextureArrayData;

UENUM(meta = (VoxelSegmentedEnum))
enum class EVoxelMegaMaterialGenerationType
{
	Custom,
	Generated
};

UCLASS(BlueprintType, meta = (VoxelAssetType, AssetColor=Green))
class VOXEL_API UVoxelMegaMaterial : public UVoxelAsset
{
	GENERATED_BODY()

public:
	// Will show a notification when we are trying to render a mega material with new surfaces not in the Surfaces array
	UPROPERTY(EditAnywhere, Category = "Surfaces")
	bool bDetectNewSurfaces = true;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> Materials;

	UPROPERTY(EditAnywhere, Category = "Surfaces")
	TArray<TObjectPtr<UVoxelSurfaceTypeAsset>> SurfaceTypes;

public:
	// Template defining texture slots and material for cubic surfaces
	// Create a CubicTextureTemplate asset, add texture slots, and assign a material
	UPROPERTY(EditAnywhere, Category = "Cubic Surfaces")
	TObjectPtr<UVoxelCubicTextureTemplate> CubicTextureTemplate;

	// Cubic surface types - texture arrays generated automatically from template slots
	UPROPERTY(EditAnywhere, Category = "Cubic Surfaces", meta = (EditCondition = "CubicTextureTemplate != nullptr", EditConditionHides))
	TArray<TObjectPtr<UVoxelCubicSurfaceTypeAsset>> CubicSurfaceTypes;

public:
	// This material function will be applied everywhere
	// It needs to have one input and one output, each of the type MaterialAttribute
	UPROPERTY(EditAnywhere, Category = "Global")
	TObjectPtr<UMaterialFunction> AttributePostProcess;

public:
	UPROPERTY(EditAnywhere, Category = "Non-Nanite")
	EVoxelMegaMaterialGenerationType NonNaniteMaterialType = EVoxelMegaMaterialGenerationType::Custom;

	// If you override this, this material will be used to render non-nanite meshes
	// Use Get Voxel Material Info to get the material info
	UPROPERTY(EditAnywhere, Category = "Non-Nanite", meta = (EditCondition = "NonNaniteMaterialType == EVoxelMegaMaterialGenerationType::Custom", EditConditionHides))
	TObjectPtr<UMaterialInterface> CustomNonNaniteMaterial;

public:
	UPROPERTY(EditAnywhere, Category = "Nanite")
	EVoxelMegaMaterialGenerationType NaniteDisplacementMaterialType = EVoxelMegaMaterialGenerationType::Generated;

	// If you override this, this material displacement output will be used instead of the generated one
	// Use Get Voxel Material Info to get the material info
	UPROPERTY(EditAnywhere, Category = "Nanite", meta = (EditCondition = "NaniteDisplacementMaterialType == EVoxelMegaMaterialGenerationType::Custom", EditConditionHides))
	TObjectPtr<UMaterialInterface> CustomNaniteDisplacementMaterial;

public:
	UPROPERTY(EditAnywhere, Category = "Lumen")
	EVoxelMegaMaterialGenerationType LumenMaterialType = EVoxelMegaMaterialGenerationType::Generated;

	// Use this to override the material used by Lumen
	// Use Get Voxel Material Info to get the material info
	UPROPERTY(EditAnywhere, Category = "Lumen", meta = (EditCondition = "LumenMaterialType == EVoxelMegaMaterialGenerationType::Custom", EditConditionHides))
	TObjectPtr<UMaterialInterface> CustomLumenMaterial;

public:
	// Enable dither-based smooth blends
	UPROPERTY(EditAnywhere, Category = "Misc")
	bool bEnableSmoothBlends = true;

	UPROPERTY(EditAnywhere, Category = "Misc", meta = (InlineEditConditionToggle))
	bool bEnableDitherNoiseTexture = false;

	// Noise texture to apply to the smooth blend dithering
	UPROPERTY(EditAnywhere, Category = "Misc", AdvancedDisplay, meta = (EditCondition = "bEnableDitherNoiseTexture"))
	TSoftObjectPtr<UTexture2D> DitherNoiseTexture;

	// If true the generated materials will be set to the Masked blend mode
	UPROPERTY(EditAnywhere, Category = "Misc", AdvancedDisplay)
	bool bGenerateMaskedMaterial = false;

	// If true the generated materials will be set to two sided
	UPROPERTY(EditAnywhere, Category = "Misc", AdvancedDisplay)
	bool bGenerateTwoSidedMaterial = false;

	// If true, will set bHasPixelAnimation to true to reduce TSR blurriness
	UPROPERTY(EditAnywhere, Category = "Misc", AdvancedDisplay)
	bool bSetHasPixelAnimation = false;

	// Compiles the PixelDepthOffset output in the non-nanite material
	UPROPERTY(EditAnywhere, Category = "Misc", AdvancedDisplay)
	bool bEnablePixelDepthOffset = false;

	// Custom output nodes in this material will be copied to the generated material
	UPROPERTY(EditAnywhere, Category = "Misc", AdvancedDisplay)
	TObjectPtr<UMaterialInterface> CustomOutputsMaterial;

public:
#if WITH_EDITOR
	bool bEditorOnly = false;

	static UVoxelMegaMaterial* CreateTransient();
#endif

public:
	UVoxelMegaMaterial();

	TSharedRef<FVoxelMegaMaterialProxy> GetProxy();
#if WITH_EDITOR
	UVoxelMegaMaterialGeneratedData& GetGeneratedData_EditorOnly();
#endif

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform) override;
#endif
	//~ End UObject Interface

private:
	UPROPERTY(Transient)
	TObjectPtr<UVoxelMegaMaterialGeneratedData> GeneratedData;

	// Generated cubic material - stored here (not in GeneratedData) to avoid
	// invalidation loops when CheckMaterial triggers recompilation
	// Base material is opaque or masked depending on whether any cubic surfaces use masked blend mode
	UPROPERTY(Transient)
	TObjectPtr<UMaterial> GeneratedCubicMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceConstant> GeneratedCubicMaterialInstance;

	// Translucent variant of the cubic material (only generated if any cubic surfaces use translucent blend mode)
	UPROPERTY(Transient)
	TObjectPtr<UMaterial> GeneratedCubicMaterialTranslucent;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceConstant> GeneratedCubicMaterialTranslucentInstance;

	UPROPERTY()
	TObjectPtr<UVoxelMegaMaterialGeneratedData> CookedGeneratedData;

	TSharedPtr<FVoxelMegaMaterialProxy> Proxy;

#if WITH_EDITOR
	TSharedPtr<FVoxelMegaMaterialUsageTracker> UsageTracker;
	TVoxelMap<EVoxelMegaMaterialTarget, TWeakPtr<FVoxelNotification>> TargetToNotification;
	TWeakPtr<FVoxelNotification> EmptyMegaMaterialNotification;
#endif

	friend FVoxelMegaMaterialProxy;
	friend FVoxelCubicTextureArrayData;
	friend UVoxelMegaMaterialGeneratedData;
};