// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelLinearColorMetadata.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelQueryBlueprintLibrary.generated.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;
class UTextureRenderTarget2D;
class UVoxelSurfaceTypeInterface;
enum ETextureRenderTargetFormat : int;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum class EVoxelFloatQueryType : uint8
{
	// Write a constant value in the channel
	Constant,
	// Write the height in the channel
	Height,
	// Write a metadata in the channel
	Metadata
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelFloatQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelFloatQueryType Type = EVoxelFloatQueryType::Constant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (EditCondition = "Type == EVoxelFloatQueryType::Constant"))
	float Constant = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (EditCondition = "Type == EVoxelFloatQueryType::Metadata"))
	TObjectPtr<UVoxelMetadata> Metadata;

	// Component to extract from Metadata if it's a vector or a color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (EditCondition = "Type == EVoxelFloatQueryType::Metadata"))
	EVoxelTextureChannel ComponentToExtract = EVoxelTextureChannel::R;

private:
	mutable FVoxelMetadataRef MetadataRef;

	friend class UVoxelQueryBlueprintLibrary;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelColorQuery
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelFloatQuery R;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelFloatQuery G;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelFloatQuery B;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelFloatQuery A;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelQueryResult
{
	GENERATED_BODY()

public:
	// Height or distance based on layer type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float Value = 0.f;

	// The normal at that location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector Normal = FVector(ForceInit);

	// Surface type before smart surface types are resolved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelSurfaceTypeInterface> UnresolvedSurfaceType;

	// The final surface type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UVoxelSurfaceTypeInterface> SurfaceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TMap<TObjectPtr<UVoxelMetadata>, FVoxelPinValue> MetadataToValue;

private:
	TVoxelObjectPtr<UVoxelSurfaceTypeInterface> WeakUnresolvedSurfaceType;
	TVoxelObjectPtr<UVoxelSurfaceTypeInterface> WeakSurfaceType;
	TVoxelMap<TVoxelObjectPtr<UVoxelMetadata>, FVoxelPinValue> WeakMetadataToValue;

	friend class UVoxelQueryBlueprintLibrary;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelQueryStampResult
{
	GENERATED_BODY()

public:
	// Either a VoxelStampComponent, or a VoxelInstancedStampComponent if the stamp is part of one of those
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<USceneComponent> StampComponent;

	// If the stamp is part of a VoxelInstancedStampComponent, this returns the index of the stamp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 StampIndex = -1;

	// The layer stamp is writing to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelStackLayer Layer;

	// Returns distance, even for height stamps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float DistanceBeforeStamp = 0.f;

	// Returns distance, even for height stamps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float DistanceAfterStamp = 0.f;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelQueryBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TVoxelFuture<bool> ExportVoxelDataToRenderTarget(
		UWorld* World,
		UTextureRenderTarget2D* RenderTarget,
		const FVoxelStackHeightLayer& Layer,
		const FVoxelBox2D& ZoneToQuery,
		const FVoxelColorQuery& Query);

	/**
	 * Will query data in ZoneToQuery and write it into RenderTarget
	 * @param RenderTarget	Render target to write into
	 * @param Layer			Layer to query
	 * @param ZoneToQuery	The zone, in world space, to query
	 * @param Query			The data to write in the render target
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Query", meta = (Layer = "(Stack=\"/Script/Voxel.VoxelLayerStack'/Voxel/Default/DefaultStack.DefaultStack'\",Layer=\"/Script/Voxel.VoxelHeightLayer'/Voxel/Default/DefaultHeightLayer.DefaultHeightLayer'\")"))
	static FVoxelFuture K2_ExportVoxelDataToRenderTarget(
		bool& bSuccess,
		UWorld* World,
		UTextureRenderTarget2D* RenderTarget,
		FVoxelStackHeightLayer Layer,
		FVoxelBox2D ZoneToQuery,
		FVoxelColorQuery Query);

public:
	// Make a constant value to be used in ExportVoxelDataToRenderTarget
	UFUNCTION(BlueprintPure, Category = "Voxel|Query")
	static FVoxelFloatQuery MakeConstant(const float Constant)
	{
		FVoxelFloatQuery Result;
		Result.Type = EVoxelFloatQueryType::Constant;
		Result.Constant = Constant;
		return Result;
	}

	// To be used in ExportVoxelDataToRenderTarget
	UFUNCTION(BlueprintPure, Category = "Voxel|Query")
	static FVoxelFloatQuery QueryHeight()
	{
		FVoxelFloatQuery Result;
		Result.Type = EVoxelFloatQueryType::Height;
		return Result;
	}

	/**
	 * To be used in ExportVoxelDataToRenderTarget
	 * @param Metadata				The metadata
	 * @param ComponentToExtract	Component to extract from Metadata if it's a vector or a color
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Query")
	static FVoxelFloatQuery QueryMetadata(
		UVoxelMetadata* Metadata,
		const EVoxelTextureChannel ComponentToExtract)
	{
		FVoxelFloatQuery Result;
		Result.Type = EVoxelFloatQueryType::Metadata;
		Result.Metadata = Metadata;
		Result.ComponentToExtract = ComponentToExtract;
		return Result;
	}

public:
	UFUNCTION(BlueprintPure, Category = "Voxel|Query")
	static FVoxelColorQuery QueryColorMetadata(UVoxelLinearColorMetadata* Metadata)
	{
		FVoxelColorQuery Result;
		Result.R = QueryMetadata(Metadata, EVoxelTextureChannel::R);
		Result.G = QueryMetadata(Metadata, EVoxelTextureChannel::G);
		Result.B = QueryMetadata(Metadata, EVoxelTextureChannel::B);
		Result.A = QueryMetadata(Metadata, EVoxelTextureChannel::A);
		return Result;
	}

private:
	static TVoxelArray<uint8> CreateRenderTargetData(
		ETextureRenderTargetFormat Format,
		int32 Num,
		const FVoxelFloatBuffer& R,
		const FVoxelFloatBuffer& G,
		const FVoxelFloatBuffer& B,
		const FVoxelFloatBuffer& A);

public:
	static TVoxelFuture<TOptional<FVoxelQueryResult>> QueryVoxelLayer(
		UWorld* World,
		const FVoxelStackLayer& Layer,
		const FVector& Position,
		const TArray<UVoxelMetadata*>& MetadatasToQuery,
		float GradientStep = 100.f);

	static TVoxelFuture<TOptional<TArray<FVoxelQueryResult>>> MultiQueryVoxelLayer(
		UWorld* World,
		const FVoxelStackLayer& Layer,
		const TArray<FVector>& Positions,
		const TArray<UVoxelMetadata*>& MetadatasToQuery,
		float GradientStep = 100.f);

public:
	/**
	 * Query a voxel layer
	 * @param Result				The query results
	 * @param Layer					The layer to query
	 * @param Position				The position to query
	 * @param MetadatasToQuery		The metadatas to query
	 * @param GradientStep			Used to compute normals, which are passed to smart surface types
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AutoCreateRefTerm = "MetadatasToQuery", AdvancedDisplay = "GradientStep"))
	static FVoxelFuture K2_QueryVoxelLayer(
		bool& bSuccess,
		FVoxelQueryResult& Result,
		UWorld* World,
		FVoxelStackLayer Layer,
		FVector Position,
		TArray<UVoxelMetadata*> MetadatasToQuery,
		float GradientStep = 100.f);

	/**
	 * Query a voxel layer
	 * @param Results				The query results
	 * @param Layer					The layer to query
	 * @param Positions				The positions to query
	 * @param MetadatasToQuery		The metadatas to query
	 * @param GradientStep			Used to compute normals, which are passed to smart surface types
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AutoCreateRefTerm = "MetadatasToQuery", AdvancedDisplay = "GradientStep"))
	static FVoxelFuture K2_MultiQueryVoxelLayer(
		bool& bSuccess,
		TArray<FVoxelQueryResult>& Results,
		UWorld* World,
		FVoxelStackLayer Layer,
		TArray<FVector> Positions,
		TArray<UVoxelMetadata*> MetadatasToQuery,
		float GradientStep = 100.f);

public:
	static TVoxelFuture<TOptional<TArray<FVoxelQueryStampResult>>> QueryVoxelStamps(
		UWorld* World,
		const FVoxelStackLayer& Layer,
		const FVector& Position);

	/**
	 * Queries the list of stamps in specified layer at position
	 * @param Results				The query results
	 * @param Layer					The layer to query
	 * @param Position				The position to query
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static FVoxelFuture K2_QueryVoxelStamps(
		bool& bSuccess,
		TArray<FVoxelQueryStampResult>& Results,
		UWorld* World,
		const FVoxelStackLayer& Layer,
		const FVector& Position);
};