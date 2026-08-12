// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeStampRef.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#include "VoxelVolumeStamp_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelVolumeStamp_K2 : public UVoxelStampBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Casting", meta = (ExpandEnumAsExecs = "Result"))
	static FVoxelVolumeStampRef CastToVolumeStamp(
		const FVoxelStampRef Stamp,
		EVoxelStampCastResult& Result)
	{
		return CastToStampImpl<FVoxelVolumeStamp>(Stamp, Result);
	}

	// Make a copy of this stamp
	// You can then call Set XXXX on the copy without having the original stamp be modified
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume", DisplayName = "Get Volume Stamp")
	static void MakeCopy(
		const FVoxelVolumeStampRef Stamp,
		FVoxelVolumeStampRef& Copy)
	{
		Copy = FVoxelVolumeStampRef(Stamp.MakeCopy());
	}

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	static void Make(FVoxelVolumeStampRef& Stamp)
	{
		Stamp = {};
	}

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	static void Break(const FVoxelVolumeStampRef Stamp)
	{
	}

	/**
	 * Get Layer
	 * Layer that this stamps belong to
	 * You can control the order of layers in Layer Stacks
	 * You can select the layer stack to use in your Voxel World or PCG Sampler settings
	 * @param Layer Layer that this stamps belong to
	 * You can control the order of layers in Layer Stacks
	 * You can select the layer stack to use in your Voxel World or PCG Sampler settings
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume", DisplayName = "Get Layer")
	static void GetLayer(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		UVoxelVolumeLayer*& Layer)
	{
		Layer = FVoxelUtilities::MakeSafe<UVoxelVolumeLayer*>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Layer = Stamp->Layer;
	}

	/**
	 * Set Layer
	 * Layer that this stamps belong to
	 * You can control the order of layers in Layer Stacks
	 * You can select the layer stack to use in your Voxel World or PCG Sampler settings
	 * This will automatically refresh the stamp
	 * @param Layer Layer that this stamps belong to
	 * You can control the order of layers in Layer Stacks
	 * You can select the layer stack to use in your Voxel World or PCG Sampler settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume", DisplayName = "Set Layer", meta = (Layer = "/Script/Voxel.VoxelVolumeLayer'/Voxel/Default/DefaultVolumeLayer.DefaultVolumeLayer'"))
	static void SetLayer(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeStampRef& OutStamp,
		UVoxelVolumeLayer* Layer)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->Layer = Layer;
		Stamp.Update();
	}

	// Get Blend Mode
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume", DisplayName = "Get Blend Mode")
	static void GetBlendMode(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		EVoxelVolumeBlendMode& BlendMode)
	{
		BlendMode = FVoxelUtilities::MakeSafe<EVoxelVolumeBlendMode>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		BlendMode = Stamp->BlendMode;
	}

	// Set Blend Mode
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume", DisplayName = "Set Blend Mode")
	static void SetBlendMode(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeStampRef& OutStamp,
		EVoxelVolumeBlendMode BlendMode)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->BlendMode = BlendMode;
		Stamp.Update();
	}

	// Get Additional Layers
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume", DisplayName = "Get Additional Layers", meta = (AutoCreateRefTerm = "AdditionalLayers"))
	static void GetAdditionalLayers(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		TArray<UVoxelVolumeLayer*>& AdditionalLayers)
	{
		AdditionalLayers = FVoxelUtilities::MakeSafe<TArray<UVoxelVolumeLayer*>>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		AdditionalLayers = Stamp->AdditionalLayers;
	}

	// Set Additional Layers
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume", DisplayName = "Set Additional Layers", meta = (AutoCreateRefTerm = "AdditionalLayers"))
	static void SetAdditionalLayers(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeStampRef& OutStamp,
		TArray<UVoxelVolumeLayer*> AdditionalLayers)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->AdditionalLayers = AdditionalLayers;
		Stamp.Update();
	}

	/**
	 * Get Bounds Extension Multiplier
	 * By how much to extend the bounds, relative to the bounds size
	 * Increase this if you are using a high smoothness
	 * Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	 * @param BoundsExtensionMultiplier By how much to extend the bounds, relative to the bounds size
	 * Increase this if you are using a high smoothness
	 * Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume", DisplayName = "Get Bounds Extension Multiplier")
	static void GetBoundsExtensionMultiplier(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		float& BoundsExtensionMultiplier)
	{
		BoundsExtensionMultiplier = FVoxelUtilities::MakeSafe<float>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		BoundsExtensionMultiplier = Stamp->BoundsExtensionMultiplier;
	}

	/**
	 * Set Bounds Extension Multiplier
	 * By how much to extend the bounds, relative to the bounds size
	 * Increase this if you are using a high smoothness
	 * Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	 * This will automatically refresh the stamp
	 * @param BoundsExtensionMultiplier By how much to extend the bounds, relative to the bounds size
	 * Increase this if you are using a high smoothness
	 * Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume", DisplayName = "Set Bounds Extension Multiplier", meta = (BoundsExtensionMultiplier = "1.000000"))
	static void SetBoundsExtensionMultiplier(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeStampRef& OutStamp,
		float BoundsExtensionMultiplier)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->BoundsExtensionMultiplier = BoundsExtensionMultiplier;
		Stamp.Update();
	}

	/**
	 * Get Maximum Bounds Extension
	 * Bounds Extension will be limited to this value in cm, regardless of what the multiplier is set to
	 * @param MaximumBoundsExtension Bounds Extension will be limited to this value in cm, regardless of what the multiplier is set to
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Volume", DisplayName = "Get Maximum Bounds Extension")
	static void GetMaximumBoundsExtension(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		float& MaximumBoundsExtension)
	{
		MaximumBoundsExtension = FVoxelUtilities::MakeSafe<float>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		MaximumBoundsExtension = Stamp->MaximumBoundsExtension;
	}

	/**
	 * Set Maximum Bounds Extension
	 * Bounds Extension will be limited to this value in cm, regardless of what the multiplier is set to
	 * This will automatically refresh the stamp
	 * @param MaximumBoundsExtension Bounds Extension will be limited to this value in cm, regardless of what the multiplier is set to
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Volume", DisplayName = "Set Maximum Bounds Extension", meta = (MaximumBoundsExtension = "1000.000000"))
	static void SetMaximumBoundsExtension(
		UPARAM(Required) FVoxelVolumeStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelVolumeStampRef& OutStamp,
		float MaximumBoundsExtension)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->MaximumBoundsExtension = MaximumBoundsExtension;
		Stamp.Update();
	}
};