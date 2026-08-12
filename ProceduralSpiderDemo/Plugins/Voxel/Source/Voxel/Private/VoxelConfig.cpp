// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelConfig.h"
#include "VoxelWorld.h"
#include "NavigationSystem.h"
#include "Engine/RendererSettings.h"
#include "MegaMaterial/VoxelMaterialHook.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"
#include "Collision/VoxelCollisionChannels.h"
#include "VT/RuntimeVirtualTexture.h"

extern int32 GVoxelMegaMaterialDebugMode;
constexpr uint32 GVoxelMagicNumber = 0x2F66ED00u;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelWorldManager
{
public:
	int32 GetId(const TVoxelObjectPtr<const AVoxelWorld>& VoxelWorld)
	{
		if (const int32* IndexPtr = VoxelWorldToIndex.Find(VoxelWorld))
		{
			return *IndexPtr;
		}

		TVoxelSet<int32> UsedIndices;
		for (auto It = VoxelWorldToIndex.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid_Slow())
			{
				It.RemoveCurrent();
				continue;
			}

			UsedIndices.Add(It.Value());
		}

		for (int32 Index = 0; Index < 256; Index++)
		{
			if (UsedIndices.Contains(Index))
			{
				continue;
			}

			VoxelWorldToIndex.Add_EnsureNew(VoxelWorld, Index);
			return Index;
		}

		ensure(false);
		return 0;
	}

private:
	TVoxelMap<TVoxelObjectPtr<const AVoxelWorld>, int32> VoxelWorldToIndex;
};
FVoxelWorldManager* GVoxelWorldManager = new FVoxelWorldManager();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelConfig::FVoxelConfig(const AVoxelWorld& VoxelWorld)
	: FVoxelConfig(*VoxelWorld.GetWorld(), VoxelWorld)
{
}

