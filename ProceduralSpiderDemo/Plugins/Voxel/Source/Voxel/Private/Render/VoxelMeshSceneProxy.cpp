// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelMeshSceneProxy.h"
#include "Render/VoxelMeshRenderProxy.h"
#include "Render/VoxelMeshComponent.h"
#include "Render/VoxelVertexFactory.h"
#include "Nanite/VoxelNaniteComponent.h"
#include "VoxelMesh.h"

#include "SceneView.h"
#include "MeshBatch.h"
#include "Engine/Engine.h"
#include "MaterialDomain.h"
#include "SceneManagement.h"
#include "PrimitiveSceneInfo.h"
#include "Materials/Material.h"
#include "RayTracingInstance.h"
#include "RayTracingGeometry.h"
#include "RayTracingGeometryManagerInterface.h"

void RefreshVoxelMeshComponents()
{
	ForEachObjectOfClass<UVoxelMeshComponent>([&](UVoxelMeshComponent& Component)
	{
		Component.MarkRenderStateDirty();
	});

	ForEachObjectOfClass<UVoxelNaniteComponent>([&](UVoxelNaniteComponent& Component)
	{
		Component.MarkRenderStateDirty();
	});
}

#if WITH_EDITOR
void FPrimitiveSceneProxy::SetLevelInstanceEditingState_RenderThread(const bool bInEditingState)
{
	bLevelInstanceEditingState = bInEditingState;
}
#endif

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelShowMeshSections, false,
	"voxel.render.ShowMeshSections",
	"If true, will assign a unique color to each mesh section",
	RefreshVoxelMeshComponents);

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelAllowOcclusion, false,
	"voxel.render.AllowOcclusion",
	"Occlusion is disabled by default to avoid gaps when turning the camera or doing large edits",
	RefreshVoxelMeshComponents);

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelDisableRaytracing, false,
	"voxel.render.DisableRaytracing",
	"Force disable raytracing voxel meshes",
	RefreshVoxelMeshComponents);

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelShowRaytracedMeshes, false,
	"voxel.render.ShowRaytracedMeshes",
	"Draw raytraced meshes",
	RefreshVoxelMeshComponents);

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelShowDistanceFieldMeshes, false,
	"voxel.render.ShowDistanceFieldMeshes",
	"Draw distance field meshes",
	RefreshVoxelMeshComponents);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMeshSceneProxy::FVoxelMeshSceneProxy(const UVoxelMeshComponent& Component)
	: FPrimitiveSceneProxy(&Component)
	, MaterialRef(Component.MaterialRef.ToSharedRef())
	, LumenMaterialRef(Component.LumenMaterialRef.ToSharedRef())
	, RenderProxy(Component.RenderProxy.ToSharedRef())
{
	ensure(Component.bAffectDynamicIndirectLighting == RenderProxy->bEnableLumen);
	ensure(Component.bVisibleInRayTracing == RenderProxy->bEnableRaytracing);
	ensure(Component.bAffectDistanceFieldLighting == RenderProxy->bEnableMeshDistanceField);
	ensure(Component.bAffectIndirectLightingWhileHidden);

	// Mesh can't be deformed, required for VSM caching
	bHasDeformableMesh = false;

	// HasDistanceFieldRepresentation might still return false
	bSupportsDistanceFieldRepresentation = true;

#if VOXEL_ENGINE_VERSION >= 506
	bSupportsRuntimeVirtualTexture = true;
#endif

	if (RenderProxy->bEnableLumen)
	{
		bCastsDynamicIndirectShadow = true;
	}

	UpdateVisibleInLumenScene();
	EnableGPUSceneSupportFlags();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMeshSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
{
	VOXEL_FUNCTION_COUNTER();

	if (RenderProxy->bRenderInBasePass)
	{
		FMeshBatch MeshBatch;
		DrawMesh(MeshBatch, false);

		PDI->DrawMesh(MeshBatch, FLT_MAX);
	}

	if (bVisibleInLumenScene &&
		!RenderProxy->bRenderInBasePass)
	{
		FMeshBatch MeshBatch;
		DrawMesh(MeshBatch, true);

		MeshBatch.CastShadow = false;
		MeshBatch.bUseAsOccluder = false;
		MeshBatch.bUseForDepthPass = false;
		MeshBatch.bDitheredLODTransition = false;
		MeshBatch.bRenderToVirtualTexture = false;

		PDI->DrawMesh(MeshBatch, FLT_MAX);
	}

	if (RuntimeVirtualTextureMaterialTypes.Num() > 0)
	{
		FMeshBatch MeshBatch;
		DrawMesh(MeshBatch, false);

		MeshBatch.CastShadow = false;
		MeshBatch.bUseAsOccluder = false;
		MeshBatch.bUseForDepthPass = false;
		MeshBatch.bUseForMaterial = false;
		MeshBatch.bDitheredLODTransition = false;
		MeshBatch.bRenderToVirtualTexture = true;

		for (const ERuntimeVirtualTextureMaterialType MaterialType : RuntimeVirtualTextureMaterialTypes)
		{
			MeshBatch.RuntimeVirtualTextureMaterialType = uint32(MaterialType);

			PDI->DrawMesh(MeshBatch, FLT_MAX);
		}
	}
}

void FVoxelMeshSceneProxy::GetDynamicMeshElements(
	const TArray<const FSceneView*>& Views,
	const FSceneViewFamily& ViewFamily,
	const uint32 VisibilityMap,
	FMeshElementCollector& Collector) const
{
	VOXEL_FUNCTION_COUNTER();

	const FEngineShowFlags& EngineShowFlags = ViewFamily.EngineShowFlags;

#if UE_ENABLE_DEBUG_DRAWING
	// Render bounds
	{
		VOXEL_SCOPE_COUNTER("Render Bounds");
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				RenderBounds(Collector.GetPDI(ViewIndex), EngineShowFlags, GetBounds(), IsSelected());
			}
		}
	}
