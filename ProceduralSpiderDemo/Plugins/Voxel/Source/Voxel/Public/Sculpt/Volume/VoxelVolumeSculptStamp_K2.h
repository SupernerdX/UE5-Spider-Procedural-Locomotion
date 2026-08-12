// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeSculptStampRef.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#include "VoxelVolumeSculptStamp_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelVolumeSculptStamp_K2 : public UVoxelStampBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Casting", meta = (ExpandEnumAsExecs = "Result"))
	static FVoxelVolumeSculptStampRef CastToVolumeSculptStamp(
		const FVoxelStampRef Stamp,
		EVoxelStampCastResult& Result)
	{
		return CastToStampImpl<FVoxelVolumeSculptStamp>(Stamp, Result);
	}

	// Make a copy of this stamp
	// You can then call Set XXXX on the copy without having the original stamp be modified
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Get Volume Sculpt Stamp")
	static void MakeCopy(
		const FVoxelVolumeSculptStampRef Stamp,
		FVoxelVolumeSculptStampRef& Copy)
	{
		Copy = FVoxelVolumeSculptStampRef(Stamp.MakeCopy());
	}

	/**
	 * @param bStoreMovableDistances If true, stores the distance field into two floats: a Additive and a Subtractive distance
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * Will use 2-4x more memory/disk space
	 * @param MaxErrorPercentage Max error percentage allowed when saving
	 * This is used to pick the bit depth to use when saving a chunk
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
	 * @param BoundsExtensionMultiplier By how much to extend the bounds, relative to the bounds size
	 * Increase this if you are using a high smoothness
	 * Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	 * @param MaximumBoundsExtension Bounds Extension will be limited to this value in cm, regardless of what the multiplier is set to
	 * @param Priority Priority of the stamp within its layer
	 * Higher priority stamps will be applied last
	 * @param LODRange This stamp will only be applied on LODs within this range (inclusive)
	 * @param bDisableStampSelection If true you won\'t be able to select this stamp by clicking on it
	 * @param bApplyOnVoid If false, this stamp will only apply on parts where another stamp has been applied first
	 * This is useful to avoid having stamps going beyond world bounds
	 * Only used if BlendMode is not Override nor Intersect
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Make Voxel Volume Sculpt Stamp", meta = (Keywords = "Construct, Create", Scale = "100.000000", bStoreMovableDistances = "True", MaxErrorPercentage = "0.500000", NearMaxLOD = "3", MidMaxLOD = "6", Layer = "/Script/Voxel.VoxelVolumeLayer'/Voxel/Default/DefaultVolumeLayer.DefaultVolumeLayer'", BlendMode = "Override", AutoCreateRefTerm = "AdditionalLayers", BoundsExtensionMultiplier = "1.000000", MaximumBoundsExtension = "1000.000000", Behavior = "AffectAll", Smoothness = "100.000000", LODRange = "(Min=0,Max=32)", bApplyOnVoid = "True"))
	static void Make(
		FVoxelVolumeSculptStampRef& Stamp,
		float Scale,
		bool bStoreMovableDistances,
		float MaxErrorPercentage,
		bool bIsInfinite,
		int32 NearMaxLOD,
		int32 MidMaxLOD,
		UVoxelLayerStack* StackOverride,
		UVoxelVolumeLayer* Layer,
		EVoxelVolumeBlendMode BlendMode,
		TArray<UVoxelVolumeLayer*> AdditionalLayers,
		float BoundsExtensionMultiplier,
		float MaximumBoundsExtension,
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
		Stamp = FVoxelVolumeSculptStampRef::New();
		Stamp->Scale = Scale;
		Stamp->bStoreMovableDistances = bStoreMovableDistances;
		Stamp->MaxErrorPercentage = MaxErrorPercentage;
		Stamp->bIsInfinite = bIsInfinite;
		Stamp->NearMaxLOD = NearMaxLOD;
		Stamp->MidMaxLOD = MidMaxLOD;
		Stamp->StackOverride = StackOverride;
		Stamp->Layer = Layer;
		Stamp->BlendMode = BlendMode;
		Stamp->AdditionalLayers = AdditionalLayers;
		Stamp->BoundsExtensionMultiplier = BoundsExtensionMultiplier;
		Stamp->MaximumBoundsExtension = MaximumBoundsExtension;
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
	 * @param bStoreMovableDistances If true, stores the distance field into two floats: a Additive and a Subtractive distance
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * Will use 2-4x more memory/disk space
	 * @param MaxErrorPercentage Max error percentage allowed when saving
	 * This is used to pick the bit depth to use when saving a chunk
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
	 * @param BoundsExtensionMultiplier By how much to extend the bounds, relative to the bounds size
	 * Increase this if you are using a high smoothness
	 * Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	 * @param MaximumBoundsExtension Bounds Extension will be limited to this value in cm, regardless of what the multiplier is set to
	 * @param Priority Priority of the stamp within its layer
	 * Higher priority stamps will be applied last
	 * @param LODRange This stamp will only be applied on LODs within this range (inclusive)
	 * @param bDisableStampSelection If true you won\'t be able to select this stamp by clicking on it
	 * @param bApplyOnVoid If false, this stamp will only apply on parts where another stamp has been applied first
	 * This is useful to avoid having stamps going beyond world bounds
	 * Only used if BlendMode is not Override nor Intersect
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Break Voxel Volume Sculpt Stamp", meta = (Keywords = "Break", AutoCreateRefTerm = "AdditionalLayers"))
	static void Break(
		const FVoxelVolumeSculptStampRef Stamp,
		float& Scale,
		bool& bStoreMovableDistances,
		float& MaxErrorPercentage,
		bool& bIsInfinite,
		int32& NearMaxLOD,
		int32& MidMaxLOD,
		UVoxelLayerStack*& StackOverride,
		UVoxelVolumeLayer*& Layer,
		EVoxelVolumeBlendMode& BlendMode,
		TArray<UVoxelVolumeLayer*>& AdditionalLayers,
		float& BoundsExtensionMultiplier,
		float& MaximumBoundsExtension,
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
		Scale = FVoxelUtilities::MakeSafe<float>();
		bStoreMovableDistances = FVoxelUtilities::MakeSafe<bool>();
		MaxErrorPercentage = FVoxelUtilities::MakeSafe<float>();
		bIsInfinite = FVoxelUtilities::MakeSafe<bool>();
		NearMaxLOD = FVoxelUtilities::MakeSafe<int32>();
		MidMaxLOD = FVoxelUtilities::MakeSafe<int32>();
		StackOverride = FVoxelUtilities::MakeSafe<UVoxelLayerStack*>();
		Layer = FVoxelUtilities::MakeSafe<UVoxelVolumeLayer*>();
		BlendMode = FVoxelUtilities::MakeSafe<EVoxelVolumeBlendMode>();
		AdditionalLayers = FVoxelUtilities::MakeSafe<TArray<UVoxelVolumeLayer*>>();
		BoundsExtensionMultiplier = FVoxelUtilities::MakeSafe<float>();
		MaximumBoundsExtension = FVoxelUtilities::MakeSafe<float>();
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

		Scale = Stamp->Scale;
		bStoreMovableDistances = Stamp->bStoreMovableDistances;
		MaxErrorPercentage = Stamp->MaxErrorPercentage;
		bIsInfinite = Stamp->bIsInfinite;
		NearMaxLOD = Stamp->NearMaxLOD;
		MidMaxLOD = Stamp->MidMaxLOD;
		StackOverride = Stamp->StackOverride;
		Layer = Stamp->Layer;
		BlendMode = Stamp->BlendMode;
		AdditionalLayers = Stamp->AdditionalLayers;
		BoundsExtensionMultiplier = Stamp->BoundsExtensionMultiplier;
		MaximumBoundsExtension = Stamp->MaximumBoundsExtension;
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

	// Get Scale
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Get Scale")
	static void GetScale(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		float& Scale)
	{
		Scale = FVoxelUtilities::MakeSafe<float>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Scale = Stamp->Scale;
	}

	// Set Scale
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Set Scale", meta = (Scale = "100.000000"))
	static void SetScale(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeSculptStampRef& OutStamp,
		float Scale)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->Scale = Scale;
		Stamp.Update();
	}

	/**
	 * Get Store Movable Distances
	 * If true, stores the distance field into two floats: a Additive and a Subtractive distance
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * Will use 2-4x more memory/disk space
	 * @param bStoreMovableDistances If true, stores the distance field into two floats: a Additive and a Subtractive distance
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * Will use 2-4x more memory/disk space
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Get Store Movable Distances")
	static void GetStoreMovableDistances(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		bool& bStoreMovableDistances)
	{
		bStoreMovableDistances = FVoxelUtilities::MakeSafe<bool>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		bStoreMovableDistances = Stamp->bStoreMovableDistances;
	}

	/**
	 * Set Store Movable Distances
	 * If true, stores the distance field into two floats: a Additive and a Subtractive distance
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * Will use 2-4x more memory/disk space
	 * This will automatically refresh the stamp
	 * @param bStoreMovableDistances If true, stores the distance field into two floats: a Additive and a Subtractive distance
	 * This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	 * Will use 2-4x more memory/disk space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Set Store Movable Distances", meta = (bStoreMovableDistances = "True"))
	static void SetStoreMovableDistances(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeSculptStampRef& OutStamp,
		bool bStoreMovableDistances)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->bStoreMovableDistances = bStoreMovableDistances;
		Stamp.Update();
	}

	/**
	 * Get Max Error Percentage
	 * Max error percentage allowed when saving
	 * This is used to pick the bit depth to use when saving a chunk
	 * @param MaxErrorPercentage Max error percentage allowed when saving
	 * This is used to pick the bit depth to use when saving a chunk
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Get Max Error Percentage")
	static void GetMaxErrorPercentage(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		float& MaxErrorPercentage)
	{
		MaxErrorPercentage = FVoxelUtilities::MakeSafe<float>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		MaxErrorPercentage = Stamp->MaxErrorPercentage;
	}

	/**
	 * Set Max Error Percentage
	 * Max error percentage allowed when saving
	 * This is used to pick the bit depth to use when saving a chunk
	 * This will automatically refresh the stamp
	 * @param MaxErrorPercentage Max error percentage allowed when saving
	 * This is used to pick the bit depth to use when saving a chunk
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Set Max Error Percentage", meta = (MaxErrorPercentage = "0.500000"))
	static void SetMaxErrorPercentage(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeSculptStampRef& OutStamp,
		float MaxErrorPercentage)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->MaxErrorPercentage = MaxErrorPercentage;
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
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Get Is Infinite")
	static void GetIsInfinite(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Set Is Infinite")
	static void SetIsInfinite(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeSculptStampRef& OutStamp,
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
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Get Near Max LOD")
	static void GetNearMaxLOD(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Set Near Max LOD", meta = (NearMaxLOD = "3"))
	static void SetNearMaxLOD(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeSculptStampRef& OutStamp,
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
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Get Mid Max LOD")
	static void GetMidMaxLOD(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Set Mid Max LOD", meta = (MidMaxLOD = "6"))
	static void SetMidMaxLOD(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeSculptStampRef& OutStamp,
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
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Get Stack Override")
	static void GetStackOverride(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume Sculpt", DisplayName = "Set Stack Override")
	static void SetStackOverride(
		UPARAM(Required) FVoxelVolumeSculptStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeSculptStampRef& OutStamp,
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