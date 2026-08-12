// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraph.h"

VOXEL_INITIALIZE_STYLE(VoxelEditorAssetIconsStyle)
{
	Set("ClassIcon.VoxelLayerStack", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/CompositingElement_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelLayerStack", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/CompositingElement_64", CoreStyleConstants::Icon64x64));

	Set("ClassIcon.VoxelHeightLayer", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/DecalActor_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelHeightLayer", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/DecalActor_64", CoreStyleConstants::Icon64x64));
	Set("ClassIcon.VoxelVolumeLayer", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/DecalActor_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelVolumeLayer", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/DecalActor_64", CoreStyleConstants::Icon64x64));
	Set("ClassIcon.VoxelScatterLayer", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/DecalActor_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelScatterLayer", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/DecalActor_64", CoreStyleConstants::Icon64x64));

	Set("ClassIcon.VoxelFunctionLibraryAsset", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/BlueprintFunctionLibrary_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelFunctionLibraryAsset", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/BlueprintFunctionLibrary_64", CoreStyleConstants::Icon64x64));

	Set("ClassIcon.VoxelGraph", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Blueprint_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelGraph", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Blueprint_64", CoreStyleConstants::Icon64x64));
	for (const UClass* Class : GetDerivedClasses<UVoxelGraph>())
	{
		Set("ClassIcon." + Class->GetFName(), new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Blueprint_16", CoreStyleConstants::Icon16x16));
		Set("ClassThumbnail." + Class->GetFName(), new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Blueprint_64", CoreStyleConstants::Icon64x64));
	}

	Set("ClassIcon.VoxelMegaMaterial", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Material_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelMegaMaterial", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Material_64", CoreStyleConstants::Icon64x64));

	Set("ClassIcon.VoxelSurfaceAsset", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Material_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelSurfaceAsset", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Material_64", CoreStyleConstants::Icon64x64));

	Set("ClassIcon.VoxelSmartSurface", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Material_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelSmartSurface", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/Material_64", CoreStyleConstants::Icon64x64));

	Set("ClassIcon.VoxelStaticMesh", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/StaticMesh_16", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelStaticMesh", new CORE_IMAGE_BRUSH_SVG("Starship/AssetIcons/StaticMesh_64", CoreStyleConstants::Icon64x64));

	Set("ClassIcon.VoxelGraphHeightmap", new CORE_IMAGE_BRUSH("Icons/icon_Landscape_Target_Heightmap_48x", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelGraphHeightmap", new CORE_IMAGE_BRUSH("Icons/icon_Landscape_Target_Heightmap_48x", CoreStyleConstants::Icon64x64));
	Set("ClassIcon.VoxelHeightmap", new CORE_IMAGE_BRUSH("Icons/icon_Landscape_Target_Heightmap_48x", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.VoxelHeightmap", new CORE_IMAGE_BRUSH("Icons/icon_Landscape_Target_Heightmap_48x", CoreStyleConstants::Icon64x64));

	Set("ClassIcon.ActorFactory_VoxelHeightSculptActor", new CORE_IMAGE_BRUSH("Icons/AssetIcons/Plane_16x", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.ActorFactory_VoxelHeightSculptActor", new CORE_IMAGE_BRUSH("Icons/AssetIcons/Plane_64x", CoreStyleConstants::Icon64x64));

	Set("ClassIcon.ActorFactory_VoxelVolumeSculptActor", new CORE_IMAGE_BRUSH("Icons/AssetIcons/Cube_16x", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.ActorFactory_VoxelVolumeSculptActor", new CORE_IMAGE_BRUSH("Icons/AssetIcons/Cube_64x", CoreStyleConstants::Icon64x64));
}