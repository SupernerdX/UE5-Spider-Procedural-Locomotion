// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelScatterInvokerComponent.generated.h"

UCLASS(ClassGroup = Voxel, HideCategories = ("Physics", "LOD", "Activation", "Collision", "Cooking", "AssetUserData"), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelScatterInvokerComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker", AdvancedDisplay)
	float PositionPrecision = 100.f;

public:
	UVoxelScatterInvokerComponent();

	//~ Begin USceneComponent Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//~ End USceneComponent Interface
};