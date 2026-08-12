// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelExposedSeed.h"
#include "MegaMaterial/VoxelRenderMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialTarget.h"
#include "Materials/MaterialInstanceBasePropertyOverrides.h"
#include "VoxelMegaMaterialState.generated.h"

class UVoxelMetadata;
class UVoxelMegaMaterial;
class UVoxelSurfaceTypeAsset;

USTRUCT()
struct FVoxelMaterialFunctionBaseState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<const UMaterialFunction> Object;

	UPROPERTY()
	FGuid StateId;

	FVoxelMaterialFunctionBaseState() = default;
#if WITH_EDITOR
	explicit FVoxelMaterialFunctionBaseState(const UMaterialFunction& MaterialFunction);
#endif
};

USTRUCT()
struct FVoxelMaterialFunctionState : public FVoxelMaterialFunctionBaseState
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxelMaterialFunctionBaseState> UsedFunctions;

	FVoxelMaterialFunctionState() = default;
#if WITH_EDITOR
	explicit FVoxelMaterialFunctionState(const UMaterialFunction& MaterialFunction);
#endif
};

USTRUCT()
struct FVoxelMaterialState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<const UMaterial> Object;

	UPROPERTY()
	FGuid StateId;

	UPROPERTY()
	bool bTangentSpaceNormal = false;

	UPROPERTY()
	TEnumAsByte<EMaterialShadingModel> ShadingModel = {};

	UPROPERTY()
	TArray<FVoxelMaterialFunctionBaseState> UsedFunctions;

	FVoxelMaterialState() = default;
#if WITH_EDITOR
	explicit FVoxelMaterialState(const UMaterial& Material);
#endif
};

USTRUCT()
struct FVoxelMaterialInstanceBaseState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<const UMaterialInstance> Object;

	UPROPERTY()
	FMaterialInstanceBasePropertyOverrides BasePropertyOverrides;

	FVoxelMaterialInstanceBaseState() = default;
#if WITH_EDITOR
	explicit FVoxelMaterialInstanceBaseState(const UMaterialInstance& MaterialInstance);
#endif
};

USTRUCT()
struct FVoxelMaterialInterfaceState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<const UMaterialInterface> Object;

	UPROPERTY()
	FDisplacementScaling DisplacementScaling;

	UPROPERTY()
	TArray<FVoxelMaterialInstanceBaseState> MaterialInstanceHierarchy;

	UPROPERTY()
	FVoxelMaterialState Material;

	FVoxelMaterialInterfaceState() = default;
#if WITH_EDITOR
	explicit FVoxelMaterialInterfaceState(const UMaterialInterface& MaterialInterface);
#endif
};

USTRUCT()
struct FVoxelMegaMaterialState
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxelMaterialFunctionState AttributePostProcess;

	UPROPERTY()
	bool bEnableSmoothBlends = false;

	UPROPERTY()
	bool bEnableDitherNoiseTexture = false;

	UPROPERTY()
	TSoftObjectPtr<UTexture2D> DitherNoiseTexture;

	UPROPERTY()
	bool bGenerateMaskedMaterial = false;

	UPROPERTY()
	bool bGenerateTwoSidedMaterial = false;

	UPROPERTY()
	bool bSetHasPixelAnimation = false;

	UPROPERTY()
	bool bEnablePixelDepthOffset = false;

	UPROPERTY()
	FVoxelMaterialInterfaceState CustomOutputsMaterial;

	FVoxelMegaMaterialState() = default;
#if WITH_EDITOR
	explicit FVoxelMegaMaterialState(const UVoxelMegaMaterial& MegaMaterial);
#endif
};

USTRUCT()
struct FVoxelSurfaceTypeState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<const UVoxelSurfaceTypeAsset> Object;

	UPROPERTY()
	float BlendSmoothness = 0.f;

	UPROPERTY()
	FVoxelExposedSeed Seed;

	UPROPERTY()
	FVoxelMaterialInterfaceState MaterialInterface;

	FVoxelSurfaceTypeState() = default;
#if WITH_EDITOR
	explicit FVoxelSurfaceTypeState(const UVoxelSurfaceTypeAsset& Asset);
#endif
};

USTRUCT()
struct FVoxelMegaMaterialTargetBaseState
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxelMegaMaterialState MegaMaterial;

	UPROPERTY()
	TMap<FVoxelMaterialRenderIndex, FVoxelSurfaceTypeState> IndexToSurfaceType;

	UPROPERTY()
	TArray<TObjectPtr<UVoxelMetadata>> MetadataIndexToMetadata;
};

USTRUCT()
struct FVoxelMegaMaterialTargetState : public FVoxelMegaMaterialTargetBaseState
{
	GENERATED_BODY()

	UPROPERTY()
	EVoxelMegaMaterialTarget Target = {};

	bool Equals(
		const FVoxelMegaMaterialTargetState& New,
		FString& OutDiff) const;
};

USTRUCT()
struct FVoxelMegaMaterialMaterialState
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxelMegaMaterialState MegaMaterial;

	UPROPERTY()
	FVoxelSurfaceTypeState SurfaceType;

	UPROPERTY()
	TArray<TObjectPtr<UVoxelMetadata>> MetadataIndexToMetadata;

	bool Equals(
		const FVoxelMegaMaterialMaterialState& New,
		FString& OutDiff) const;
};