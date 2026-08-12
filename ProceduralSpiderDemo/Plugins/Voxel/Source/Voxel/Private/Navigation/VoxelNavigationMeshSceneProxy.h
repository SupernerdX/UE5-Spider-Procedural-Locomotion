// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "PrimitiveSceneProxy.h"

class FVoxelNavigationMesh;

class FVoxelNavigationMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	const TSharedRef<const FVoxelNavigationMesh> NavigationMesh;

	explicit FVoxelNavigationMeshSceneProxy(
		const UPrimitiveComponent& Component,
		const TSharedRef<const FVoxelNavigationMesh>& NavigationMesh);

public:
	//~ Begin FPrimitiveSceneProxy Interface
	virtual void DestroyRenderThreadResources() override;
	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override;

	virtual bool CanBeOccluded() const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual uint32 GetMemoryFootprint() const override;
	virtual SIZE_T GetTypeHash() const override;
	//~ End FPrimitiveSceneProxy Interface

private:
	mutable TSharedPtr<class FVoxelNavigationMeshRenderData> RenderData;
};