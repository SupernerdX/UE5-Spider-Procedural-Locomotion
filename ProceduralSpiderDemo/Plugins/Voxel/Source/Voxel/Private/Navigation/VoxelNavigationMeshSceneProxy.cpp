// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Navigation/VoxelNavigationMeshSceneProxy.h"
#include "Navigation/VoxelNavigationMesh.h"
#include "MeshBatch.h"
#include "RawIndexBuffer.h"
#include "SceneView.h"
#include "SceneManagement.h"
#include "LocalVertexFactory.h"
#include "Materials/Material.h"
#include "Rendering/ColorVertexBuffer.h"
#include "Rendering/PositionVertexBuffer.h"
#include "Rendering/StaticMeshVertexBuffer.h"

class FVoxelNavigationMeshRenderData
{
public:
	explicit FVoxelNavigationMeshRenderData(const FVoxelNavigationMesh& NavigationMesh)
	{
		VOXEL_FUNCTION_COUNTER();

		IndexBuffer.SetIndices(ReinterpretCastRef<TArray<uint32>>(NavigationMesh.Indices), EIndexBufferStride::Force32Bit);
		PositionVertexBuffer.Init(NavigationMesh.Vertices.Num(), false);
		StaticMeshVertexBuffer.Init(NavigationMesh.Vertices.Num(), 1, false);
		ColorVertexBuffer.Init(NavigationMesh.Vertices.Num(), false);

		for (int32 Index = 0; Index < NavigationMesh.Vertices.Num(); Index++)
		{
			PositionVertexBuffer.VertexPosition(Index) = NavigationMesh.Vertices[Index] + FVector3f(0, 0, 0.1f);
			StaticMeshVertexBuffer.SetVertexUV(Index, 0, FVector2f::ZeroVector);
			ColorVertexBuffer.VertexColor(Index) = FColor::Green;
			StaticMeshVertexBuffer.SetVertexTangents(Index, FVector3f::ForwardVector, FVector3f::RightVector, FVector3f::UpVector);
		}

		FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
		IndexBuffer.InitResource(RHICmdList);
		PositionVertexBuffer.InitResource(RHICmdList);
		StaticMeshVertexBuffer.InitResource(RHICmdList);
		ColorVertexBuffer.InitResource(RHICmdList);

		VertexFactory = MakeUnique<FLocalVertexFactory>(GMaxRHIFeatureLevel, "FVoxelNavigationMesh_RenderData");

		FLocalVertexFactory::FDataType Data;
		PositionVertexBuffer.BindPositionVertexBuffer(VertexFactory.Get(), Data);
		StaticMeshVertexBuffer.BindTangentVertexBuffer(VertexFactory.Get(), Data);
		StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(VertexFactory.Get(), Data);
		ColorVertexBuffer.BindColorVertexBuffer(VertexFactory.Get(), Data);

		VertexFactory->SetData(RHICmdList, Data);
		VertexFactory->InitResource(RHICmdList);
	}
	~FVoxelNavigationMeshRenderData()
	{
		VOXEL_FUNCTION_COUNTER();

		IndexBuffer.ReleaseResource();
		PositionVertexBuffer.ReleaseResource();
		StaticMeshVertexBuffer.ReleaseResource();
		ColorVertexBuffer.ReleaseResource();
		VertexFactory->ReleaseResource();
	}

	void Draw_RenderThread(FMeshBatch& MeshBatch) const
	{
		VOXEL_FUNCTION_COUNTER();

		MeshBatch.Type = PT_TriangleList;
		MeshBatch.VertexFactory = VertexFactory.Get();

		FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
		BatchElement.IndexBuffer = &IndexBuffer;
		BatchElement.FirstIndex = 0;
		BatchElement.NumPrimitives = IndexBuffer.GetNumIndices() / 3;
		BatchElement.MinVertexIndex = 0;
		BatchElement.MaxVertexIndex = PositionVertexBuffer.GetNumVertices() - 1;
	}

private:
	FRawStaticIndexBuffer IndexBuffer{ false };
	FPositionVertexBuffer PositionVertexBuffer;
	FStaticMeshVertexBuffer StaticMeshVertexBuffer;
	FColorVertexBuffer ColorVertexBuffer;
	TUniquePtr<FLocalVertexFactory> VertexFactory;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelNavigationMeshSceneProxy::FVoxelNavigationMeshSceneProxy(
	const UPrimitiveComponent& Component,
	const TSharedRef<const FVoxelNavigationMesh>& NavigationMesh)
	: FPrimitiveSceneProxy(&Component)
	, NavigationMesh(NavigationMesh)
{
	// We create render data on-demand, can't be on a background thread
	bSupportsParallelGDME = false;

	bVerifyUsedMaterials = false;
}

void FVoxelNavigationMeshSceneProxy::DestroyRenderThreadResources()
{
	VOXEL_FUNCTION_COUNTER();
	RenderData.Reset();
}

void FVoxelNavigationMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, const uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!RenderData)
	{
		RenderData = MakeShared<FVoxelNavigationMeshRenderData>(*NavigationMesh);
	}

	const UMaterial* Material = GEngine->WireframeMaterial;
	if (!ensure(Material))
	{
		return;
	}

	const FMaterialRenderProxy* MaterialProxy = Material->GetRenderProxy();
	if (!ensure(MaterialProxy))
	{
		return;
	}

	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (!(VisibilityMap & (1 << ViewIndex)))
		{
			continue;
		}

		ensure(Views[ViewIndex]->Family->EngineShowFlags.Navigation);

		FMeshBatch& MeshBatch = Collector.AllocateMesh();
		MeshBatch.MaterialRenderProxy = MaterialProxy;
		MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
		MeshBatch.bDisableBackfaceCulling = true;
		MeshBatch.DepthPriorityGroup = SDPG_World;
		RenderData->Draw_RenderThread(MeshBatch);

		Collector.AddMesh(ViewIndex, MeshBatch);
	}
}

bool FVoxelNavigationMeshSceneProxy::CanBeOccluded() const
{
	return false;
}

FPrimitiveViewRelevance FVoxelNavigationMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = true;
	Result.bRenderInMainPass = true;
	Result.bDynamicRelevance = View->Family->EngineShowFlags.Navigation;
	return Result;
}

uint32 FVoxelNavigationMeshSceneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

SIZE_T FVoxelNavigationMeshSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}