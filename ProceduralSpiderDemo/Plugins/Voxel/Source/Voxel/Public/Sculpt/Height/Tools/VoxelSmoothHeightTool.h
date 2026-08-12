// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/Height/Tools/VoxelHeightTool.h"
#include "VoxelSmoothHeightTool.generated.h"

class AStaticMeshActor;

UCLASS(DisplayName = "Smooth", meta = (Order = 2, Icon = "LandscapeEditor.SmoothTool"))
class VOXEL_API UVoxelSmoothHeightTool : public UVoxelHeightTool
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float Radius = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (UIMin = 0, UIMax = 1))
	float Strength = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ShowOnlyInnerProperties))
	FVoxelToolBrushBase Brush;

	//~ Begin UVoxelTool Interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick() override;
	//~ End UVoxelTool Interface

	//~ Begin UVoxelHeightTool Interface
	virtual TSharedPtr<FVoxelHeightModifier> GetModifier(float StrengthMultiplier) const override;
	//~ End UVoxelHeightTool Interface

private:
	UPROPERTY(Transient)
	TObjectPtr<AStaticMeshActor> PreviewActor;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;
};