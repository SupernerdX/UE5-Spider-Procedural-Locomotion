// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/VoxelToolBrush.h"
#include "Sculpt/Volume/Tools/VoxelVolumeTool.h"
#include "VoxelMetadataOverrides.h"
#include "Surface/VoxelSurfaceTypePicker.h"
#include "VoxelPaintVolumeTool.generated.h"

class AStaticMeshActor;
class UVoxelSurfaceTypeInterface;

UCLASS(DisplayName = "Paint", meta = (Order = 0, Icon = "LandscapeEditor.PaintTool"))
class VOXEL_API UVoxelPaintVolumeTool : public UVoxelVolumeTool
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelSurfaceTypePicker SurfaceTypeToPaint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelMetadataOverrides MetadatasToPaint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float Radius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (UIMin = 0, UIMax = 1))
	float Strength = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ShowOnlyInnerProperties))
	FVoxelToolBrushBase Brush;

	//~ Begin UVoxelTool Interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick() override;

	virtual TSharedPtr<FVoxelVolumeModifier> GetModifier(float StrengthMultiplier) const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UVoxelTool Interface

private:
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;
};