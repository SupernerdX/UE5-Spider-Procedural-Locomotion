// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNavigationMeshInterface.h"
#include "VoxelNavigationComponent.generated.h"

class FVoxelNavigationMesh;

UCLASS()
class VOXEL_API UVoxelNavigationComponent final
	: public UPrimitiveComponent
	, public IVoxelNavigationMeshInterface
{
	GENERATED_BODY()

public:
	UVoxelNavigationComponent();

public:
	//~ Begin UPrimitiveComponent Interface
	virtual void UpdateNavigationMesh() override;
	virtual bool ShouldCreatePhysicsState() const override { return false; }
	virtual bool IsNavigationRelevant() const override;
	virtual bool DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface
};