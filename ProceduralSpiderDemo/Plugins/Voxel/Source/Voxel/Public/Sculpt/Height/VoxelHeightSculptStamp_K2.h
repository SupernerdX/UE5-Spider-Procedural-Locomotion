// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightSculptStampRef.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#include "VoxelHeightSculptStamp_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelHeightSculptStamp_K2 : public UVoxelStampBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Casting", meta = (ExpandEnumAsExecs = "Result"))
	static FVoxelHeightSculptStampRef CastToHeightSculptStamp(
		const FVoxelStampRef Stamp,
		EVoxelStampCastResult& Result)
	{
		return CastToStampImpl<FVoxelHeightSculptStamp>(Stamp, Result);
	}

	// Make a copy of this stamp
	// You can then call Set XXXX on the copy without having the original stamp be modified
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Get Height Sculpt Stamp")
	static void MakeCopy(
		const FVoxelHeightSculptStampRef Stamp,
		FVoxelHeightSculptStampRef& Copy)
	{
		Copy = FVoxelHeightSculptStampRef(Stamp.MakeCopy());
	}

	/**
	 * @param bStoreRelativeHeights If true, stores the relative height field, based on existing height field while sculpting
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * @param TargetPrecision Maximum allowed height error in world units after packing.
	 * A value of 1 means packed heights will be within 1cm of their original value.
	 * Lower values preserve more detail but use more bits per sample.
	 * For reference, a 10m height range at precision 1cm requires 10 bits.
	 * This is calculated per chunk.
	 * @param bIsInfinite Set this to true for runtime stamps
	 * Will ensure only updated chunks are invalidated when sculpting outside the existing stamp bounds
	 * Do not set this to true on too many stamps, this adds a small overhead to all computed chunks
	 * @param NearMaxLOD Any chunk whose LOD is <= to this will use the Near quality
	 * @param MidMaxLOD Any chunk whose LOD is <= to this will use the Mid quality
	 * @param StackOverride Use this if this stamp is not rendered in the Voxel World stack
	 * This stack will be used during sculpting to query the distances before any sculpt is applied
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Make Voxel Height Sculpt Stamp", meta = (Keywords = "Construct, Create", ScaleXY = "100.000000", bStoreRelativeHeights = "True", TargetPrecision = "1.000000", NearMaxLOD = "3", MidMaxLOD = "6", Layer = "/Script/Voxel.VoxelHeightLayer'/Voxel/Default/DefaultHeightLayer.DefaultHeightLayer'", BlendMode = "Override", AutoCreateRefTerm = "AdditionalLayers", HeightPaddingMultiplier = "0.100000", Behavior = "AffectAll", Smoothness = "100.000000", LODRange = "(Min=0,Max=32)", bApplyOnVoid = "True"))
	static void Make(
		FVoxelHeightSculptStampRef& Stamp,
		float ScaleXY,
		bool bStoreRelativeHeights,
		float TargetPrecision,
		bool bIsInfinite,
		int32 NearMaxLOD,
		int32 MidMaxLOD,
		UVoxelLayerStack* StackOverride,
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
		Stamp = FVoxelHeightSculptStampRef::New();
		Stamp->ScaleXY = ScaleXY;
		Stamp->bStoreRelativeHeights = bStoreRelativeHeights;
		Stamp->TargetPrecision = TargetPrecision;
		Stamp->bIsInfinite = bIsInfinite;
		Stamp->NearMaxLOD = NearMaxLOD;
		Stamp->MidMaxLOD = MidMaxLOD;
		Stamp->StackOverride = StackOverride;
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
	 * @param bStoreRelativeHeights If true, stores the relative height field, based on existing height field while sculpting
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * @param TargetPrecision Maximum allowed height error in world units after packing.
	 * A value of 1 means packed heights will be within 1cm of their original value.
	 * Lower values preserve more detail but use more bits per sample.
	 * For reference, a 10m height range at precision 1cm requires 10 bits.
	 * This is calculated per chunk.
	 * @param bIsInfinite Set this to true for runtime stamps
	 * Will ensure only updated chunks are invalidated when sculpting outside the existing stamp bounds
	 * Do not set this to true on too many stamps, this adds a small overhead to all computed chunks
	 * @param NearMaxLOD Any chunk whose LOD is <= to this will use the Near quality
	 * @param MidMaxLOD Any chunk whose LOD is <= to this will use the Mid quality
	 * @param StackOverride Use this if this stamp is not rendered in the Voxel World stack
	 * This stack will be used during sculpting to query the distances before any sculpt is applied
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
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Break Voxel Height Sculpt Stamp", meta = (Keywords = "Break", AutoCreateRefTerm = "AdditionalLayers"))
	static void Break(
		const FVoxelHeightSculptStampRef Stamp,
		float& ScaleXY,
		bool& bStoreRelativeHeights,
		float& TargetPrecision,
		bool& bIsInfinite,
		int32& NearMaxLOD,
		int32& MidMaxLOD,
		UVoxelLayerStack*& StackOverride,
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
		ScaleXY = FVoxelUtilities::MakeSafe<float>();
		bStoreRelativeHeights = FVoxelUtilities::MakeSafe<bool>();
		TargetPrecision = FVoxelUtilities::MakeSafe<float>();
		bIsInfinite = FVoxelUtilities::MakeSafe<bool>();
		NearMaxLOD = FVoxelUtilities::MakeSafe<int32>();
		MidMaxLOD = FVoxelUtilities::MakeSafe<int32>();
		StackOverride = FVoxelUtilities::MakeSafe<UVoxelLayerStack*>();
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

		ScaleXY = Stamp->ScaleXY;
		bStoreRelativeHeights = Stamp->bStoreRelativeHeights;
		TargetPrecision = Stamp->TargetPrecision;
		bIsInfinite = Stamp->bIsInfinite;
		NearMaxLOD = Stamp->NearMaxLOD;
		MidMaxLOD = Stamp->MidMaxLOD;
		StackOverride = Stamp->StackOverride;
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

	// Get Scale XY
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Get Scale XY")
	static void GetScaleXY(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		float& ScaleXY)
	{
		ScaleXY = FVoxelUtilities::MakeSafe<float>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		ScaleXY = Stamp->ScaleXY;
	}

	// Set Scale XY
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Set Scale XY", meta = (ScaleXY = "100.000000"))
	static void SetScaleXY(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightSculptStampRef& OutStamp,
		float ScaleXY)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->ScaleXY = ScaleXY;
		Stamp.Update();
	}

	/**
	 * Get Store Relative Heights
	 * If true, stores the relative height field, based on existing height field while sculpting
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * @param bStoreRelativeHeights If true, stores the relative height field, based on existing height field while sculpting
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Get Store Relative Heights")
	static void GetStoreRelativeHeights(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		bool& bStoreRelativeHeights)
	{
		bStoreRelativeHeights = FVoxelUtilities::MakeSafe<bool>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		bStoreRelativeHeights = Stamp->bStoreRelativeHeights;
	}

	/**
	 * Set Store Relative Heights
	 * If true, stores the relative height field, based on existing height field while sculpting
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * This will automatically refresh the stamp
	 * @param bStoreRelativeHeights If true, stores the relative height field, based on existing height field while sculpting
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Set Store Relative Heights", meta = (bStoreRelativeHeights = "True"))
	static void SetStoreRelativeHeights(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightSculptStampRef& OutStamp,
		bool bStoreRelativeHeights)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->bStoreRelativeHeights = bStoreRelativeHeights;
		Stamp.Update();
	}

	/**
	 * Get Target Precision
	 * Maximum allowed height error in world units after packing.
	 * A value of 1 means packed heights will be within 1cm of their original value.
	 * Lower values preserve more detail but use more bits per sample.
	 * For reference, a 10m height range at precision 1cm requires 10 bits.
	 * This is calculated per chunk.
	 * @param TargetPrecision Maximum allowed height error in world units after packing.
	 * A value of 1 means packed heights will be within 1cm of their original value.
	 * Lower values preserve more detail but use more bits per sample.
	 * For reference, a 10m height range at precision 1cm requires 10 bits.
	 * This is calculated per chunk.
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Get Target Precision")
	static void GetTargetPrecision(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		float& TargetPrecision)
	{
		TargetPrecision = FVoxelUtilities::MakeSafe<float>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		TargetPrecision = Stamp->TargetPrecision;
	}

	/**
	 * Set Target Precision
	 * Maximum allowed height error in world units after packing.
	 * A value of 1 means packed heights will be within 1cm of their original value.
	 * Lower values preserve more detail but use more bits per sample.
	 * For reference, a 10m height range at precision 1cm requires 10 bits.
	 * This is calculated per chunk.
	 * This will automatically refresh the stamp
	 * @param TargetPrecision Maximum allowed height error in world units after packing.
	 * A value of 1 means packed heights will be within 1cm of their original value.
	 * Lower values preserve more detail but use more bits per sample.
	 * For reference, a 10m height range at precision 1cm requires 10 bits.
	 * This is calculated per chunk.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Set Target Precision", meta = (TargetPrecision = "1.000000"))
	static void SetTargetPrecision(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightSculptStampRef& OutStamp,
		float TargetPrecision)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->TargetPrecision = TargetPrecision;
		Stamp.Update();
	}

	/**
	 * Get Is Infinite
	 * Set this to true for runtime stamps
	 * Will ensure only updated chunks are invalidated when sculpting outside the existing stamp bounds
	 * Do not set this to true on too many stamps, this adds a small overhead to all computed chunks
	 * @param bIsInfinite Set this to true for runtime stamps
	 * Will ensure only updated chunks are invalidated when sculpting outside the existing stamp bounds
	 * Do not set this to true on too many stamps, this adds a small overhead to all computed chunks
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Get Is Infinite")
	static void GetIsInfinite(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		bool& bIsInfinite)
	{
		bIsInfinite = FVoxelUtilities::MakeSafe<bool>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		bIsInfinite = Stamp->bIsInfinite;
	}

	/**
	 * Set Is Infinite
	 * Set this to true for runtime stamps
	 * Will ensure only updated chunks are invalidated when sculpting outside the existing stamp bounds
	 * Do not set this to true on too many stamps, this adds a small overhead to all computed chunks
	 * This will automatically refresh the stamp
	 * @param bIsInfinite Set this to true for runtime stamps
	 * Will ensure only updated chunks are invalidated when sculpting outside the existing stamp bounds
	 * Do not set this to true on too many stamps, this adds a small overhead to all computed chunks
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Set Is Infinite")
	static void SetIsInfinite(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightSculptStampRef& OutStamp,
		bool bIsInfinite)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->bIsInfinite = bIsInfinite;
		Stamp.Update();
	}

	/**
	 * Get Near Max LOD
	 * Any chunk whose LOD is <= to this will use the Near quality
	 * @param NearMaxLOD Any chunk whose LOD is <= to this will use the Near quality
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Get Near Max LOD")
	static void GetNearMaxLOD(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		int32& NearMaxLOD)
	{
		NearMaxLOD = FVoxelUtilities::MakeSafe<int32>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		NearMaxLOD = Stamp->NearMaxLOD;
	}

	/**
	 * Set Near Max LOD
	 * Any chunk whose LOD is <= to this will use the Near quality
	 * This will automatically refresh the stamp
	 * @param NearMaxLOD Any chunk whose LOD is <= to this will use the Near quality
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Set Near Max LOD", meta = (NearMaxLOD = "3"))
	static void SetNearMaxLOD(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightSculptStampRef& OutStamp,
		int32 NearMaxLOD)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->NearMaxLOD = NearMaxLOD;
		Stamp.Update();
	}

	/**
	 * Get Mid Max LOD
	 * Any chunk whose LOD is <= to this will use the Mid quality
	 * @param MidMaxLOD Any chunk whose LOD is <= to this will use the Mid quality
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Get Mid Max LOD")
	static void GetMidMaxLOD(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		int32& MidMaxLOD)
	{
		MidMaxLOD = FVoxelUtilities::MakeSafe<int32>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		MidMaxLOD = Stamp->MidMaxLOD;
	}

	/**
	 * Set Mid Max LOD
	 * Any chunk whose LOD is <= to this will use the Mid quality
	 * This will automatically refresh the stamp
	 * @param MidMaxLOD Any chunk whose LOD is <= to this will use the Mid quality
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Set Mid Max LOD", meta = (MidMaxLOD = "6"))
	static void SetMidMaxLOD(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightSculptStampRef& OutStamp,
		int32 MidMaxLOD)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->MidMaxLOD = MidMaxLOD;
		Stamp.Update();
	}

	/**
	 * Get Stack Override
	 * Use this if this stamp is not rendered in the Voxel World stack
	 * This stack will be used during sculpting to query the distances before any sculpt is applied
	 * @param StackOverride Use this if this stamp is not rendered in the Voxel World stack
	 * This stack will be used during sculpting to query the distances before any sculpt is applied
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Get Stack Override")
	static void GetStackOverride(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		UVoxelLayerStack*& StackOverride)
	{
		StackOverride = FVoxelUtilities::MakeSafe<UVoxelLayerStack*>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		StackOverride = Stamp->StackOverride;
	}

	/**
	 * Set Stack Override
	 * Use this if this stamp is not rendered in the Voxel World stack
	 * This stack will be used during sculpting to query the distances before any sculpt is applied
	 * This will automatically refresh the stamp
	 * @param StackOverride Use this if this stamp is not rendered in the Voxel World stack
	 * This stack will be used during sculpting to query the distances before any sculpt is applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Height Sculpt", DisplayName = "Set Stack Override")
	static void SetStackOverride(
		UPARAM(Required) FVoxelHeightSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelHeightSculptStampRef& OutStamp,
		UVoxelLayerStack* StackOverride)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->StackOverride = StackOverride;
		Stamp.Update();
	}
};