// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelDeveloperSettings.h"
#include "VoxelLayer.h"
#include "VoxelSettings.generated.h"

USTRUCT()
struct VOXEL_API FVoxelInlineCurveTemplate
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FString DisplayNameOverride;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FSoftObjectPath CurveAsset;
};

UCLASS(config = Engine, DefaultConfig, meta = (DisplayName = "Voxel Plugin"))
class VOXEL_API UVoxelSettings : public UVoxelDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Config")
	bool bEnableAutoStampActorLabeling = true;

	// Voxel Stamp Actor label format
	// Type - Stamp Type;
	// Identifier - Name of related object (mesh stamp - mesh name, graph stamp - graph name);
	// BlendMode;
	// Layer - First layer name;
	// Priority;
	UPROPERTY(Config, EditAnywhere, Category = "Config", meta = (EditCondition = "bEnableAutoStampActorLabeling"))
	FString StampActorLabelFormat = "{Identifier} {BlendMode} {Layer} {Priority} {Type}";

	UPROPERTY(Config, EditAnywhere, Category = "Config")
	bool bDisableMegaMaterialCacheAutoSave = false;

	// Disables automatic MegaMaterial recompilation when materials or surface types change
	UPROPERTY(Config, EditAnywhere, Category = "Config")
	bool bDisableMegaMaterialAutoRecompile = false;

	// Will disable stamp priority calculation, when spawning or duplicating stamps
	UPROPERTY(Config, EditAnywhere, Category = "Config")
	bool bDisableAutomaticPriority = false;

	UPROPERTY(Config, EditAnywhere, Category = "Config")
	bool bRegenerateSeedOnStampDuplicate = true;

	UPROPERTY(Config, EditAnywhere, Category = "Layers")
	TSoftObjectPtr<UVoxelHeightLayer> DefaultHeightLayer = UVoxelHeightLayer::Default();

	UPROPERTY(Config, EditAnywhere, Category = "Layers")
	TSoftObjectPtr<UVoxelVolumeLayer> DefaultVolumeLayer = UVoxelVolumeLayer::Default();

	UPROPERTY(Config, EditAnywhere, Category = "Curve Editor")
	TArray<FVoxelInlineCurveTemplate> CurveTemplates =
	{
		{ "", FSoftObjectPath("/Script/Engine.CurveFloat'/Voxel/CurveTemplates/LinearRampUp.LinearRampUp'") },
		{ "", FSoftObjectPath("/Script/Engine.CurveFloat'/Voxel/CurveTemplates/LinearRampDown.LinearRampDown'") },
		{ "", FSoftObjectPath("/Script/Engine.CurveFloat'/Voxel/CurveTemplates/LinearRampDownUp.LinearRampDownUp'") },
		{ "", FSoftObjectPath("/Script/Engine.CurveFloat'/Voxel/CurveTemplates/DropOff.DropOff'") },
		{ "", FSoftObjectPath("/Script/Engine.CurveFloat'/Voxel/CurveTemplates/EaseIn.EaseIn'") },
		{ "", FSoftObjectPath("/Script/Engine.CurveFloat'/Voxel/CurveTemplates/PulseOut.PulseOut'") },
		{ "", FSoftObjectPath("/Script/Engine.CurveFloat'/Voxel/CurveTemplates/SmoothRampUp.SmoothRampUp'") },
		{ "", FSoftObjectPath("/Script/Engine.CurveFloat'/Voxel/CurveTemplates/SmoothRampDown.SmoothRampDown'") },
		{ "", FSoftObjectPath("/Script/Engine.CurveFloat'/Voxel/CurveTemplates/RampUpDown.RampUpDown'") },
	};

	UVoxelSettings();
};