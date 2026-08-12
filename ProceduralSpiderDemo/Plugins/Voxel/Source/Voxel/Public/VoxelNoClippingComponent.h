// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "VoxelNoClippingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVoxelOnTeleported, FVector, InvalidLocation, FVector, CorrectedLocation, float, DistanceValueFromQuery);

UCLASS(ClassGroup = Voxel, HideCategories = ("Physics", "LOD", "Activation", "Collision", "Cooking", "AssetUserData"), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelNoClippingComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UVoxelNoClippingComponent();

protected:
	//~ Begin USceneComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End USceneComponent Interface

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelStackLayer Layer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bAutoAdjustPlayer = true;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FVoxelOnTeleported OnTeleported;

private:
	void Execute(const AActor* Owner, float Distance);

private:
	TOptional<TVoxelFuture<FVoxelFloatBuffer>> Future;
	FVector CheckLocation = FVector::ZeroVector;
	FVector LastCorrectLocation = FVector::ZeroVector;
};