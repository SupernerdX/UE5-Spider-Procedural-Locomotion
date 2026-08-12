// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightGraphStampRef.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#include "VoxelHeightGraphStamp_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelHeightGraphStamp_K2 : public UVoxelStampBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Casting", meta = (ExpandEnumAsExecs = "Result"))
	static FVoxelHeightGraphStampRef CastToHeightGraphStamp(
		const FVoxelStampRef Stamp,
		EVoxelStampCastResult& Result)
	{
		return CastToStampImpl<FVoxelHeightGraphStamp>(Stamp, Result);
	}

	// Make a copy of this stamp
	// You can then call Set XXXX on the copy without having the original stamp be modified
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Graph", DisplayName = "Get Height Graph Stamp")
	static void MakeCopy(
		const FVoxelHeightGraphStampRef Stamp,
		FVoxelHeightGraphStampRef& Copy)
	{
		Copy = FVoxelHeightGraphStampRef(Stamp.MakeCopy());
	}

	/**
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Graph", DisplayName = "Make Voxel Height Graph Stamp", meta = (Keywords = "Construct, Create", Layer = "/Script/Voxel.VoxelHeightLayer'/Voxel/Default/DefaultHeightLayer.DefaultHeightLayer'", AutoCreateRefTerm = "AdditionalLayers", HeightPaddingMultiplier = "0.100000", Behavior = "AffectAll", Smoothness = "100.000000", LODRange = "(Min=0,Max=32)", bApplyOnVoid = "True"))
	static void Make(
		FVoxelHeightGraphStampRef& Stamp,
		UVoxelHeightGraph* Graph,
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
		Stamp = FVoxelHeightGraphStampRef::New();
		Stamp->Graph = Graph;
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
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Graph", DisplayName = "Break Voxel Height Graph Stamp", meta = (Keywords = "Break", AutoCreateRefTerm = "AdditionalLayers"))
	static void Break(
		const FVoxelHeightGraphStampRef Stamp,
		UVoxelHeightGraph*& Graph,
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
		Graph = FVoxelUtilities::MakeSafe<UVoxelHeightGraph*>();
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

		Graph = Stamp->Graph;
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

	// Get Graph
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Graph", DisplayName = "Get Graph")
	static void GetGraph(
		UPARAM(Required) FVoxelHeightGraphStampRef Stamp,
		UVoxelHeightGraph*& Graph)
	{
		Graph = FVoxelUtilities::MakeSafe<UVoxelHeightGraph*>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Graph = Stamp->Graph;
	}

	// Set Graph
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Graph", DisplayName = "Set Graph")
	static void SetGraph(
		UPARAM(Required) FVoxelHeightGraphStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightGraphStampRef& OutStamp,
		UVoxelHeightGraph* Graph)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->Graph = Graph;
		Stamp.Update();
	}
};