#endif

	if (ShouldUseStaticPath(ViewFamily) ||
		EngineShowFlags.Collision ||
		EngineShowFlags.CollisionPawn ||
		EngineShowFlags.CollisionVisibility)
	{
		return;
	}

	if (!RenderProxy->bRenderInBasePass &&
#if RHI_RAYTRACING
		(!GVoxelShowRaytracedMeshes || !IsRayTracingRelevant()) &&
#endif
		(!GVoxelShowDistanceFieldMeshes || !HasDistanceFieldRepresentation()))
	{
		return;
	}

	VOXEL_SCOPE_COUNTER("Render Mesh");

	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (!(VisibilityMap & (1 << ViewIndex)))
		{
			continue;
		}

		FMeshBatch& MeshBatch = Collector.AllocateMesh();
		MeshBatch.bUseWireframeSelectionColoring = IsSelected() && EngineShowFlags.Wireframe; // Else mesh LODs view is messed up when actor is selected
		DrawMesh(MeshBatch, false);

		if (GVoxelShowMeshSections)
		{
			const uint32 Hash = FVoxelUtilities::MurmurHash64(uint64(&RenderProxy.Get()));

			MeshBatch.MaterialRenderProxy = FVoxelUtilities::CreateColoredMaterialRenderProxy(
				Collector,
				ReinterpretCastRef<const FColor>(Hash));
		}

		VOXEL_SCOPE_COUNTER("Collector.AddMesh");
		Collector.AddMesh(ViewIndex, MeshBatch);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMeshSceneProxy::GetDistanceFieldAtlasData(const FDistanceFieldVolumeData*& OutDistanceFieldData, float& SelfShadowBias) const
{
	OutDistanceFieldData = RenderProxy->DistanceFieldVolumeData.Get();
	SelfShadowBias = RenderProxy->SelfShadowBias;
}

const FCardRepresentationData* FVoxelMeshSceneProxy::GetMeshCardRepresentation() const
{
	return RenderProxy->CardRepresentationData.Get();
}

bool FVoxelMeshSceneProxy::HasDistanceFieldRepresentation() const
{
	return RenderProxy->DistanceFieldVolumeData.IsValid();
}

bool FVoxelMeshSceneProxy::HasDynamicIndirectShadowCasterRepresentation() const
{
	return bCastsDynamicIndirectShadow;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FPrimitiveViewRelevance FVoxelMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	const FEngineShowFlags& EngineShowFlags = View->Family->EngineShowFlags;

	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);

	if (ShouldUseStaticPath(*View->Family))
	{
		Result.bStaticRelevance = true;
		Result.bDynamicRelevance = false;
	}
	else
	{
		Result.bStaticRelevance = false;
		Result.bDynamicRelevance = true;
	}

	if (EngineShowFlags.Bounds)
	{
		Result.bDynamicRelevance = true;
	}

#if WITH_EDITOR
	Result.bEditorVisualizeLevelInstanceRelevance = IsEditingLevelInstanceChild();
	Result.bEditorStaticSelectionRelevance = RenderProxy->bRenderInBasePass && (WantsEditorEffects() || IsSelected() || IsHovered());
#endif

#if WITH_EDITOR
	// TODO HACK See https://issues.voxelplugin.com/issue/VP-424/Fix-level-instance-coloring

	Result.bEditorVisualizeLevelInstanceRelevance = true;
	ConstCast(this)->SetLevelInstanceEditingState_RenderThread(true);
#endif

	// Tricky: ShouldRenderInMainPass should still return true to ensure Lumen cards are rendered
	Result.bRenderInMainPass =
		RenderProxy->bRenderInBasePass &&
		ShouldRenderInMainPass();

	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;

	const UMaterialInterface* MaterialObject = MaterialRef->GetMaterial();
	if (ensure(MaterialObject))
	{
		MaterialObject->GetRelevance_Concurrent(UE_507_SWITCH(View->FeatureLevel, GetFeatureLevelShaderPlatform_Checked(View->FeatureLevel))).SetPrimitiveViewRelevance(Result);
	}

	return Result;
}

