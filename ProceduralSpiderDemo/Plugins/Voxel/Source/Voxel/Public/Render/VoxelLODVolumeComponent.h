// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLODVolumeComponent.generated.h"

UENUM()
enum class EVoxelLODInvokerType
{
	Sphere,
	Box
};

UCLASS(ClassGroup = Voxel, HideCategories = ("Physics", "LOD", "Activation", "Collision", "Cooking", "AssetUserData"), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelLODVolumeComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker", meta = (ClampMin = 0, ClampMax = 30))
	int32 MinLOD = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker")
	EVoxelLODInvokerType Type = EVoxelLODInvokerType::Sphere;

	// In world space, not affected by scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker", meta = (EditCondition = "Type == EVoxelLODInvokerType::Sphere", EditConditionHides))
	float SphereRadius = 1000.f;

	// In world space, not affected by scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker", meta = (EditCondition = "Type == EVoxelLODInvokerType::Box", EditConditionHides))
	FVector BoxExtent = FVector(1000.f);

	// How much this component needs to be moved before it triggers an invalidation (re-evaluating the LOD state for all chunks).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker", AdvancedDisplay)
	float PositionPrecision = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker", AdvancedDisplay, meta = (EditCondition = "Type == EVoxelLODInvokerType::Box", EditConditionHides))
	float RotationPrecision = 1.f;

	// Color used to draw the shape.
	UPROPERTY(EditAnywhere, Category = "Invoker", AdvancedDisplay)
	FColor ShapeColor = FColor(223, 149, 157, 255);

	// Only show this component if the actor is selected
	UPROPERTY(EditAnywhere, Category = "Invoker", AdvancedDisplay)
	bool bDrawOnlyIfSelected = false;

	// Used to control the line thickness when rendering
	UPROPERTY(EditAnywhere, Category = "Invoker", AdvancedDisplay)
	float LineThickness = 0.f;

public:
	UVoxelLODVolumeComponent();

	//~ Begin USceneComponent Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface
};