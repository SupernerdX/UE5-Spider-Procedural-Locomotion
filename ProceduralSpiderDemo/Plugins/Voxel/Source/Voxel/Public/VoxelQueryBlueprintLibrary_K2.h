// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLatentAction.h"
#include "VoxelQueryBlueprintLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelQueryBlueprintLibrary_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelQueryBlueprintLibrary_BlueprintOnly : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Will query data in ZoneToQuery and write it into RenderTarget
	 * @param RenderTarget Render target to write into
	 * @param Layer Layer to query
	 * @param ZoneToQuery The zone, in world space, to query
	 * @param Query The data to write in the render target
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Query", meta = (Layer = "(Stack=\"/Script/Voxel.VoxelLayerStack'/Voxel/Default/DefaultStack.DefaultStack'\",Layer=\"/Script/Voxel.VoxelHeightLayer'/Voxel/Default/DefaultHeightLayer.DefaultHeightLayer'\")", WorldContext = "WorldContextObject"))
	static void ExportVoxelDataToRenderTarget(
		bool& bSuccess,
		UObject* WorldContextObject,
		UTextureRenderTarget2D* RenderTarget,
		FVoxelStackHeightLayer Layer,
		FVoxelBox2D ZoneToQuery,
		FVoxelColorQuery Query)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelQueryBlueprintLibrary::K2_ExportVoxelDataToRenderTarget(
				bSuccess,
				GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull),
				RenderTarget,
				Layer,
				ZoneToQuery,
				Query);
		});
	}

	/**
	 * Will query data in ZoneToQuery and write it into RenderTarget
	 * @param RenderTarget Render target to write into
	 * @param Layer Layer to query
	 * @param ZoneToQuery The zone, in world space, to query
	 * @param Query The data to write in the render target
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Query", meta = (Layer = "(Stack=\"/Script/Voxel.VoxelLayerStack'/Voxel/Default/DefaultStack.DefaultStack'\",Layer=\"/Script/Voxel.VoxelHeightLayer'/Voxel/Default/DefaultHeightLayer.DefaultHeightLayer'\")", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending"))
	static void ExportVoxelDataToRenderTargetAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		bool& bSuccess,
		UTextureRenderTarget2D* RenderTarget,
		FVoxelStackHeightLayer Layer,
		FVoxelBox2D ZoneToQuery,
		FVoxelColorQuery Query,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelQueryBlueprintLibrary::K2_ExportVoxelDataToRenderTarget(
					bSuccess,
					GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull),
					RenderTarget,
					Layer,
					ZoneToQuery,
					Query);
			});
	}

	/**
	 * Query a voxel layer
	 * @param Results The query results
	 * @param Layer The layer to query
	 * @param Positions The positions to query
	 * @param MetadatasToQuery The metadatas to query
	 * @param GradientStep Used to compute normals, which are passed to smart surface types
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "GradientStep", AutoCreateRefTerm = "MetadatasToQuery, Results, Positions, MetadatasToQuery", WorldContext = "WorldContextObject"))
	static void MultiQueryVoxelLayer(
		bool& bSuccess,
		TArray<FVoxelQueryResult>& Results,
		UObject* WorldContextObject,
		FVoxelStackLayer Layer,
		TArray<FVector> Positions,
		TArray<UVoxelMetadata*> MetadatasToQuery,
		float GradientStep = 100.0f)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelQueryBlueprintLibrary::K2_MultiQueryVoxelLayer(
				bSuccess,
				Results,
				GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull),
				Layer,
				Positions,
				MetadatasToQuery,
				GradientStep);
		});
	}

	/**
	 * Query a voxel layer
	 * @param Results The query results
	 * @param Layer The layer to query
	 * @param Positions The positions to query
	 * @param MetadatasToQuery The metadatas to query
	 * @param GradientStep Used to compute normals, which are passed to smart surface types
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "GradientStep, bExecuteIfAlreadyPending", AutoCreateRefTerm = "MetadatasToQuery, Results, Positions, MetadatasToQuery", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void MultiQueryVoxelLayerAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		bool& bSuccess,
		TArray<FVoxelQueryResult>& Results,
		FVoxelStackLayer Layer,
		TArray<FVector> Positions,
		TArray<UVoxelMetadata*> MetadatasToQuery,
		float GradientStep = 100.0f,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelQueryBlueprintLibrary::K2_MultiQueryVoxelLayer(
					bSuccess,
					Results,
					GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull),
					Layer,
					Positions,
					MetadatasToQuery,
					GradientStep);
			});
	}

	/**
	 * Query a voxel layer
	 * @param Result The query results
	 * @param Layer The layer to query
	 * @param Position The position to query
	 * @param MetadatasToQuery The metadatas to query
	 * @param GradientStep Used to compute normals, which are passed to smart surface types
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "GradientStep", AutoCreateRefTerm = "MetadatasToQuery, MetadatasToQuery", WorldContext = "WorldContextObject"))
	static void QueryVoxelLayer(
		bool& bSuccess,
		FVoxelQueryResult& Result,
		UObject* WorldContextObject,
		FVoxelStackLayer Layer,
		FVector Position,
		TArray<UVoxelMetadata*> MetadatasToQuery,
		float GradientStep = 100.0f)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelQueryBlueprintLibrary::K2_QueryVoxelLayer(
				bSuccess,
				Result,
				GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull),
				Layer,
				Position,
				MetadatasToQuery,
				GradientStep);
		});
	}

	/**
	 * Query a voxel layer
	 * @param Result The query results
	 * @param Layer The layer to query
	 * @param Position The position to query
	 * @param MetadatasToQuery The metadatas to query
	 * @param GradientStep Used to compute normals, which are passed to smart surface types
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "GradientStep, bExecuteIfAlreadyPending", AutoCreateRefTerm = "MetadatasToQuery, MetadatasToQuery", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void QueryVoxelLayerAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		bool& bSuccess,
		FVoxelQueryResult& Result,
		FVoxelStackLayer Layer,
		FVector Position,
		TArray<UVoxelMetadata*> MetadatasToQuery,
		float GradientStep = 100.0f,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelQueryBlueprintLibrary::K2_QueryVoxelLayer(
					bSuccess,
					Result,
					GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull),
					Layer,
					Position,
					MetadatasToQuery,
					GradientStep);
			});
	}

	/**
	 * Queries the list of stamps in specified layer at position
	 * @param Results The query results
	 * @param Layer The layer to query
	 * @param Position The position to query
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AutoCreateRefTerm = "Results", WorldContext = "WorldContextObject"))
	static void QueryVoxelStamps(
		bool& bSuccess,
		TArray<FVoxelQueryStampResult>& Results,
		UObject* WorldContextObject,
		const FVoxelStackLayer& Layer,
		const FVector& Position)
	{
		Voxel::ExecuteSynchronously([&]
		{
			return UVoxelQueryBlueprintLibrary::K2_QueryVoxelStamps(
				bSuccess,
				Results,
				GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull),
				Layer,
				Position);
		});
	}

	/**
	 * Queries the list of stamps in specified layer at position
	 * @param Results The query results
	 * @param Layer The layer to query
	 * @param Position The position to query
	 * @param bExecuteIfAlreadyPending If true, this node will execute even if the last call has not yet completed. Be careful when using this on tick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bExecuteIfAlreadyPending", AutoCreateRefTerm = "Results"))
	static void QueryVoxelStampsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		bool& bSuccess,
		TArray<FVoxelQueryStampResult>& Results,
		const FVoxelStackLayer& Layer,
		const FVector& Position,
		bool bExecuteIfAlreadyPending = false)
	{
		FVoxelLatentAction::Execute(
			WorldContextObject,
			LatentInfo,
			bExecuteIfAlreadyPending,
			[&]
			{
				return UVoxelQueryBlueprintLibrary::K2_QueryVoxelStamps(
					bSuccess,
					Results,
					GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull),
					Layer,
					Position);
			});
	}
};