// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMeshComponentSettings.generated.h"

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMeshComponentSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (Overridable))
	TArray<TObjectPtr<UMaterialInterface>> MaterialOverrides;

	// Scale applied to change the computation of LOD distances when using the StaticMesh screen sizes. 
	// Smaller values make LODs transition earlier.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Culling", meta = (Overridable))
	float InstanceLODDistanceScale = 1.f;

	// Distance from camera at which each instance begins to draw.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Culling", meta = (Overridable))
	int32 InstanceMinDrawDistance = 0;

	// Distance from camera at which each instance begins to fade out.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Culling", meta = (Overridable))
	int32 InstanceStartCullDistance = 0;

	// Distance from camera at which each instance completely fades out.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Culling", meta = (Overridable))
	int32 InstanceEndCullDistance = 0;

	// If true, this component will use GPU LOD selection.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Culling", meta = (Overridable))
	bool bUseGpuLodSelection = true;

	// Controls whether the foliage should cast a shadow or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	bool bCastShadow = true;

	// Whether to use the mesh distance field representation (when present) for shadowing indirect lighting (from lightmaps or skylight) on Movable components.
	// This works like capsule shadows on skeletal meshes, except using the mesh distance field so no physics asset is required.
	// The StaticMesh must have 'Generate Mesh Distance Field' enabled, or the project must have 'Generate Mesh Distance Fields' enabled for this feature to work.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (DisplayName = "Distance Field Indirect Shadow", Overridable))
	bool bCastDistanceFieldIndirectShadow = false;

	// Controls how dark the dynamic indirect shadow can be.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (UIMin = "0", UIMax = "1", DisplayName = "Distance Field Indirect Shadow Min Visibility", EditCondition = "bCastDistanceFieldIndirectShadow", Overridable))
	float DistanceFieldIndirectShadowMinVisibility = 0.1f;

	// Whether to override the DistanceFieldSelfShadowBias setting of the static mesh asset with the DistanceFieldSelfShadowBias of this component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	bool bOverrideDistanceFieldSelfShadowBias = false;

	// Useful for reducing self shadowing from distance field methods when using world position offset to animate the mesh's vertices.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bOverrideDistanceFieldSelfShadowBias", Overridable))
	float DistanceFieldSelfShadowBias = 0.f;

	// Enable dynamic sort mesh's triangles to remove ordering issue when rendered with a translucent material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (UIMin = "0", UIMax = "1", DisplayName = "Sort Triangles (Experimental)", Overridable))
	bool bSortTriangles = false;

	// Controls whether the static mesh component's backface culling should be reversed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	bool bReverseCulling = false;

	// Whether to override the lightmap resolution defined in the static mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	bool bOverrideLightMapRes = false;

	// Light map resolution to use on this component, used if bOverrideLightMapRes is true and there is a valid StaticMesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMax = 4096, EditCondition="bOverrideLightMapRes", Overridable))
	int32 OverriddenLightMapRes = 64;

	// Quality of indirect lighting for Movable primitives.  This has a large effect on Indirect Lighting Cache update time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	TEnumAsByte<EIndirectLightingCacheQuality> IndirectLightingCacheQuality = ILCQ_Point;

	// Controls the type of lightmap used for this component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	ELightmapType LightmapType = ELightmapType::Default;

	// Whether the primitive will be used as an emissive light source.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	bool bEmissiveLightSource = false;

	// Controls whether the primitive should inject light into the Light Propagation Volume.  This flag is only used if CastShadow is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", Overridable))
	bool bAffectDynamicIndirectLighting = true;

	// Controls whether the primitive should affect indirect lighting when hidden. This flag is only used if bAffectDynamicIndirectLighting is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bAffectDynamicIndirectLighting", Overridable))
	bool bAffectIndirectLightingWhileHidden = false;

	// Controls whether the primitive should affect dynamic distance field lighting methods.  This flag is only used if CastShadow is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	bool bAffectDistanceFieldLighting = false;

	// Controls whether the primitive should cast shadows in the case of non precomputed shadowing.  This flag is only used if CastShadow is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", DisplayName = "Dynamic Shadow", Overridable))
	bool bCastDynamicShadow = true;

	// Whether the object should cast a static shadow from shadow casting lights.  This flag is only used if CastShadow is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", DisplayName = "Static Shadow", Overridable))
	bool bCastStaticShadow = true;

	// Control shadow invalidation behavior, in particular with respect to Virtual Shadow Maps and material effects like World Position Offset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", Overridable))
	EShadowCacheInvalidationBehavior ShadowCacheInvalidationBehavior = EShadowCacheInvalidationBehavior::Auto;

	// Whether the object should cast a volumetric translucent shadow.
	// Volumetric translucent shadows are useful for primitives with smoothly changing opacity like particles representing a volume,
	// But have artifacts when used on highly opaque surfaces.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", DisplayName = "Volumetric Translucent Shadow", Overridable))
	bool bCastVolumetricTranslucentShadow = false;

	// Whether the object should cast contact shadows.
	// This flag is only used if CastShadow is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", DisplayName = "Contact Shadow", Overridable))
	bool bCastContactShadow = true;

	// When enabled, the component will only cast a shadow on itself and not other components in the world.
	// This is especially useful for first person weapons, and forces bCastInsetShadow to be enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", Overridable))
	bool bSelfShadowOnly = false;

	// When enabled, the component will be rendering into the far shadow cascades (only for directional lights).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", DisplayName = "Far Shadow", Overridable))
	bool bCastFarShadow = false;

	// Whether this component should create a per-object shadow that gives higher effective shadow resolution.
	// Useful for cinematic character shadowing. Assumed to be enabled if bSelfShadowOnly is enabled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", DisplayName = "Dynamic Inset Shadow", Overridable))
	bool bCastInsetShadow = false;

	// Whether this component should cast shadows from lights that have bCastShadowsFromCinematicObjectsOnly enabled.
	// This is useful for characters in a cinematic with special cinematic lights, where the cost of shadowmap rendering of the environment is undesired.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", Overridable))
	bool bCastCinematicShadow = false;

	// If true, the primitive will cast shadows even if bHidden is true.
	// Controls whether the primitive should cast shadows when hidden.
	// This flag is only used if CastShadow is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", DisplayName = "Hidden Shadow", Overridable))
	bool bCastHiddenShadow = false;

	// Whether this primitive should cast dynamic shadows as if it were a two sided material.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (EditCondition = "bCastShadow", DisplayName = "Shadow Two Sided", Overridable))
	bool bCastShadowAsTwoSided = false;

	// Whether to light this component and any attachments as a group.  This only has effect on the root component of an attachment tree.
	// When enabled, attached component shadowing settings like bCastInsetShadow, bCastVolumetricTranslucentShadow, etc, will be ignored.
	// This is useful for improving performance when multiple movable components are attached together.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	bool bLightAttachmentsAsGroup = false;

	// If set, then it overrides any bLightAttachmentsAsGroup set in a parent.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	bool bExcludeFromLightAttachmentGroup = false;

	// Whether the whole component should be shadowed as one from stationary lights, which makes shadow receiving much cheaper.
	// When enabled shadowing data comes from the volume lighting samples precomputed by Lightmass, which are very sparse.
	// This is currently only used on stationary directional lights.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	bool bSingleSampleShadowFromStationaryLights = false;

	// Lighting channels that placed foliage will be assigned. Lights with matching channels will affect the foliage.
	// These channels only apply to opaque materials, direct lighting, and dynamic lighting and shadowing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (Overridable))
	FLightingChannels LightingChannels;

	// If true, WireframeColorOverride will be used. If false, color is determined based on mobility and physics simulation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bOverrideWireframeColor = false;

	// Wireframe color to use if bOverrideWireframeColor is true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (EditCondition = "bOverrideWireframeColor", Overridable))
	FColor WireframeColorOverride = FColor::White;

	// Forces this component to always use Nanite for masked materials, even if FNaniteSettings::bAllowMaskedMaterials=false
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bForceNaniteForMasked = false;

	// Forces this component to use fallback mesh for rendering if Nanite is enabled on the mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bDisallowNanite = false;

	// Whether to evaluate World Position Offset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bEvaluateWorldPositionOffset = true;

	// Whether world position offset turns on velocity writes.
	// If the WPO isn't static then setting false may give incorrect motion vectors.
	// But if we know that the WPO is static then setting false may save performance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bWorldPositionOffsetWritesVelocity = true;

	// Distance at which to disable World Position Offset for an entire instance (0 = Never disable WPO).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	int32 WorldPositionOffsetDisableDistance = 0;

	// Used to forcefully disable pixel programmable rasterization of Nanite when the mesh is further than a given distance from the camera.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	float NanitePixelProgrammableDistance = 0.f;

	// Whether to evaluate World Position Offset for ray tracing. 
	// This is only used when running with r.RayTracing.Geometry.StaticMeshes.WPO=1 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bEvaluateWorldPositionOffsetInRayTracing = true;

	// Translucent material to blend on top of this mesh. Mesh will be rendered twice - once with a base material and once with overlay material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	TObjectPtr<UMaterialInterface> OverlayMaterial;

	// The max draw distance for overlay material. A distance of 0 indicates that overlay will be culled using primitive max distance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	float OverlayMaterialMaxDrawDistance = 0.f;

	// If true, this component will be visible in reflection captures.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bVisibleInReflectionCaptures = true;

	// If true, this component will be visible in real-time sky light reflection captures.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bVisibleInRealTimeSkyCaptures = true;

	// If true, this component will be visible in ray tracing effects. Turning this off will remove it from ray traced reflections, shadows, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bVisibleInRayTracing = true;

	// If true, this component will be rendered in the main pass (z prepass, basepass, transparency)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bRenderInMainPass = true;

	// If true, this component will be rendered in the depth pass even if it's not rendered in the main pass
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (EditCondition = "!bRenderInMainPass", Overridable))
	bool bRenderInDepthPass = true;

	// Whether the primitive receives decals.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bReceivesDecals = true;

	// If this is True, this component won't be visible when the view actor is the component's owner, directly or indirectly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bOwnerNoSee = false;

	// If this is True, this component will only be visible when the view actor is the component's owner, directly or indirectly.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bOnlyOwnerSee = false;

	// Treat this primitive as part of the background for occlusion purposes. This can be used as an optimization to reduce the cost of rendering skyboxes, large ground planes that are part of the vista, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bTreatAsBackgroundForOcclusion = false;

	// Whether to render the primitive in the depth only pass.  
	// This should generally be true for all objects, and let the renderer make decisions about whether to render objects in the depth only pass.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	bool bUseAsOccluder = true;

	// If true, this component will be rendered in the CustomDepth pass (usually used for outlines)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (DisplayName = "Render CustomDepth Pass", Overridable))
	bool bRenderCustomDepth = false;

	// Optionally write this 0-255 value to the stencil buffer in CustomDepth pass (Requires project setting or r.CustomDepth == 3)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (UIMin = "0", UIMax = "255", EditCondition = "bRenderCustomDepth", DisplayName = "CustomDepth Stencil Value", Overridable))
	int32 CustomDepthStencilValue = 0;

	// When true, will only be visible in Scene Capture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (DisplayName = "Visible In Scene Capture Only", Overridable))
	bool bVisibleInSceneCaptureOnly = false;

	// When true, will not be captured by Scene Capture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (DisplayName = "Hidden In Scene Capture", Overridable))
	bool bHiddenInSceneCapture = false;

	// Translucent objects with a lower sort priority draw behind objects with a higher priority.
	// Translucent objects with the same priority are rendered from back-to-front based on their bounds origin.
	// This setting is also used to sort objects being drawn into a runtime virtual texture.
	// Ignored if the object is not translucent.  The default priority is zero.
	// Warning: This should never be set to a non-default value unless you know what you are doing, as it will prevent the renderer from sorting correctly.
	// It is especially problematic on dynamic gameplay effects.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	int32 TranslucencySortPriority = 0;

	// Modified sort distance offset for translucent objects in world units.
	// A positive number will move the sort distance further and a negative number will move the distance closer.
	// Ignored if the object is not translucent.
	// Warning: Adjusting this value will prevent the renderer from correctly sorting based on distance.  Only modify this value if you are certain it will not cause visual artifacts.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (Overridable))
	float TranslucencySortDistanceOffset = 0.f;

	// Scales the bounds of the object.
	// This is useful when using World Position Offset to animate the vertices of the object outside of its bounds.
	// Warning: Increasing the bounds of an object will reduce performance and shadow quality!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (UIMin = "1", UIMax = "10.0", Overridable))
	float BoundsScale = 1.f;

	// Mask used for stencil buffer writes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (EditCondition = "bRenderCustomDepth", Overridable))
	ERendererStencilMask CustomDepthStencilWriteMask = ERendererStencilMask::ERSM_Default;

	// If false, vertex color mesh painting is disabled on this component. 
	// This may be set to false by blueprint functions that override vertex colors in construction script.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Painting", meta = (Overridable))
	bool bEnableVertexColorMeshPainting = true;

	// If false, texture color mesh painting is disabled on this component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Painting", meta = (Overridable))
	bool bEnableTextureColorMeshPainting = true;

	// Whether to override the MeshPaintTextureCoordinateIndex set on the static mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Painting", meta = (Overridable))
	bool bOverrideMeshPaintTextureCoordinateIndex = false;

	// Whether to override the MeshPaintTextureCoordinateIndex set on the static mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Painting", meta = (Overridable))
	bool bOverrideMeshPaintTextureResolution = false;

	// The overriden coordinate index to use when texture color painting on this component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Painting", meta = (EditCondition = "bOverrideMeshPaintTextureCoordinateIndex", UIMin = "0", UIMax = "3", Overridable))
	int32 OverriddenMeshPaintTextureCoordinateIndex = 0;

	// The overriden resolution of texture color mesh paint textures on this component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Painting", meta = (EditCondition = "bOverrideMeshPaintTextureResolution", UIMin = "0", UIMax = "4096", ClampMax = "4096", Overridable))
	int32 OverriddenMeshPaintTextureResolution = 0;

	// Ignore this instance of this static mesh when calculating streaming information.
	// This can be useful when doing things like applying character textures to static geometry,
	// to avoid them using distance-based streaming.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Streaming", meta = (Overridable))
	bool bIgnoreInstanceForTextureStreaming = false;

	// Allows adjusting the desired streaming distance of streaming textures that uses UV 0.
	// 1.0 is the default, whereas a higher value makes the textures stream in sooner from far away.
	// A lower value (0.0-1.0) makes the textures stream in later (you have to be closer).
	// Value can be < 0 (from legcay content, or code changes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Streaming", meta = (Overridable))
	float StreamingDistanceMultiplier = 1.f;

	// Bias to the LOD selected for rendering to runtime virtual textures.
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Virtual Texture", meta = (DisplayName = "Virtual Texture LOD Bias", UIMin = "-7", UIMax = "8", Overridable))
	int8 VirtualTextureLodBias = 0;

	// Number of lower mips in the runtime virtual texture to skip for rendering this primitive.
	// Larger values reduce the effective draw distance in the runtime virtual texture.
	// This culling method doesn't take into account primitive size or virtual texture size.
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Virtual Texture", meta = (DisplayName = "Virtual Texture Skip Mips", UIMin = "0", UIMax = "7", Overridable))
	int8 VirtualTextureCullMips = 0;

	// Set the minimum pixel coverage before culling from the runtime virtual texture.
	// Larger values reduce the effective draw distance in the runtime virtual texture.
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Virtual Texture", meta = (UIMin = "0", UIMax = "7", Overridable))
	int8 VirtualTextureMinCoverage = 0;

	// Controls if this component draws in the main pass as well as in the virtual texture.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Virtual Texture", meta = (DisplayName = "Draw in Main Pass", Overridable))
	ERuntimeVirtualTextureMainPassType VirtualTextureRenderPassType = ERuntimeVirtualTextureMainPassType::Exclusive;

public:
	void ApplyToComponent(UInstancedStaticMeshComponent& Component) const;
};