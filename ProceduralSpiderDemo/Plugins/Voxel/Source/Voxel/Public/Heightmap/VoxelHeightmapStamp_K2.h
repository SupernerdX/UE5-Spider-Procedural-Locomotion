// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightmapStampRef.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#include "VoxelHeightmapStamp_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelHeightmapStamp_K2 : public UVoxelStampBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Casting", meta = (ExpandEnumAsExecs = "Result"))
	static FVoxelHeightmapStampRef CastToHeightmapStamp(
		const FVoxelStampRef Stamp,
		EVoxelStampCastResult& Result)
	{
		return CastToStampImpl<FVoxelHeightmapStamp>(Stamp, Result);
	}

	// Make a copy of this stamp
	// You can then call Set XXXX on the copy without having the original stamp be modified
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Heightmap", DisplayName = "Get Heightmap Stamp")
	static void MakeCopy(
		const FVoxelHeightmapStampRef Stamp,
		FVoxelHeightmapStampRef& Copy)
	{
		Copy = FVoxelHeightmapStampRef(Stamp.MakeCopy());
	}

	/**
	 * @param DefaultSurfaceType Default surface used as base when no weightmap is applied
	 * @param WeightmapSurfaceTypes Use this to override weightmap surfaces
	 * @param Layer Layer that this stamps belong to
	 * You can control the order of layers in Layer Stacks
	 * You can select the layer stack to use in your Voxel World or PCG Sampler settings
	 * @param HeightPaddingMultiplier By how much to extend the bounds, relative to the bounds size
	 * Increase this if you are using a high smoothness
	 * Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	 * @param Priority Priority of the stamp within its layer
	 * Higher priority stamps will be applied last
	 * @param LODRange This stamp will only be applied on LODs within this range (inclusive)
	 * @param bDisableStampSelection If true you won\'t be able to select this stamp by clicking on it
	 * @param bApplyOnVoid If false, this stamp will only apply on parts where another stamp has been applied first
	 * This is useful to avoid having stamps going beyond world bounds
	 * Only used if BlendMode is not Override nor Intersect
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Heightmap", DisplayName = "Make Voxel Heightmap Stamp", meta = (Keywords = "Construct, Create", AutoCreateRefTerm = "WeightmapSurfaceTypes, AdditionalLayers", Layer = "/Script/Voxel.VoxelHeightLayer'/Voxel/Default/DefaultHeightLayer.DefaultHeightLayer'", HeightPaddingMultiplier = "0.100000", Behavior = "AffectAll", Smoothness = "100.000000", LODRange = "(Min=0,Max=32)", bApplyOnVoid = "True"))
	static void Make(
		FVoxelHeightmapStampRef& Stamp,
		UVoxelHeightmap* Heightmap,
		UVoxelSurfaceTypeInterface* DefaultSurfaceType,
		TArray<FVoxelHeightmapStampWeightmapSurfaceType> WeightmapSurfaceTypes,
		UVoxelHeightLayer* Layer,
		EVoxelHeightBlendMode BlendMode,
		TArray<UVoxelHeightLayer*> AdditionalLayers,
		float HeightPaddingMultiplier,
		FTransform Transform,
		EVoxelStampBehavior Behavior,
		int32 Priority,
		float Smoothness,
		FVoxelMetadataOverrides MetadataOverrides,
		FVoxelExposedSeed StampSeed,
		UPARAM(DisplayName = "LOD Range") FInt32Interval LODRange,
		bool bDisableStampSelection,
		bool bApplyOnVoid)
	{
		Stamp = FVoxelHeightmapStampRef::New();
		Stamp->Heightmap = Heightmap;
		Stamp->DefaultSurfaceType = DefaultSurfaceType;
		Stamp->WeightmapSurfaceTypes = WeightmapSurfaceTypes;
		Stamp->Layer = Layer;
		Stamp->BlendMode = BlendMode;
		Stamp->AdditionalLayers = AdditionalLayers;
		Stamp->HeightPaddingMultiplier = HeightPaddingMultiplier;
		Stamp->Transform = Transform;
		Stamp->Behavior = Behavior;
		Stamp->Priority = Priority;
		Stamp->Smoothness = Smoothness;
		Stamp->MetadataOverrides = MetadataOverrides;
		Stamp->StampSeed = StampSeed;
		Stamp->LODRange = LODRange;
		Stamp->bDisableStampSelection = bDisableStampSelection;
		Stamp->bApplyOnVoid = bApplyOnVoid;
	}

	/**
	 * @param DefaultSurfaceType Default surface used as base when no weightmap is applied
	 * @param WeightmapSurfaceTypes Use this to override weightmap surfaces
	 * @param Layer Layer that this stamps belong to
	 * You can control the order of layers in Layer Stacks
	 * You can select the layer stack to use in your Voxel World or PCG Sampler settings
	 * @param HeightPaddingMultiplier By how much to extend the bounds, relative to the bounds size
	 * Increase this if you are using a high smoothness
	 * Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	 * @param Priority Priority of the stamp within its layer
	 * Higher priority stamps will be applied last
	 * @param LODRange This stamp will only be applied on LODs within this range (inclusive)
	 * @param bDisableStampSelection If true you won\'t be able to select this stamp by clicking on it
	 * @param bApplyOnVoid If false, this stamp will only apply on parts where another stamp has been applied first
	 * This is useful to avoid having stamps going beyond world bounds
	 * Only used if BlendMode is not Override nor Intersect
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Heightmap", DisplayName = "Break Voxel Heightmap Stamp", meta = (Keywords = "Break", AutoCreateRefTerm = "WeightmapSurfaceTypes, AdditionalLayers"))
	static void Break(
		const FVoxelHeightmapStampRef Stamp,
		UVoxelHeightmap*& Heightmap,
		UVoxelSurfaceTypeInterface*& DefaultSurfaceType,
		TArray<FVoxelHeightmapStampWeightmapSurfaceType>& WeightmapSurfaceTypes,
		UVoxelHeightLayer*& Layer,
		EVoxelHeightBlendMode& BlendMode,
		TArray<UVoxelHeightLayer*>& AdditionalLayers,
		float& HeightPaddingMultiplier,
		FTransform& Transform,
		EVoxelStampBehavior& Behavior,
		int32& Priority,
		float& Smoothness,
		FVoxelMetadataOverrides& MetadataOverrides,
		FVoxelExposedSeed& StampSeed,
		UPARAM(DisplayName = "LOD Range") FInt32Interval& LODRange,
		bool& bDisableStampSelection,
		bool& bApplyOnVoid)
	{
		Heightmap = FVoxelUtilities::MakeSafe<UVoxelHeightmap*>();
		DefaultSurfaceType = FVoxelUtilities::MakeSafe<UVoxelSurfaceTypeInterface*>();
		WeightmapSurfaceTypes = FVoxelUtilities::MakeSafe<TArray<FVoxelHeightmapStampWeightmapSurfaceType>>();
		Layer = FVoxelUtilities::MakeSafe<UVoxelHeightLayer*>();
		BlendMode = FVoxelUtilities::MakeSafe<EVoxelHeightBlendMode>();
		AdditionalLayers = FVoxelUtilities::MakeSafe<TArray<UVoxelHeightLayer*>>();
		HeightPaddingMultiplier = FVoxelUtilities::MakeSafe<float>();
		Transform = FVoxelUtilities::MakeSafe<FTransform>();
		Behavior = FVoxelUtilities::MakeSafe<EVoxelStampBehavior>();
		Priority = FVoxelUtilities::MakeSafe<int32>();
		Smoothness = FVoxelUtilities::MakeSafe<float>();
		MetadataOverrides = FVoxelUtilities::MakeSafe<FVoxelMetadataOverrides>();
		StampSeed = FVoxelUtilities::MakeSafe<FVoxelExposedSeed>();
		LODRange = FVoxelUtilities::MakeSafe<FInt32Interval>();
		bDisableStampSelection = FVoxelUtilities::MakeSafe<bool>();
		bApplyOnVoid = FVoxelUtilities::MakeSafe<bool>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Heightmap = Stamp->Heightmap;
		DefaultSurfaceType = Stamp->DefaultSurfaceType;
		WeightmapSurfaceTypes = Stamp->WeightmapSurfaceTypes;
		Layer = Stamp->Layer;
		BlendMode = Stamp->BlendMode;
		AdditionalLayers = Stamp->AdditionalLayers;
		HeightPaddingMultiplier = Stamp->HeightPaddingMultiplier;
		Transform = Stamp->Transform;
		Behavior = Stamp->Behavior;
		Priority = Stamp->Priority;
		Smoothness = Stamp->Smoothness;
		MetadataOverrides = Stamp->MetadataOverrides;
		StampSeed = Stamp->StampSeed;
		LODRange = Stamp->LODRange;
		bDisableStampSelection = Stamp->bDisableStampSelection;
		bApplyOnVoid = Stamp->bApplyOnVoid;
	}

	// Get Heightmap
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Heightmap", DisplayName = "Get Heightmap")
	static void GetHeightmap(
		UPARAM(Required) FVoxelHeightmapStampRef Stamp,
		UVoxelHeightmap*& Heightmap)
	{
		Heightmap = FVoxelUtilities::MakeSafe<UVoxelHeightmap*>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Heightmap = Stamp->Heightmap;
	}

	// Set Heightmap
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Heightmap", DisplayName = "Set Heightmap")
	static void SetHeightmap(
		UPARAM(Required) FVoxelHeightmapStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightmapStampRef& OutStamp,
		UVoxelHeightmap* Heightmap)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->Heightmap = Heightmap;
		Stamp.Update();
	}

	/**
	 * Get Default Surface Type
	 * Default surface used as base when no weightmap is applied
	 * @param DefaultSurfaceType Default surface used as base when no weightmap is applied
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Heightmap", DisplayName = "Get Default Surface Type")
	static void GetDefaultSurfaceType(
		UPARAM(Required) FVoxelHeightmapStampRef Stamp,
		UVoxelSurfaceTypeInterface*& DefaultSurfaceType)
	{
		DefaultSurfaceType = FVoxelUtilities::MakeSafe<UVoxelSurfaceTypeInterface*>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		DefaultSurfaceType = Stamp->DefaultSurfaceType;
	}

	/**
	 * Set Default Surface Type
	 * Default surface used as base when no weightmap is applied
	 * This will automatically refresh the stamp
	 * @param DefaultSurfaceType Default surface used as base when no weightmap is applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Heightmap", DisplayName = "Set Default Surface Type")
	static void SetDefaultSurfaceType(
		UPARAM(Required) FVoxelHeightmapStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightmapStampRef& OutStamp,
		UVoxelSurfaceTypeInterface* DefaultSurfaceType)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->DefaultSurfaceType = DefaultSurfaceType;
		Stamp.Update();
	}

	/**
	 * Get Weightmap Surface Types
	 * Use this to override weightmap surfaces
	 * @param WeightmapSurfaceTypes Use this to override weightmap surfaces
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Heightmap", DisplayName = "Get Weightmap Surface Types", meta = (AutoCreateRefTerm = "WeightmapSurfaceTypes"))
	static void GetWeightmapSurfaceTypes(
		UPARAM(Required) FVoxelHeightmapStampRef Stamp,
		TArray<FVoxelHeightmapStampWeightmapSurfaceType>& WeightmapSurfaceTypes)
	{
		WeightmapSurfaceTypes = FVoxelUtilities::MakeSafe<TArray<FVoxelHeightmapStampWeightmapSurfaceType>>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		WeightmapSurfaceTypes = Stamp->WeightmapSurfaceTypes;
	}

	/**
	 * Set Weightmap Surface Types
	 * Use this to override weightmap surfaces
	 * This will automatically refresh the stamp
	 * @param WeightmapSurfaceTypes Use this to override weightmap surfaces
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Heightmap", DisplayName = "Set Weightmap Surface Types", meta = (AutoCreateRefTerm = "WeightmapSurfaceTypes"))
	static void SetWeightmapSurfaceTypes(
		UPARAM(Required) FVoxelHeightmapStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightmapStampRef& OutStamp,
		TArray<FVoxelHeightmapStampWeightmapSurfaceType> WeightmapSurfaceTypes)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->WeightmapSurfaceTypes = WeightmapSurfaceTypes;
		Stamp.Update();
	}
};