FVoxelConfig::FVoxelConfig(
	UWorld& World,
	const AVoxelWorld& VoxelWorld)
	: World(&World)
	, VoxelWorldId(GVoxelMagicNumber | GVoxelWorldManager->GetId(VoxelWorld))
	, VoxelWorld(&VoxelWorld)
	, VoxelWorldObject(&VoxelWorld)
	, bIsEditorWorld(!World.IsGameWorld())
	, bUseCameraInvoker(INLINE_LAMBDA
	{
#if WITH_EDITOR
		if (VoxelWorld.bUseCameraAsInvoker)
		{
			return true;
		}

		if (!World.IsGameWorld())
		{
			return !VoxelWorld.bDisableCameraInvokerInEditor;
		}

		return false;
#else
		return VoxelWorld.bUseCameraAsInvoker;
#endif
	})
	, CameraInvokerPositionPrecision(VoxelWorld.CameraInvokerPositionPrecision)
	, FeatureLevel(World.GetFeatureLevel())

	, LocalToWorld(VoxelWorld.ActorToWorld())
	, LocalToWorld2D(FVoxelUtilities::MakeTransform2(LocalToWorld))

	, VoxelSize(FMath::Max(VoxelWorld.VoxelSize, 1))
	, RenderChunkSize(INLINE_LAMBDA
	{
		switch (VoxelWorld.RenderChunkSize)
		{
		default: ensure(false);
		case EVoxelRenderChunkSize::Size32: return 32;
		case EVoxelRenderChunkSize::Size64: return 64;
		case EVoxelRenderChunkSize::Size128: return 128;
		case EVoxelRenderChunkSize::Size256: return 256;
		}
	})
	, LODQuality(VoxelWorld.LODQuality)
	, QualityExponent(VoxelWorld.bEnableCubicMesher ? 0.01 : VoxelWorld.QualityExponent)
	, MegaMaterialProxy(INLINE_LAMBDA
	{
		UVoxelMegaMaterial* MegaMaterial = VoxelWorld.MegaMaterial;
		if (!MegaMaterial)
		{
			return FVoxelMegaMaterialProxy::Default();
		}
		return MegaMaterial->GetProxy();
	})
	, LayerToRender(FVoxelStackLayer(VoxelWorld.LayerStack, nullptr))
	, bEnableNanite(
		VoxelWorld.bEnableNanite &&
		CanEnableNanite() &&
		GVoxelMegaMaterialDebugMode == 0 &&
		!VoxelWorld.bEnableCubicMesher)
	, bEnableCubicMesher(VoxelWorld.bEnableCubicMesher)
	, MaxLOD(INLINE_LAMBDA
	{
		if (VoxelWorld.bEnableCubicMesher)
		{
			return 0;
		}

		if (!VoxelWorld.bLimitMaxLOD)
		{
			return MAX_int32;
		}

		return VoxelWorld.MaxLOD;
	})
	, MaxBackgroundTasks(VoxelWorld.MaxBackgroundTasks)

	, VisibilityCollision(VoxelWorld.VisibilityCollision)
	, CollisionChunkSize(FMath::Clamp(VoxelWorld.CollisionChunkSize, 1, 64))
	, InvokerCollision(VoxelWorld.InvokerCollision)
	, bDoubleSidedCollision(VoxelWorld.bDoubleSidedCollision)
	, bGenerateOverlapEvents(VoxelWorld.bGenerateOverlapEvents)
	, CollisionVoxelSize(FMath::Max(VoxelWorld.bOverrideCollisionVoxelSize ? VoxelWorld.CollisionVoxelSize : VoxelWorld.VoxelSize, 1))

	, bEnableNavigation(VoxelWorld.bEnableNavigation)
	, NavigationChunkSize(FMath::Clamp(VoxelWorld.NavigationChunkSize, 1, 64))
	, MaxAdditionalNavigationChunks(VoxelWorld.MaxAdditionalNavigationChunks)
	, bGenerateInsideNavMeshBounds(VoxelWorld.bGenerateNavigationInsideNavMeshBounds || !UseNavigationInvokers(World))
	, bOnlyGenerateNavigationInEditor(VoxelWorld.bOnlyGenerateNavigationInEditor)
	, NavigationVoxelSize(FMath::Max(VoxelWorld.bOverrideNavigationVoxelSize ? VoxelWorld.NavigationVoxelSize : VoxelWorld.VoxelSize, 1))
	, NavigationMeshComponentClass(VoxelWorld.NavigationMeshComponent)

	, NaniteMaxTessellationLOD(VoxelWorld.bEnableTessellation ? VoxelWorld.NaniteMaxTessellationLOD : -1)
	, DisplacementFade(VoxelWorld.bEnableDisplacementFade ? VoxelWorld.DisplacementFade : FDisplacementFadeRange::Invalid())
	, NanitePositionPrecision(VoxelWorld.NanitePositionPrecision)
	, bCompressNaniteVertices(VoxelWorld.bCompressNaniteVertices)
	, bUseNaniteBuilder(VoxelWorld.bUseNaniteBuilder)

	, bEnableLumen(VoxelWorld.bEnableLumen && CanEnableLumen())
	, bEnableRaytracing((VoxelWorld.bEnableRaytracing && CanEnableRayTracing()) || (bEnableLumen && UseHardwareRayTracingForLumen()))
	, bGenerateMeshDistanceFields(VoxelWorld.bGenerateMeshDistanceFields || (bEnableLumen && !UseHardwareRayTracingForLumen()))
	, RuntimeVirtualTextures(VoxelWorld.RuntimeVirtualTextures)
	, BlockinessMetadata(VoxelWorld.BlockinessMetadata)
	, RaytracingMaxLOD(VoxelWorld.RaytracingMaxLOD)
	, MeshDistanceFieldMaxLOD(VoxelWorld.MeshDistanceFieldMaxLOD)
	, MeshDistanceFieldBias(VoxelWorld.MeshDistanceFieldBias)
	, MeshDistanceFieldBricksPerChunk(VoxelWorld.MeshDistanceFieldBricksPerChunk)
	, ComponentSettings(VoxelWorld.ComponentSettings)
	, bCacheTextureStreaming(VoxelWorld.bCacheTextureStreaming)
	, bRenderScatterActors(VoxelWorld.bRenderScatterActors)
	, bUseCameraScatterInvoker(INLINE_LAMBDA
	{
	#if WITH_EDITOR
		if (VoxelWorld.bUseCameraAsScatterInvoker)
		{
			return true;
		}

		if (!World.IsGameWorld())
		{
			return !VoxelWorld.bDisableCameraScatterInvokerInEditor;
		}

		return false;
	#else
		return VoxelWorld.bUseCameraAsScatterInvoker;
	#endif
	})
	, CameraScatterInvokerPositionPrecision(VoxelWorld.CameraScatterInvokerPositionPrecision)
{
	if (!VoxelWorld.ActorToWorld().GetScale3D().Equals(FVector::OneVector))
	{
		VOXEL_MESSAGE(Error, "{0}: VoxelWorld should have a uniform scale of 1", VoxelWorld);
	}

#if WITH_EDITOR
	FVoxelMaterialHook::EnsureIsEnabled();
#endif

	if (World.WorldType == EWorldType::Editor ||
		World.WorldType == EWorldType::EditorPreview)
	{
		ConstCast(VisibilityCollision).SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// Needed for foliage painting
		ConstCast(VisibilityCollision).SetResponseToChannel(ECC_WorldStatic, ECR_Block);

		ConstCast(VisibilityCollision).SetResponseToChannel(ECC_VoxelEditor, ECR_Block);

		ConstCast(VisibilityCollision).SetObjectType(ECC_VoxelEditor);
	}
}

