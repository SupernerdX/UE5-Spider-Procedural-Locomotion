// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/Tools/VoxelVolumeTool.h"
#include "Sculpt/Volume/Tools/VoxelFlattenVolumeModifier.h"
#include "VoxelFlattenVolumeTool.generated.h"

class AStaticMeshActor;

UCLASS(DisplayName = "Flatten", meta = (Order = 4, Icon = "LandscapeEditor.FlattenTool"))
class VOXEL_API UVoxelFlattenVolumeTool : public UVoxelVolumeTool
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float Radius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (UIMin = "0", UIMax = "10000"))
	float Height = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (UIMin = "0", UIMax = "1"))
	float Falloff = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EVoxelLevelToolType Type = EVoxelLevelToolType::Additive;

	// If true, the plane used for flatten will be the same while clicking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bFreezeOnClick = true;

	// Use Average Position & Normal
	// If true, use linetraces to find average position/normal under the cursor
	// If false, use a single linetrace from the cursor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bUseAverage = true;

	// Flattens to the angle of the clicked point, instead of horizontal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bUseSlopeFlatten = false;

	// Number of Rays to calculate average position and slope
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", AdvancedDisplay)
	int32 NumRays = 100;

	//~ Begin UVoxelTool Interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void OnEditBegin() override;
	virtual void OnEditEnd() override;

	virtual bool PrepareModifierData() override;
	//~ End UVoxelTool Interface

	//~ Begin UVoxelVolumeTool Interface
	virtual TSharedPtr<FVoxelVolumeModifier> GetModifier(float StrengthMultiplier) const override;
	//~ End UVoxelVolumeTool Interface

private:
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;

	FVector Position = FVector(ForceInit);
	FVector Normal = FVector::UpVector;
	TOptional<FPlane> FirstHitPlane;
};