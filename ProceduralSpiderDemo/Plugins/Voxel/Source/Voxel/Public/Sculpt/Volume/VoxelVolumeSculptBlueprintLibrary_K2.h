// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLatentAction.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sculpt/Volume/VoxelVolumeSculptBlueprintLibrary.h"
#include "VoxelVolumeSculptBlueprintLibrary_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelVolumeSculptBlueprintLibrary_BlueprintOnly : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Apply a sculpt graph
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Graph Graph
	 * @param Rotation Rotation to apply to the graph
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void ApplySculptGraph(
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector& Center,
		const float Radius = 500.0f,
		const FVoxelVolumeSculptGraphWrapper& Graph = FVoxelVolumeSculptGraphWrapper(),
		const FRotator& Rotation = FRotator::ZeroRotator)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::ApplySculptGraph(
				SculptActor,
				Center,
				Radius,
				Graph,
				Rotation);
		});
	}

	/**
	 * Apply a sculpt graph
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Graph Graph
	 * @param Rotation Rotation to apply to the graph
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void ApplySculptGraphAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector& Center,
		const float Radius = 500.0f,
		const FVoxelVolumeSculptGraphWrapper& Graph = FVoxelVolumeSculptGraphWrapper(),
		const FRotator& Rotation = FRotator::ZeroRotator,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::ApplySculptGraph(
					SculptActor,
					Center,
					Radius,
					Graph,
					Rotation);
			});
	}

	/**
	 * Flatten a voxel surface horizontally
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Normal Normal of the flattening, usually (0, 0, 1)
	 * @param Radius Radius, in world space
	 * @param Height How far up/down to sculpt
	 * @param Falloff Between 0 and 1
	 * @param Type Whether to add below, remove above or do both
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void Flatten(
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const FVector Normal = FVector::UpVector,
		const float Radius = 500.0f,
		const float Height = 1000.0f,
		const float Falloff = 0.1f,
		const EVoxelLevelToolType Type = EVoxelLevelToolType::Additive)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::Flatten(
				SculptActor,
				Center,
				Normal,
				Radius,
				Height,
				Falloff,
				Type);
		});
	}

	/**
	 * Flatten a voxel surface horizontally
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Normal Normal of the flattening, usually (0, 0, 1)
	 * @param Radius Radius, in world space
	 * @param Height How far up/down to sculpt
	 * @param Falloff Between 0 and 1
	 * @param Type Whether to add below, remove above or do both
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void FlattenAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const FVector Normal = FVector::UpVector,
		const float Radius = 500.0f,
		const float Height = 1000.0f,
		const float Falloff = 0.1f,
		const EVoxelLevelToolType Type = EVoxelLevelToolType::Additive,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::Flatten(
					SculptActor,
					Center,
					Normal,
					Radius,
					Height,
					Falloff,
					Type);
			});
	}

	// Get Save
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void GetSave(
		FVoxelVolumeSculptSave& Save,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		bool bCompress = true)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::K2_GetSave(
				Save,
				SculptActor,
				bCompress);
		});
	}

	/**
	 * Get Save
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void GetSaveAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelVolumeSculptSave& Save,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		bool bCompress = true,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::K2_GetSave(
					Save,
					SculptActor,
					bCompress);
			});
	}

	// Load from Save
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void LoadFromSave(
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		FVoxelVolumeSculptSave Save)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::LoadFromSave(
				SculptActor,
				Save);
		});
	}

	/**
	 * Load from Save
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void LoadFromSaveAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		FVoxelVolumeSculptSave Save,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::LoadFromSave(
					SculptActor,
					Save);
			});
	}

	/**
	 * Paint a surface type on a sculpt stamp
	 * @param SculptActor The sculpt actor to paint
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Strength, usually between 0 and 1
	 * @param Mode Add or remove surface type
	 * @param SurfaceTypeToPaint Surface type to paint
	 * @param MetadatasToPaint Metadatas to paint
	 * @param Brush Optional, used to apply a pattern to the paint
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void PaintSurface(
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 500.0f,
		const float Strength = 0.05f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		UVoxelSurfaceTypeInterface* SurfaceTypeToPaint = nullptr,
		const FVoxelMetadataOverrides MetadatasToPaint = FVoxelMetadataOverrides(),
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::PaintSurface(
				SculptActor,
				Center,
				Radius,
				Strength,
				Mode,
				SurfaceTypeToPaint,
				MetadatasToPaint,
				Brush);
		});
	}

	/**
	 * Paint a surface type on a sculpt stamp
	 * @param SculptActor The sculpt actor to paint
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Strength, usually between 0 and 1
	 * @param Mode Add or remove surface type
	 * @param SurfaceTypeToPaint Surface type to paint
	 * @param MetadatasToPaint Metadatas to paint
	 * @param Brush Optional, used to apply a pattern to the paint
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void PaintSurfaceAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 500.0f,
		const float Strength = 0.05f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		UVoxelSurfaceTypeInterface* SurfaceTypeToPaint = nullptr,
		const FVoxelMetadataOverrides MetadatasToPaint = FVoxelMetadataOverrides(),
		const FVoxelToolBrush Brush = FVoxelToolBrush(),
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::PaintSurface(
					SculptActor,
					Center,
					Radius,
					Strength,
					Mode,
					SurfaceTypeToPaint,
					MetadatasToPaint,
					Brush);
			});
	}

	/**
	 * Move a voxel surface to towards a plane. Useful to sculpt angles.
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Strength, usually between 0 and 1
	 * @param PlanePoint A point on the plane to fit towards. Usually the click position.
	 * @param PlaneNormal The plane normal that will decide the angle
	 * @param MergeMode Merge mode
	 * @param Brush Optional, used to apply a pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void SculptAngle(
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 500.0f,
		const float Strength = 1.0f,
		const FVector& PlanePoint = FVector::ZeroVector,
		const FVector& PlaneNormal = FVector::UpVector,
		const EVoxelSDFMergeMode MergeMode = EVoxelSDFMergeMode::Override,
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::SculptAngle(
				SculptActor,
				Center,
				Radius,
				Strength,
				PlanePoint,
				PlaneNormal,
				MergeMode,
				Brush);
		});
	}

	/**
	 * Move a voxel surface to towards a plane. Useful to sculpt angles.
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Strength, usually between 0 and 1
	 * @param PlanePoint A point on the plane to fit towards. Usually the click position.
	 * @param PlaneNormal The plane normal that will decide the angle
	 * @param MergeMode Merge mode
	 * @param Brush Optional, used to apply a pattern
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void SculptAngleAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 500.0f,
		const float Strength = 1.0f,
		const FVector& PlanePoint = FVector::ZeroVector,
		const FVector& PlaneNormal = FVector::UpVector,
		const EVoxelSDFMergeMode MergeMode = EVoxelSDFMergeMode::Override,
		const FVoxelToolBrush Brush = FVoxelToolBrush(),
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::SculptAngle(
					SculptActor,
					Center,
					Radius,
					Strength,
					PlanePoint,
					PlaneNormal,
					MergeMode,
					Brush);
			});
	}

	/**
	 * Add or remove a sphere
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Size Size, in world space
	 * @param Rotation Rotation, in world space
	 * @param Mode Whether to add or remove
	 * @param Smoothness Smoothness of the add/remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void SculptCube(
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const FVector Size = FVector(1000.0, 1000.0, 1000.0),
		const FRotator Rotation = FRotator::ZeroRotator,
		const float Roundness = 0.0f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		const float Smoothness = 0.0f)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::SculptCube(
				SculptActor,
				Center,
				Size,
				Rotation,
				Roundness,
				Mode,
				Smoothness);
		});
	}

	/**
	 * Add or remove a sphere
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Size Size, in world space
	 * @param Rotation Rotation, in world space
	 * @param Mode Whether to add or remove
	 * @param Smoothness Smoothness of the add/remove
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void SculptCubeAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const FVector Size = FVector(1000.0, 1000.0, 1000.0),
		const FRotator Rotation = FRotator::ZeroRotator,
		const float Roundness = 0.0f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		const float Smoothness = 0.0f,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::SculptCube(
					SculptActor,
					Center,
					Size,
					Rotation,
					Roundness,
					Mode,
					Smoothness);
			});
	}

	/**
	 * Add or remove a sphere
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Mode Whether to add or remove
	 * @param Smoothness Smoothness of the add/remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void SculptSphere(
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 1000.0f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		const float Smoothness = 0.0f)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::SculptSphere(
				SculptActor,
				Center,
				Radius,
				Mode,
				Smoothness);
		});
	}

	/**
	 * Add or remove a sphere
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Mode Whether to add or remove
	 * @param Smoothness Smoothness of the add/remove
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void SculptSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 1000.0f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		const float Smoothness = 0.0f,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::SculptSphere(
					SculptActor,
					Center,
					Radius,
					Mode,
					Smoothness);
			});
	}

	/**
	 * Progressively/smoothly sculpt a voxel surface
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Sculpt speed
	 * @param Mode Whether to add or remove
	 * @param Brush Optional, used to apply a pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void SculptSurface(
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 500.0f,
		const float Strength = 0.5f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::SculptSurface(
				SculptActor,
				Center,
				Radius,
				Strength,
				Mode,
				Brush);
		});
	}

	/**
	 * Progressively/smoothly sculpt a voxel surface
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Sculpt speed
	 * @param Mode Whether to add or remove
	 * @param Brush Optional, used to apply a pattern
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void SculptSurfaceAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 500.0f,
		const float Strength = 0.5f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		const FVoxelToolBrush Brush = FVoxelToolBrush(),
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::SculptSurface(
					SculptActor,
					Center,
					Radius,
					Strength,
					Mode,
					Brush);
			});
	}

	/**
	 * Smooth a voxel sculpt stamp
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Strength of the smoothing
	 * @param Brush Optional, used to apply a pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void Smooth(
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 1000.0f,
		const float Strength = 1.0f,
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelVolumeSculptBlueprintLibrary::Smooth(
				SculptActor,
				Center,
				Radius,
				Strength,
				Brush);
		});
	}

	/**
	 * Smooth a voxel sculpt stamp
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Strength of the smoothing
	 * @param Brush Optional, used to apply a pattern
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void SmoothAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptVolume* SculptActor,
		const FVector Center,
		const float Radius = 1000.0f,
		const float Strength = 1.0f,
		const FVoxelToolBrush Brush = FVoxelToolBrush(),
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelVolumeSculptBlueprintLibrary::Smooth(
					SculptActor,
					Center,
					Radius,
					Strength,
					Brush);
			});
	}
};