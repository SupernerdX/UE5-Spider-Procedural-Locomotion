// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLayeredMaterial.generated.h"

class FVoxelLayeredMaterialProxy;

USTRUCT()
struct VOXEL_API FVoxelLayeredMaterialLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (Units = degrees, UIMin = 0, UIMax = 180))
	float MinAngle = 0;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (UIMin = 0, UIMax = 1))
	float MinFalloff = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (Units = degrees, UIMin = 0, UIMax = 180))
	float MaxAngle = 180.f;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (UIMin = 0, UIMax = 1))
	float MaxFalloff = 0.1f;
};

UCLASS()
class VOXEL_API UVoxelLayeredMaterial : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelLayeredMaterialLayer> Layers;

	UVoxelLayeredMaterial();

private:
	mutable TSharedPtr<FVoxelLayeredMaterialProxy> Proxy;
};