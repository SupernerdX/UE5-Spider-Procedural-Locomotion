// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "RayTracingGeometry.h"
#include "PrimitiveSceneProxy.h"

class UVoxelMeshComponent;
class FVoxelMeshRenderProxy;

class FVoxelMeshSceneProxy final : public FPrimitiveSceneProxy
{
public:
	const TSharedRef<FVoxelMaterialRef> MaterialRef;
	const TSharedRef<FVoxelMaterialRef> LumenMaterialRef;
	const TSharedRef<FVoxelMeshRenderProxy> RenderProxy;

	explicit FVoxelMeshSceneProxy(const UVoxelMeshComponent& Component);

	//~ Begin FPrimitiveSceneProxy Interface
	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override;

	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override;

	virtual void GetDistanceFieldAtlasData(
		const FDistanceFieldVolumeData*& OutDistanceFieldData,
		float& SelfShadowBias) const override;

	virtual const FCardRepresentationData* GetMeshCardRepresentation() const override;
	virtual bool HasDistanceFieldRepresentation() const override;
	virtual bool HasDynamicIndirectShadowCasterRepresentation() const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual bool CanBeOccluded() const override;
	virtual uint32 GetMemoryFootprint() const override;
	virtual SIZE_T GetTypeHash() const override;

#if RHI_RAYTRACING
	virtual bool IsRayTracingRelevant() const override;
	virtual bool IsRayTracingStaticRelevant() const override;
	virtual bool HasRayTracingRepresentation() const override;

	virtual ERayTracingPrimitiveFlags GetCachedRayTracingInstance(FRayTracingInstance& RayTracingInstance) override;
	virtual TArray<FRayTracingGeometry*> GetStaticRayTracingGeometries() const override;
	virtual RayTracing::UE_506_SWITCH(GeometryGroupHandle, FGeometryGroupHandle) GetRayTracingGeometryGroupHandle() const override;
#endif
	//~ End FPrimitiveSceneProxy Interface
private:
	void DrawMesh(
		FMeshBatch& MeshBatch,
		bool bUseLumenMaterial) const;

	bool ShouldUseStaticPath(const FSceneViewFamily& ViewFamily) const;
};