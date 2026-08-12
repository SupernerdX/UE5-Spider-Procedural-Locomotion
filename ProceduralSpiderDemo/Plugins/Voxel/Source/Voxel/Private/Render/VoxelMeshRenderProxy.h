// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "MeshCardBuild.h"
#include "DistanceFieldAtlas.h"
#include "RayTracingGeometry.h"

class FVoxelMesh;
class FVoxelVertexFactory;
struct FVoxelRenderSubsystem;
struct FVoxelChunkNeighborInfo;
struct FVoxelMegaMaterialRenderData;

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelMeshGpuMemory, "Voxel Mesh Memory (GPU)");

class FVoxelMeshRenderProxy : public TSharedFromThis<FVoxelMeshRenderProxy>
{
public:
	const TSharedRef<const FVoxelMesh> Mesh;
	const TSharedRef<const FVoxelMegaMaterialRenderData> MegaMaterialRenderData;
	const bool bRenderInBasePass;
	const bool bEnableLumen;
	const bool bEnableRaytracing;
	const bool bEnableMeshDistanceField;
	const TVoxelArray<TVoxelObjectPtr<URuntimeVirtualTexture>> RuntimeVirtualTextures;

	// TODO?
	const float SelfShadowBias = 0.f;

	VOXEL_COUNT_INSTANCES();
	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelMeshGpuMemory);

	FVoxelMeshRenderProxy(
		const TSharedRef<const FVoxelMesh>& Mesh,
		const TSharedRef<const FVoxelMegaMaterialRenderData>& MegaMaterialRenderData,
		bool bRenderInBasePass,
		bool bEnableLumen,
		bool bEnableRaytracing,
		bool bEnableMeshDistanceField,
		const TVoxelArray<TVoxelObjectPtr<URuntimeVirtualTexture>>& RuntimeVirtualTextures,
		const FVoxelChunkNeighborInfo& NeighborInfo);
	~FVoxelMeshRenderProxy();

public:
	int64 GetAllocatedSize() const;
	FVoxelFuture Initialize_AsyncThread(const FVoxelRenderSubsystem& Subsystem);

	// Needs the buffer pools to have updated their textures - called after Render
	void InitializeVertexFactory_RenderThread(FRHICommandListBase& RHICmdList);

private:
	bool bIsFinalized = false;

	TVoxelArray<FVector4f> Vertices;

	TUniquePtr<FIndexBuffer> IndicesBuffer;
	TUniquePtr<FShaderResourceViewRHIRef> IndicesSRV;
	TUniquePtr<FVertexBufferWithSRV> VerticesBuffer;
	TUniquePtr<FVertexBufferWithSRV> NormalsBuffer;
	TUniquePtr<FVoxelVertexFactory> VertexFactory;

#if RHI_RAYTRACING
	RayTracing::UE_506_SWITCH(GeometryGroupHandle, FGeometryGroupHandle) RayTracingGeometryGroupHandle = -1;
    TUniquePtr<FRayTracingGeometry> RayTracingGeometry;
#endif

	TUniquePtr<FCardRepresentationData> CardRepresentationData;
	TSharedPtr<FDistanceFieldVolumeData> DistanceFieldVolumeData;

	friend class FVoxelMeshSceneProxy;

	void Initialize_RenderThread(
		FRHICommandListBase& RHICmdList,
		const FVoxelRenderSubsystem& Subsystem);
};