// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLatentAction.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sculpt/Height/VoxelHeightSculptBlueprintLibrary.h"
#include "VoxelHeightSculptBlueprintLibrary_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelHeightSculptBlueprintLibrary_BlueprintOnly : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Apply a sculpt graph
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Graph Graph
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void ApplySculptGraph(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D& Center,
		const float Radius = 500.0f,
		const FVoxelHeightSculptGraphWrapper& Graph = FVoxelHeightSculptGraphWrapper())
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelHeightSculptBlueprintLibrary::ApplySculptGraph(
				SculptActor,
				Center,
				Radius,
				Graph);
		});
	}

	/**
	 * Apply a sculpt graph
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Graph Graph
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void ApplySculptGraphAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D& Center,
		const float Radius = 500.0f,
		const FVoxelHeightSculptGraphWrapper& Graph = FVoxelHeightSculptGraphWrapper(),
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelHeightSculptBlueprintLibrary::ApplySculptGraph(
					SculptActor,
					Center,
					Radius,
					Graph);
			});
	}

	/**
	 * Flatten a voxel surface horizontally
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Falloff Between 0 and 1
	 * @param Type Whether to add below, remove above or do both
	 * @param TargetHeight The height to flatten towards
	 * @param Brush Optional, used to apply a pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void Flatten(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
		const float Radius = 500.0f,
		const float Falloff = 0.1f,
		const EVoxelLevelToolType Type = EVoxelLevelToolType::Additive,
		const float TargetHeight = 0.0f,
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelHeightSculptBlueprintLibrary::Flatten(
				SculptActor,
				Center,
				Radius,
				Falloff,
				Type,
				TargetHeight,
				Brush);
		});
	}

	/**
	 * Flatten a voxel surface horizontally
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Falloff Between 0 and 1
	 * @param Type Whether to add below, remove above or do both
	 * @param TargetHeight The height to flatten towards
	 * @param Brush Optional, used to apply a pattern
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void FlattenAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
		const float Radius = 500.0f,
		const float Falloff = 0.1f,
		const EVoxelLevelToolType Type = EVoxelLevelToolType::Additive,
		const float TargetHeight = 0.0f,
		const FVoxelToolBrush Brush = FVoxelToolBrush(),
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelHeightSculptBlueprintLibrary::Flatten(
					SculptActor,
					Center,
					Radius,
					Falloff,
					Type,
					TargetHeight,
					Brush);
			});
	}

	// Get Save
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void GetSave(
		FVoxelHeightSculptSave& Save,
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		bool bCompress = true)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelHeightSculptBlueprintLibrary::K2_GetSave(
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
		FVoxelHeightSculptSave& Save,
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		bool bCompress = true,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelHeightSculptBlueprintLibrary::K2_GetSave(
					Save,
					SculptActor,
					bCompress);
			});
	}

	// Load from Save
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void LoadFromSave(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		FVoxelHeightSculptSave Save)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelHeightSculptBlueprintLibrary::LoadFromSave(
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
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		FVoxelHeightSculptSave Save,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelHeightSculptBlueprintLibrary::LoadFromSave(
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
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
		const float Radius = 500.0f,
		const float Strength = 0.05f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		UVoxelSurfaceTypeInterface* SurfaceTypeToPaint = nullptr,
		const FVoxelMetadataOverrides MetadatasToPaint = FVoxelMetadataOverrides(),
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelHeightSculptBlueprintLibrary::PaintSurface(
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
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
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
				return UVoxelHeightSculptBlueprintLibrary::PaintSurface(
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
	 * Progressively/smoothly sculpt a height sculpt actor
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Sculpt speed
	 * @param Mode Whether to add or remove
	 * @param Brush Optional, used to apply a pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void SculptHeight(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
		const float Radius = 500.0f,
		const float Strength = 0.5f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelHeightSculptBlueprintLibrary::SculptHeight(
				SculptActor,
				Center,
				Radius,
				Strength,
				Mode,
				Brush);
		});
	}

	/**
	 * Progressively/smoothly sculpt a height sculpt actor
	 * @param SculptActor The sculpt actor to sculpt
	 * @param Center Center, in world space
	 * @param Radius Radius, in world space
	 * @param Strength Sculpt speed
	 * @param Mode Whether to add or remove
	 * @param Brush Optional, used to apply a pattern
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void SculptHeightAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
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
				return UVoxelHeightSculptBlueprintLibrary::SculptHeight(
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
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
		const float Radius = 1000.0f,
		const float Strength = 1.0f,
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelHeightSculptBlueprintLibrary::Smooth(
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
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
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
				return UVoxelHeightSculptBlueprintLibrary::Smooth(
					SculptActor,
					Center,
					Radius,
					Strength,
					Brush);
			});
	}
};