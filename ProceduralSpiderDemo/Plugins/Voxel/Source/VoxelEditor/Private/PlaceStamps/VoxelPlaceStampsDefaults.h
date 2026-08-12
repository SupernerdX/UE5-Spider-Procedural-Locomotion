// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelStampBehavior.h"
#include "VoxelHeightBlendMode.h"
#include "VoxelVolumeBlendMode.h"
#include "VoxelMetadataOverrides.h"
#include "VoxelPlaceStampsDefaults.generated.h"

class UVoxelVolumeLayer;
class UVoxelHeightLayer;
class UVoxelSurfaceTypeInterface;
struct FVoxelStampRef;

USTRUCT()
struct FVoxelPlaceStampDefaults
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Defaults")
	EVoxelStampBehavior Behavior = EVoxelStampBehavior::AffectAll;

	UPROPERTY(EditAnywhere, Category = "Defaults", meta = (Units = cm, ClampMin = 0))
	float Smoothness = 100;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	FVoxelMetadataOverrides MetadataOverrides;

	UPROPERTY(EditAnywhere, Category = "Defaults")
	TObjectPtr<UVoxelSurfaceTypeInterface> SurfaceType;

	// This stamp will only be applied on LODs within this range (inclusive)
	UPROPERTY(EditAnywhere, Category = "Defaults", DisplayName = "LOD Range", AdvancedDisplay)
	FInt32Interval LODRange = { 0, 32 };

public:
	// Layer that this stamps belong to
	// You can control the order of layers in Layer Stacks
	// You can select the layer stack to use in your Voxel World or PCG Sampler settings
	UPROPERTY(EditAnywhere, Category = "Height Defaults")
	TObjectPtr<UVoxelHeightLayer> HeightLayer;

	UPROPERTY(EditAnywhere, Category = "Height Defaults")
	EVoxelHeightBlendMode HeightBlendMode = EVoxelHeightBlendMode::Max;

	UPROPERTY(EditAnywhere, Category = "Height Defaults", AdvancedDisplay)
	TArray<TObjectPtr<UVoxelHeightLayer>> HeightAdditionalLayers;

	// If false, this stamp will only apply on parts where another stamp has been applied first
	// This is useful to avoid having stamps going beyond world bounds
	// Only used if BlendMode is not Override nor Intersect
	UPROPERTY(EditAnywhere, Category = "Height Defaults", DisplayName = "Apply On Void", AdvancedDisplay)
	bool bHeightApplyOnVoid = true;

	// By how much to extend the bounds, relative to the bounds size
	// Increase this if you are using a high smoothness
	// Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	UPROPERTY(EditAnywhere, Category = "Height Defaults", AdvancedDisplay, meta = (UIMin = 0, UIMax = 5))
	float HeightPaddingMultiplier = 1.f;

public:
	// Layer that this stamps belong to
	// You can control the order of layers in Layer Stacks
	// You can select the layer stack to use in your Voxel World or PCG Sampler settings
	UPROPERTY(EditAnywhere, Category = "Volume Defaults")
	TObjectPtr<UVoxelVolumeLayer> VolumeLayer;

	UPROPERTY(EditAnywhere, Category = "Volume Defaults")
	EVoxelVolumeBlendMode VolumeBlendMode = EVoxelVolumeBlendMode::Additive;

	UPROPERTY(EditAnywhere, Category = "Volume Defaults", AdvancedDisplay)
	TArray<TObjectPtr<UVoxelVolumeLayer>> VolumeAdditionalLayers;

	// If false, this stamp will only apply on parts where another stamp has been applied first
	// This is useful to avoid having stamps going beyond world bounds
	// Only used if BlendMode is not Override nor Intersect
	UPROPERTY(EditAnywhere, Category = "Volume Defaults", DisplayName = "Apply On Void", AdvancedDisplay)
	bool bVolumeApplyOnVoid = true;

	// By how much to extend the bounds, relative to the bounds size
	// Increase this if you are using a high smoothness
	// Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	UPROPERTY(EditAnywhere, Category = "Volume Defaults", AdvancedDisplay, meta = (UIMin = 0, UIMax = 5))
	float BoundsExtensionMultiplier = 1.f;

	// Bounds Extension will be limited to this value in cm, regardless of what the multiplier is set to
	UPROPERTY(EditAnywhere, Category = "Volume Defaults", AdvancedDisplay, meta = (ClampMin = 0))
	float MaximumBoundsExtension = 1000.f;

public:
	void ApplyOnStamp(const FVoxelStampRef& StampRef) const;
};

class FVoxelPlaceStampDefaultsCustomization : public IDetailCustomization
{
public:
	FVoxelPlaceStampDefaultsCustomization(const FName ActiveTabName)
		: ActiveTabName(ActiveTabName)
	{}

public:
	//~ Begin IDetailCustomization Interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	//~ End IDetailCustomization Interface

private:
	FName ActiveTabName;
};