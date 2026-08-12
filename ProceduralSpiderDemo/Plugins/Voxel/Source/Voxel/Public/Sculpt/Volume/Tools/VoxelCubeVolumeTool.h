// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/Tools/VoxelVolumeTool.h"
#include "VoxelCubeVolumeTool.generated.h"

class AStaticMeshActor;

UCLASS(DisplayName = "Cube", meta = (Order = 2, Icon = "LandscapeEditor.GizmoBrush"))
class VOXEL_API UVoxelCubeVolumeTool : public UVoxelVolumeTool
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = 0))
	FVector Size = FVector(1000.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (UIMin = 0, UIMax = 1))
	float Roundness = 0.f;

	// Relative to radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (UIMin = 0, UIMax = 1))
	float Smoothness = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector Offset = FVector::ZeroVector;

	//~ Begin UVoxelTool Interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Tick() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UVoxelTool Interface

	//~ Begin UVoxelVolumeTool Interface
	virtual TSharedPtr<FVoxelVolumeModifier> GetModifier(float StrengthMultiplier) const override;
	//~ End UVoxelVolumeTool Interface

private:
	UPROPERTY()
	TObjectPtr<AStaticMeshActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;
};