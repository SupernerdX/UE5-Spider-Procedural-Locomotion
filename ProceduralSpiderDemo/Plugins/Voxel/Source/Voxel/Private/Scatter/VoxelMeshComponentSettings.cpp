// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelMeshComponentSettings.h"
#include "Components/InstancedStaticMeshComponent.h"

void FVoxelMeshComponentSettings::ApplyToComponent(UInstancedStaticMeshComponent& Component) const
{
	VOXEL_FUNCTION_COUNTER();

#define COPY(VariableName) \
	Component.VariableName = VariableName;

	// Culling
	COPY(InstanceLODDistanceScale);
	UE_506_ONLY(COPY(InstanceMinDrawDistance));
	COPY(InstanceStartCullDistance);
	COPY(InstanceEndCullDistance);
	COPY(bUseGpuLodSelection);

	// Lighting
	Component.CastShadow = bCastShadow;
	COPY(bCastDistanceFieldIndirectShadow);
	COPY(DistanceFieldIndirectShadowMinVisibility);
	COPY(bOverrideDistanceFieldSelfShadowBias);
	COPY(DistanceFieldSelfShadowBias);
	COPY(bSortTriangles);
	COPY(bReverseCulling);
	COPY(bOverrideLightMapRes);
	COPY(OverriddenLightMapRes);
	COPY(IndirectLightingCacheQuality);
	Component.SetLightmapType(LightmapType);
	COPY(bAffectDynamicIndirectLighting);
	COPY(bAffectIndirectLightingWhileHidden);
	COPY(bAffectDistanceFieldLighting);
	COPY(bEmissiveLightSource);
	COPY(bCastDynamicShadow);
	COPY(bCastStaticShadow);
	COPY(ShadowCacheInvalidationBehavior);
	COPY(bCastVolumetricTranslucentShadow);
	COPY(bCastContactShadow);
	COPY(bSelfShadowOnly);
	COPY(bCastFarShadow);
	COPY(bCastInsetShadow);
	COPY(bCastCinematicShadow);
	COPY(bCastHiddenShadow);
	COPY(bCastShadowAsTwoSided);
	COPY(bLightAttachmentsAsGroup);
	COPY(bExcludeFromLightAttachmentGroup);
	COPY(bSingleSampleShadowFromStationaryLights);
	COPY(LightingChannels);

	// Rendering
	COPY(bOverrideWireframeColor);
	COPY(WireframeColorOverride);
	COPY(bForceNaniteForMasked);
	COPY(bDisallowNanite);
	COPY(bEvaluateWorldPositionOffset);
	COPY(bWorldPositionOffsetWritesVelocity);
	COPY(WorldPositionOffsetDisableDistance);
	COPY(NanitePixelProgrammableDistance);
	COPY(bEvaluateWorldPositionOffsetInRayTracing);
	COPY(OverlayMaterial);
	COPY(OverlayMaterialMaxDrawDistance);
	COPY(bVisibleInReflectionCaptures);
	COPY(bVisibleInRealTimeSkyCaptures);
	COPY(bVisibleInRayTracing);
	COPY(bRenderInMainPass);
	COPY(bRenderInDepthPass);
	COPY(bReceivesDecals);
	COPY(bOwnerNoSee);
	COPY(bOnlyOwnerSee);
	COPY(bTreatAsBackgroundForOcclusion);
	COPY(bUseAsOccluder);
	COPY(bRenderCustomDepth);
	COPY(CustomDepthStencilValue);
	COPY(bVisibleInSceneCaptureOnly);
	COPY(bHiddenInSceneCapture);
	COPY(TranslucencySortPriority);
	COPY(TranslucencySortDistanceOffset);
	COPY(BoundsScale);
	COPY(CustomDepthStencilWriteMask);

	// Mesh Painting
	COPY(bEnableVertexColorMeshPainting);
	COPY(bEnableTextureColorMeshPainting);
	COPY(bOverrideMeshPaintTextureCoordinateIndex);
	COPY(bOverrideMeshPaintTextureResolution);
	COPY(OverriddenMeshPaintTextureCoordinateIndex);
	COPY(OverriddenMeshPaintTextureResolution);

	// Texture streaming
	COPY(bIgnoreInstanceForTextureStreaming);
	COPY(StreamingDistanceMultiplier);

	// Virtual Texture
	COPY(VirtualTextureLodBias);
	COPY(VirtualTextureCullMips);
	COPY(VirtualTextureMinCoverage);
	COPY(VirtualTextureRenderPassType);

#undef COPY

	Component.MarkRenderStateDirty();
}