bool FVoxelMeshSceneProxy::CanBeOccluded() const
{
	if (!RenderProxy->bRenderInBasePass)
	{
		// No need to ever disable occlusion
		return true;
	}

	return GVoxelAllowOcclusion;
}

uint32 FVoxelMeshSceneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

SIZE_T FVoxelMeshSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if RHI_RAYTRACING
bool FVoxelMeshSceneProxy::IsRayTracingRelevant() const
{
	return
		IsVisibleInRayTracing() &&
		!GVoxelDisableRaytracing;
}

bool FVoxelMeshSceneProxy::IsRayTracingStaticRelevant() const
{
	return
		IsVisibleInRayTracing() &&
		!GVoxelDisableRaytracing;
}

bool FVoxelMeshSceneProxy::HasRayTracingRepresentation() const
{
	return
		IsVisibleInRayTracing() &&
		!GVoxelDisableRaytracing;
}

ERayTracingPrimitiveFlags FVoxelMeshSceneProxy::GetCachedRayTracingInstance(FRayTracingInstance& RayTracingInstance)
{
	if (!IsRayTracingRelevant() ||
		!ensure(RenderProxy->RayTracingGeometry))
	{
		return ERayTracingPrimitiveFlags::Exclude;
	}

	RayTracingInstance.Geometry = RenderProxy->RayTracingGeometry.Get();
	RayTracingInstance.bApplyLocalBoundsTransform = false;
	RayTracingInstance.NumTransforms = 1;

#if VOXEL_ENGINE_VERSION < 506
	RayTracingInstance.InstanceLayer = IsRayTracingFarField()
		? ERayTracingInstanceLayer::FarField
		: ERayTracingInstanceLayer::NearField;
#endif

	DrawMesh(RayTracingInstance.Materials.Emplace_GetRef(), true);

	ERayTracingPrimitiveFlags PrimitiveFlags = ERayTracingPrimitiveFlags::CacheInstances;

	if (IsRayTracingFarField())
	{
		PrimitiveFlags |= ERayTracingPrimitiveFlags::FarField;
	}

	return PrimitiveFlags;
}

RayTracing::UE_506_SWITCH(GeometryGroupHandle, FGeometryGroupHandle) FVoxelMeshSceneProxy::GetRayTracingGeometryGroupHandle() const
{
	return RenderProxy->RayTracingGeometryGroupHandle;
}

TArray<FRayTracingGeometry*> FVoxelMeshSceneProxy::GetStaticRayTracingGeometries() const
{
	if (!RenderProxy->RayTracingGeometry)
	{
		return {};
	}

	return TArray<FRayTracingGeometry*>
	{
		RenderProxy->RayTracingGeometry.Get()
	};
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMeshSceneProxy::DrawMesh(
	FMeshBatch& MeshBatch,
	const bool bUseLumenMaterial) const
{
	check(RenderProxy->bIsFinalized);
	check(RenderProxy->IndicesBuffer);
	check(RenderProxy->Mesh->Indices.Num() % 3 == 0);

	const UMaterialInterface* MaterialObject = (bUseLumenMaterial ? LumenMaterialRef : MaterialRef)->GetMaterial();
	if (!ensure(MaterialObject))
	{
		MaterialObject = UMaterial::GetDefaultMaterial(MD_Surface);
	}
	if (!ensure(MaterialObject))
	{
		return;
	}

	const FMaterialRenderProxy* MaterialRenderProxy = MaterialObject->GetRenderProxy();
	ensure(MaterialRenderProxy);

	// Else the virtual texture check fails in RuntimeVirtualTextureRender.cpp:338
	// and the static mesh isn't rendered at all
	MeshBatch.LODIndex = 0;
	MeshBatch.SegmentIndex = 0;
	MeshBatch.MaterialRenderProxy = MaterialRenderProxy;
	MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
	MeshBatch.DepthPriorityGroup = SDPG_World;
	MeshBatch.bCanApplyViewModeOverrides = true;
	MeshBatch.Type = PT_TriangleList;
#if RHI_RAYTRACING
	MeshBatch.CastRayTracedShadow = CastsDynamicShadow();
#endif
	MeshBatch.VertexFactory = RenderProxy->VertexFactory.Get();

	FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
	BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();
	BatchElement.IndexBuffer = RenderProxy->IndicesBuffer.Get();
	BatchElement.FirstIndex = 0;
	BatchElement.NumPrimitives = RenderProxy->Mesh->Indices.Num() / 3;
	BatchElement.MinVertexIndex = 0;
	BatchElement.MaxVertexIndex = RenderProxy->Mesh->Indices.Num() - 1;

#if UE_ENABLE_DEBUG_DRAWING
	MeshBatch.VisualizeLODIndex = RenderProxy->Mesh->ChunkLOD % GEngine->LODColorationColors.Num();
#endif
}

bool FVoxelMeshSceneProxy::ShouldUseStaticPath(const FSceneViewFamily& ViewFamily) const
{
	return
		!IsRichView(ViewFamily) &&
		!GVoxelShowMeshSections &&
		!GVoxelShowRaytracedMeshes &&
		!GVoxelShowDistanceFieldMeshes;
}