// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/VoxelLevelToolType.h"
#include "Sculpt/Height/Tools/VoxelHeightTool.h"
#include "VoxelFlattenHeightTool.generated.h"

class AStaticMeshActor;

UCLASS(DisplayName = "Flatten", meta = (Order = 4, Icon = "LandscapeEditor.FlattenTool"))
class VOXEL_API UVoxelFlattenHeightTool : public UVoxelHeightTool
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float Radius = 1000.f;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ShowOnlyInnerProperties))
	FVoxelToolBrushBase Brush;

	// Number of Rays to calculate average position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", AdvancedDisplay)
	int32 NumRays = 100;

	//~ Begin UVoxelTool Interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick() override;

	virtual void OnEditBegin() override;
	virtual void OnEditEnd() override;

	virtual bool PrepareModifierData() override;
	//~ End UVoxelTool Interface

	//~ Begin UVoxelHeightTool Interface
	virtual TSharedPtr<FVoxelHeightModifier> GetModifier(float StrengthMultiplier) const override;
	//~ End UVoxelHeightTool Interface

private:
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;

	TOptional<float> FirstHeight;
	FVector FlattenPosition;
};