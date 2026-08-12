// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelCollisionComponent.generated.h"

class FVoxelCollider;

UCLASS()
class VOXEL_API UVoxelCollisionComponent final : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UVoxelCollisionComponent();

	TSharedPtr<const FVoxelCollider> GetCollider() const
	{
		return Collider;
	}

	void SetCollider(
		const TSharedRef<const FVoxelCollider>& NewCollider,
		const FBodyInstance& NewBodyInstance,
		bool bDoubleSidedGeometry,
		bool bInGenerateOverlapEvents,
		const FTransform& RelativeTransform);

	void ClearCollider();

public:
	//~ Begin UPrimitiveComponent Interface
	virtual UBodySetup* GetBodySetup() override { return BodySetup; }
	virtual bool ShouldCreatePhysicsState() const override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual int32 GetNumMaterials() const override;
	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const override;
	//~ End UPrimitiveComponent Interface

private:
	UPROPERTY(Transient)
	TObjectPtr<UBodySetup> BodySetup;

	TSharedPtr<const FVoxelCollider> Collider;
};