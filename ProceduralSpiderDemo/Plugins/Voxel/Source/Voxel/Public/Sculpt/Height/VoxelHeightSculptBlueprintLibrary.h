// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sculpt/VoxelSculptSave.h"
#include "Sculpt/Height/Tools/VoxelPaintHeightModifier.h"
#include "Sculpt/Height/Tools/VoxelFlattenHeightModifier.h"
#include "Sculpt/Height/Tools/VoxelGraphHeightModifier.h"
#include "Sculpt/Height/Tools/VoxelSmoothHeightModifier.h"
#include "Sculpt/Height/Tools/VoxelSculptHeightModifier.h"
#include "Sculpt/Height/VoxelHeightSculptGraphWrapper.h"
#include "VoxelHeightSculptBlueprintLibrary.generated.h"

class UVoxelSculptHeightAsset;

UCLASS()
class VOXEL_API UVoxelHeightSculptBlueprintLibrary  : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static bool IsValidSave(FVoxelHeightSculptSave Save);

	UFUNCTION(BlueprintPure, Category = "Voxel")
	static bool IsCompressedSave(FVoxelHeightSculptSave Save);

	UFUNCTION(BlueprintPure, Category = "Voxel")
	static int64 GetSaveSize(FVoxelHeightSculptSave Save);

public:
	// Clear all sculpt data
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void ClearSculptData(UPARAM(Required) AVoxelSculptHeight* SculptActor);

	// Clear all sculpt cache
	// This will free up memory but subsequent edits might be slightly slower
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void ClearSculptCache(UPARAM(Required) AVoxelSculptHeight* SculptActor);

	// Will assign sculpt asset to sculpt actor
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static void SetSculptAsset(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		UVoxelSculptHeightAsset* Asset);

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static FVoxelFuture K2_GetSave(
		FVoxelHeightSculptSave& Save,
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		bool bCompress = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static FVoxelFuture LoadFromSave(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		FVoxelHeightSculptSave Save);

private:
	static FVoxelFuture ApplyModifier(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const TSharedRef<FVoxelHeightModifier>& Modifier);

public:
	/**
	 * Paint a surface type on a sculpt stamp
	 * @param SculptActor			The sculpt actor to paint
	 * @param Center				Center, in world space
	 * @param Radius				Radius, in world space
	 * @param Strength				Strength, usually between 0 and 1
	 * @param Mode					Add or remove surface type
	 * @param SurfaceTypeToPaint	Surface type to paint
	 * @param MetadatasToPaint		Metadatas to paint
	 * @param Brush					Optional, used to apply a pattern to the paint
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static FVoxelFuture PaintSurface(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
		const float Radius = 500.f,
		const float Strength = 0.05f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		UVoxelSurfaceTypeInterface* SurfaceTypeToPaint = nullptr,
		const FVoxelMetadataOverrides MetadatasToPaint = FVoxelMetadataOverrides(),
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		VOXEL_FUNCTION_COUNTER();

		const TSharedRef<FVoxelPaintHeightModifier> Modifier = MakeShared<FVoxelPaintHeightModifier>();
		Modifier->Center = Center;
		Modifier->SurfaceTypeToPaint = SurfaceTypeToPaint;
		Modifier->MetadatasToPaint = MetadatasToPaint;
		Modifier->Radius = Radius;
		Modifier->Strength = Strength;
		Modifier->Mode = Mode;
		Modifier->Brush = Brush;
		return ApplyModifier(SculptActor, Modifier);
	}

public:
	/**
	 * Flatten a voxel surface horizontally
	 * @param SculptActor	The sculpt actor to sculpt
	 * @param Center		Center, in world space
	 * @param Radius		Radius, in world space
	 * @param Falloff		Between 0 and 1
	 * @param Type			Whether to add below, remove above or do both
	 * @param TargetHeight	The height to flatten towards
	 * @param Brush			Optional, used to apply a pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static FVoxelFuture Flatten(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
		const float Radius = 500.f,
		const float Falloff = 0.1f,
		const EVoxelLevelToolType Type = EVoxelLevelToolType::Additive,
		const float TargetHeight = 0.f,
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		const TSharedRef<FVoxelFlattenHeightModifier> Modifier = MakeShared<FVoxelFlattenHeightModifier>();
		Modifier->Center = Center;
		Modifier->Radius = Radius;
		Modifier->Falloff = Falloff;
		Modifier->Type = Type;
		Modifier->TargetHeight = TargetHeight;
		Modifier->Brush = Brush;
		return ApplyModifier(SculptActor, Modifier);
	}

public:
	/**
	 * Smooth a voxel sculpt stamp
	 * @param SculptActor	The sculpt actor to sculpt
	 * @param Center		Center, in world space
	 * @param Radius		Radius, in world space
	 * @param Strength		Strength of the smoothing
	 * @param Brush			Optional, used to apply a pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static FVoxelFuture Smooth(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
		const float Radius = 1000.f,
		const float Strength = 1.f,
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		const TSharedRef<FVoxelSmoothHeightModifier> Modifier = MakeShared<FVoxelSmoothHeightModifier>();
		Modifier->Center = Center;
		Modifier->Radius = Radius;
		Modifier->Strength = Strength;
		Modifier->Brush = Brush;
		return ApplyModifier(SculptActor, Modifier);
	}

public:
	/**
	 * Progressively/smoothly sculpt a height sculpt actor
	 * @param SculptActor	The sculpt actor to sculpt
	 * @param Center		Center, in world space
	 * @param Radius		Radius, in world space
	 * @param Strength		Sculpt speed
	 * @param Mode			Whether to add or remove
	 * @param Brush			Optional, used to apply a pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static FVoxelFuture SculptHeight(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D Center,
		const float Radius = 500.f,
		const float Strength = 0.5f,
		const EVoxelSculptMode Mode = EVoxelSculptMode::Add,
		const FVoxelToolBrush Brush = FVoxelToolBrush())
	{
		const TSharedRef<FVoxelSculptHeightModifier> Modifier = MakeShared<FVoxelSculptHeightModifier>();
		Modifier->Center = Center;
		Modifier->Radius = Radius;
		Modifier->Strength = Strength;
		Modifier->Mode = Mode;
		Modifier->Brush = Brush;
		return ApplyModifier(SculptActor, Modifier);
	}

public:
	/**
	 * Apply a sculpt graph
	 * @param SculptActor	The sculpt actor to sculpt
	 * @param Center		Center, in world space
	 * @param Radius		Radius, in world space
	 * @param Graph			Graph
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Sculpt")
	static FVoxelFuture ApplySculptGraph(
		UPARAM(Required) AVoxelSculptHeight* SculptActor,
		const FVector2D& Center,
		const float Radius = 500.f,
		const FVoxelHeightSculptGraphWrapper& Graph = FVoxelHeightSculptGraphWrapper())
	{
		if (!Graph.IsValid())
		{
			VOXEL_MESSAGE(Error, "Invalid sculpt graph");
			return {};
		}

		const TSharedRef<FVoxelGraphHeightModifier> Modifier = MakeShared<FVoxelGraphHeightModifier>();
		Modifier->Center = Center;
		Modifier->Radius = Radius;
		Modifier->Graph = Graph;
		return ApplyModifier(SculptActor, Modifier);
	}

public:
	UFUNCTION(BlueprintPure, Category = "Voxel", meta = (NativeMakeFunc))
	static FVoxelHeightSculptGraphWrapper MakeVoxelHeightSculptGraphWrapper(UVoxelHeightSculptGraph* Graph)
	{
		FVoxelHeightSculptGraphWrapper Result;
		Result.Graph = Graph;
		return Result;
	}
};