// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/Volume/Tools/VoxelVolumeTool.h"
#include "Sculpt/Volume/Tools/VoxelAngleVolumeModifier.h"
#include "VoxelAngleVolumeTool.generated.h"

class AStaticMeshActor;

UCLASS(DisplayName = "Angle", meta = (Order = 3, StyleSet = "VoxelStyle", Icon = "Voxel.AngleTool"))
class VOXEL_API UVoxelAngleVolumeTool : public UVoxelVolumeTool
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float Radius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EVoxelSDFMergeMode MergeMode = EVoxelSDFMergeMode::Intersection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (UIMin = 0, UIMax = 1))
	float Strength = 1.f;

	// If true, the plane used for flatten will be the same while clicking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bFreezeOnClick = false;

	// Use Average Position & Normal
	// If true, use linetraces to find average position/normal under the cursor
	// If false, use a single linetrace from the cursor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bUseAverage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (InlineEditConditionToggle))
	bool bUseFixedRotation = false;

	// Override the normal by a fixed rotation
	// The rotation is apply to Up Vector to find the plane normal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (EditCondition = "bUseFixedRotation"))
	FRotator FixedRotation { ForceInit };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ShowOnlyInnerProperties))
	FVoxelToolBrushBase Brush;

public:
	//~ Begin UVoxelTool Interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick() override;

	virtual void OnEditBegin() override;

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

	bool bHasStoredValue = false;
	FVector LastClickFlattenPosition = FVector::ZeroVector;
	FVector LastClickFlattenNormal = FVector::UpVector;
};