bool FVoxelConfig::Equals(const FVoxelConfig& Other) const
{
	VOXEL_FUNCTION_COUNTER();

	if (bUseCameraInvoker != Other.bUseCameraInvoker ||
		CameraInvokerPositionPrecision != Other.CameraInvokerPositionPrecision)
	{
		return false;
	}

	if (FeatureLevel != Other.FeatureLevel)
	{
		return false;
	}

	if (!LocalToWorld.Equals(Other.LocalToWorld))
	{
		return false;
	}

	if (VoxelSize != Other.VoxelSize ||
		RenderChunkSize != Other.RenderChunkSize ||
		LODQuality != Other.LODQuality ||
		QualityExponent != Other.QualityExponent ||
		MegaMaterialProxy != Other.MegaMaterialProxy ||
		LayerToRender != Other.LayerToRender ||
		bEnableNanite != Other.bEnableNanite ||
		bEnableCubicMesher != Other.bEnableCubicMesher ||
		MaxLOD != Other.MaxLOD ||
		MaxBackgroundTasks != Other.MaxBackgroundTasks)
	{
		return false;
	}

	if (!FVoxelUtilities::BodyInstanceEqual(VisibilityCollision, Other.VisibilityCollision) ||
		CollisionChunkSize != Other.CollisionChunkSize ||
		!FVoxelUtilities::BodyInstanceEqual(InvokerCollision, Other.InvokerCollision) ||
		bDoubleSidedCollision != Other.bDoubleSidedCollision ||
		bGenerateOverlapEvents != Other.bGenerateOverlapEvents ||
		CollisionVoxelSize != Other.CollisionVoxelSize)
	{
		return false;
	}

	if (bEnableNavigation != Other.bEnableNavigation ||
		NavigationChunkSize != Other.NavigationChunkSize ||
		MaxAdditionalNavigationChunks != Other.MaxAdditionalNavigationChunks ||
		bGenerateInsideNavMeshBounds != Other.bGenerateInsideNavMeshBounds ||
		bOnlyGenerateNavigationInEditor != Other.bOnlyGenerateNavigationInEditor ||
		NavigationVoxelSize != Other.NavigationVoxelSize ||
		NavigationMeshComponentClass != Other.NavigationMeshComponentClass)
	{
		return false;
	}

	if (NaniteMaxTessellationLOD != Other.NaniteMaxTessellationLOD ||
		DisplacementFade != Other.DisplacementFade ||
		NanitePositionPrecision != Other.NanitePositionPrecision ||
		bCompressNaniteVertices != Other.bCompressNaniteVertices ||
		bUseNaniteBuilder != Other.bUseNaniteBuilder)
	{
		return false;
	}

	if (bEnableLumen != Other.bEnableLumen ||
		bEnableRaytracing != Other.bEnableRaytracing ||
		bGenerateMeshDistanceFields != Other.bGenerateMeshDistanceFields ||
		RuntimeVirtualTextures != Other.RuntimeVirtualTextures ||
		BlockinessMetadata != Other.BlockinessMetadata ||
		RaytracingMaxLOD != Other.RaytracingMaxLOD ||
		MeshDistanceFieldMaxLOD != Other.MeshDistanceFieldMaxLOD ||
		MeshDistanceFieldBias != Other.MeshDistanceFieldBias ||
		MeshDistanceFieldBricksPerChunk != Other.MeshDistanceFieldBricksPerChunk ||
		ComponentSettings != Other.ComponentSettings ||
		bCacheTextureStreaming != Other.bCacheTextureStreaming)
	{
		return false;
	}

	if (bRenderScatterActors != Other.bRenderScatterActors ||
		bUseCameraScatterInvoker != Other.bUseCameraScatterInvoker ||
		CameraScatterInvokerPositionPrecision != Other.CameraScatterInvokerPositionPrecision)
	{
		return false;
	}

	return true;
}

bool FVoxelConfig::CanEnableNanite() const
{
	return UseNanite(GShaderPlatformForFeatureLevel[FeatureLevel]);
}

bool FVoxelConfig::CanEnableLumen() const
{
	// Don't check DynamicGlobalIllumination == EDynamicGlobalIlluminationMethod::Lumen,
	// as that could be overriden by post process volumes & is not set in packaged
	return DoesPlatformSupportLumenGI(GShaderPlatformForFeatureLevel[FeatureLevel]);
}

bool FVoxelConfig::CanEnableRayTracing() const
{
	return
		IsRayTracingEnabled(GShaderPlatformForFeatureLevel[FeatureLevel]) &&
		(GRHISupportsInlineRayTracing || (GRHISupportsRayTracingShaders && GRHISupportsRayTracingDispatchIndirect));
}

bool FVoxelConfig::UseHardwareRayTracingForLumen() const
{
	// See Lumen::UseHardwareRayTracing

	static const auto CVarLumenUseHardwareRayTracing = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.HardwareRayTracing"));
	if (!ensure(CVarLumenUseHardwareRayTracing))
	{
		return false;
	}

	return
		CanEnableRayTracing() &&
		CVarLumenUseHardwareRayTracing->GetInt() != 0;
}

bool FVoxelConfig::UseNavigationInvokers(UWorld& TargetWorld)
{
	const UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(&TargetWorld);
	if (!NavigationSystem)
	{
		return false;
	}

	return NavigationSystem->IsSetUpForLazyGeometryExporting();
}