// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "VoxelStaticMeshCollisionComponent.generated.h"

class FVoxelCollider;

UCLASS()
class VOXEL_API UVoxelStaticMeshCollisionComponent final : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UVoxelStaticMeshCollisionComponent();

	TSharedPtr<const FVoxelCollider> GetCollider() const
	{
		return Collider;
	}

	void SetBodyInstance(const FBodyInstance& NewBodyInstance);
	void SetCollider(
		const TSharedPtr<const FVoxelCollider>& NewCollider,
		bool bDoubleSidedGeometry,
		bool bInGenerateOverlapEvents);

public:
	//~ Begin UPrimitiveComponent Interface
	virtual UBodySetup* GetBodySetup() override { return BodySetup; }
	virtual bool ShouldCreatePhysicsState() const override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface

private:
	UPROPERTY(Transient)
	TObjectPtr<UBodySetup> BodySetup;

	TSharedPtr<const FVoxelCollider> Collider;
};