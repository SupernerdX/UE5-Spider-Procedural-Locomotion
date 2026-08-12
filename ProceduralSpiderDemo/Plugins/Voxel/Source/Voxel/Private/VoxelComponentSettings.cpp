// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelComponentSettings.h"

void FVoxelComponentSettings::ApplyToComponent(UPrimitiveComponent& Component) const
{
	VOXEL_FUNCTION_COUNTER();

#define COPY(VariableName) \
	Component.VariableName = VariableName;

	Component.CastShadow = bCastShadow;
	COPY(IndirectLightingCacheQuality);
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
	COPY(bVisibleInReflectionCaptures);
	COPY(bVisibleInRealTimeSkyCaptures);
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
	COPY(CustomDepthStencilWriteMask);
	COPY(VirtualTextureLodBias);
	COPY(VirtualTextureCullMips);
	COPY(VirtualTextureMinCoverage);
	COPY(VirtualTextureRenderPassType);

#undef COPY

	Component.MarkRenderStateDirty();
}

bool FVoxelComponentSettings::operator==(const FVoxelComponentSettings& Other) const
{
	return
		bCastShadow == Other.bCastShadow &&
		IndirectLightingCacheQuality == Other.IndirectLightingCacheQuality &&
		bEmissiveLightSource == Other.bEmissiveLightSource &&
		bCastDynamicShadow == Other.bCastDynamicShadow &&
		bCastStaticShadow == Other.bCastStaticShadow &&
		ShadowCacheInvalidationBehavior == Other.ShadowCacheInvalidationBehavior &&
		bCastVolumetricTranslucentShadow == Other.bCastVolumetricTranslucentShadow &&
		bCastContactShadow == Other.bCastContactShadow &&
		bSelfShadowOnly == Other.bSelfShadowOnly &&
		bCastFarShadow == Other.bCastFarShadow &&
		bCastInsetShadow == Other.bCastInsetShadow &&
		bCastCinematicShadow == Other.bCastCinematicShadow &&
		bCastHiddenShadow == Other.bCastHiddenShadow &&
		bCastShadowAsTwoSided == Other.bCastShadowAsTwoSided &&
		bLightAttachmentsAsGroup == Other.bLightAttachmentsAsGroup &&
		bExcludeFromLightAttachmentGroup == Other.bExcludeFromLightAttachmentGroup &&
		bSingleSampleShadowFromStationaryLights == Other.bSingleSampleShadowFromStationaryLights &&
		LightingChannels.bChannel0 == Other.LightingChannels.bChannel0 &&
		LightingChannels.bChannel1 == Other.LightingChannels.bChannel1 &&
		LightingChannels.bChannel2 == Other.LightingChannels.bChannel2 &&
		bVisibleInReflectionCaptures == Other.bVisibleInReflectionCaptures &&
		bVisibleInRealTimeSkyCaptures == Other.bVisibleInRealTimeSkyCaptures &&
		bRenderInMainPass == Other.bRenderInMainPass &&
		bRenderInDepthPass == Other.bRenderInDepthPass &&
		bReceivesDecals == Other.bReceivesDecals &&
		bOwnerNoSee == Other.bOwnerNoSee &&
		bOnlyOwnerSee == Other.bOnlyOwnerSee &&
		bTreatAsBackgroundForOcclusion == Other.bTreatAsBackgroundForOcclusion &&
		bUseAsOccluder == Other.bUseAsOccluder &&
		bRenderCustomDepth == Other.bRenderCustomDepth &&
		CustomDepthStencilValue == Other.CustomDepthStencilValue &&
		bVisibleInSceneCaptureOnly == Other.bVisibleInSceneCaptureOnly &&
		bHiddenInSceneCapture == Other.bHiddenInSceneCapture &&
		TranslucencySortPriority == Other.TranslucencySortPriority &&
		TranslucencySortDistanceOffset == Other.TranslucencySortDistanceOffset &&
		CustomDepthStencilWriteMask == Other.CustomDepthStencilWriteMask &&
		VirtualTextureLodBias == Other.VirtualTextureLodBias &&
		VirtualTextureCullMips == Other.VirtualTextureCullMips &&
		VirtualTextureMinCoverage == Other.VirtualTextureMinCoverage &&
		VirtualTextureRenderPassType == Other.VirtualTextureRenderPassType;
}