// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Shape/VoxelPlaneShape.h"
#include "Shape/VoxelShapeStampRef.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#include "VoxelPlaneShape_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelPlaneShape_K2 : public UVoxelStampBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @param Height Relative to Size, by how much to extend the distance field up and down
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Shape|Plane", DisplayName = "Make Voxel Plane Shape Stamp", meta = (Keywords = "Construct, Create", Size = "(X=1000.000000,Y=1000.000000)", Height = "1.000000", Layer = "/Script/Voxel.VoxelVolumeLayer'/Voxel/Default/DefaultVolumeLayer.DefaultVolumeLayer'", AutoCreateRefTerm = "AdditionalLayers", BoundsExtensionMultiplier = "1.000000", MaximumBoundsExtension = "1000.000000", Behavior = "AffectAll", Smoothness = "100.000000", LODRange = "(Min=0,Max=32)", bApplyOnVoid = "True"))
	static void Make(
		FVoxelShapeStampRef& Stamp,
		FVector2D Size,
		double Height,
		UVoxelSurfaceTypeInterface* SurfaceType,
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
		Stamp = FVoxelShapeStampRef::New();
		Stamp->SurfaceType = SurfaceType;
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

		TVoxelInstancedStruct<FVoxelPlaneShape> Shape(FVoxelPlaneShape::StaticStruct());
		Shape->Size = Size;
		Shape->Height = Height;

		Stamp->Shape = Shape;
	}

	/**
	 * @param Height Relative to Size, by how much to extend the distance field up and down
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
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Shape|Plane", DisplayName = "Break Voxel Plane Shape Stamp", meta = (Keywords = "Break", AutoCreateRefTerm = "AdditionalLayers"))
	static void Break(
		FVoxelShapeStampRef& Stamp,
		FVector2D& Size,
		double& Height,
		UVoxelSurfaceTypeInterface*& SurfaceType,
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
		SurfaceType = FVoxelUtilities::MakeSafe<UVoxelSurfaceTypeInterface*>();
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
		Size = FVoxelUtilities::MakeSafe<FVector2D>();
		Height = FVoxelUtilities::MakeSafe<double>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		FVoxelPlaneShape* Shape = Stamp->Shape->As<FVoxelPlaneShape>();
		if (!Shape)
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		SurfaceType = Stamp->SurfaceType;
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
		Size = Shape->Size;
		Height = Shape->Height;
	}

	// Get Size
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Shape|Plane", DisplayName = "Get Size")
	static void GetSize(
		UPARAM(Required) FVoxelShapeStampRef Stamp,
		FVector2D& Size)
	{
		Size = FVoxelUtilities::MakeSafe<FVector2D>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		FVoxelPlaneShape* Shape = Stamp->Shape->As<FVoxelPlaneShape>();
		if (!Shape)
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Size = Shape->Size;
	}

	// Set Size
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Shape|Plane", DisplayName = "Set Size", meta = (Size = "(X=1000.000000,Y=1000.000000)"))
	static void SetSize(
		UPARAM(Required) FVoxelShapeStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelShapeStampRef& OutStamp,
		FVector2D Size)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		FVoxelPlaneShape* Shape = Stamp->Shape->As<FVoxelPlaneShape>();
		if (!Shape)
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Shape->Size = Size;
		Stamp.Update();
	}

	/**
	 * Get Height
	 * Relative to Size, by how much to extend the distance field up and down
	 * @param Height Relative to Size, by how much to extend the distance field up and down
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Shape|Plane", DisplayName = "Get Height")
	static void GetHeight(
		UPARAM(Required) FVoxelShapeStampRef Stamp,
		double& Height)
	{
		Height = FVoxelUtilities::MakeSafe<double>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		FVoxelPlaneShape* Shape = Stamp->Shape->As<FVoxelPlaneShape>();
		if (!Shape)
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Height = Shape->Height;
	}

	/**
	 * Set Height
	 * Relative to Size, by how much to extend the distance field up and down
	 * This will automatically refresh the stamp
	 * @param Height Relative to Size, by how much to extend the distance field up and down
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Shape|Plane", DisplayName = "Set Height", meta = (Height = "1.000000"))
	static void SetHeight(
		UPARAM(Required) FVoxelShapeStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelShapeStampRef& OutStamp,
		double Height)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		FVoxelPlaneShape* Shape = Stamp->Shape->As<FVoxelPlaneShape>();
		if (!Shape)
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Shape->Height = Height;
		Stamp.Update();
	}
};