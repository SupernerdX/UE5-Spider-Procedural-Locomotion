// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPCGHelpers.h"
#include "VoxelStackLayer.h"
#include "VoxelMetadataRef.h"
#include "Scatter/VoxelScatterUtilities.h"
#include "PCGVoxelSamplerV2.generated.h"

class FVoxelLayers;
class UPCGPointData;
class FVoxelSurfaceTypeTable;

// Sample points on a voxel world
UCLASS(DisplayName = "Voxel Sampler Experimental")
class VOXELPCG_API UPCGVoxelSamplerV2Settings : public UVoxelPCGSettings
{
	GENERATED_BODY()

public:
	//~ Begin UPCGSettings interface
#if WITH_EDITOR
	virtual EPCGSettingsType GetType() const override
	{
		return EPCGSettingsType::Sampler;
	}
	virtual EPCGChangeType GetChangeTypeForProperty(const FName& InPropertyName) const override
	{
		return Super::GetChangeTypeForProperty(InPropertyName) | EPCGChangeType::Cosmetic;
	}
#endif

	virtual bool UseSeed() const override
	{
		return true;
	}

	virtual FString GetAdditionalTitleInformation() const override;
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	//~ End UPCGSettings interface

	//~ Begin UVoxelPCGSettings Interface
	virtual TSharedPtr<FVoxelPCGOutput> CreateOutput(FPCGContext& Context) const override;
	virtual FString GetNodeDebugInfo() const override;
	//~ End UVoxelPCGSettings Interface

public:
	// If no Bounding Shape input is provided, the actor bounds are used to limit the sample generation domain.
	// This option allows ignoring the actor bounds and generating over the entire surface. Use with caution as this
	// may generate a lot of points.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bUnbounded = false;

	// Layer to sample
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	FVoxelStackLayer Layer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	float DistanceBetweenPoints = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	EVoxelHeightScatterType HeightScatterType = EVoxelHeightScatterType::Grid;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(ClampMin="0", PCG_Overridable))
	float Looseness = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TArray<TObjectPtr<UVoxelMetadata>> MetadatasToQuery;

	// The LOD to sample
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, ClampMin = 0))
	int32 LOD = 0;

	// If false smart surface types won't be resolved
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable))
	bool bResolveSmartSurfaceTypes = true;

	// Increasing this will cause cells with little 'surface' inside them to be ignored. Can help prevent visible lines in point density.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, ClampMin = 0))
	float MinCellArea = 0.03f;
};

class FVoxelSamplerV2PCGOutput : public FVoxelPCGOutput
{
public:
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const FVoxelBox Bounds;

	const FVoxelWeakStackLayer WeakLayer;
	const int32 LOD;
	const bool bResolveSmartSurfaceTypes;
	const float DistanceBetweenPoints;
	const float Looseness;
	const EVoxelHeightScatterType HeightScatterType;
	const float MinCellArea;
	const TVoxelObjectPtr<UPCGPointData> WeakPointData;
	const TVoxelArray<FVoxelMetadataRef> MetadatasToQuery;

	FVoxelSamplerV2PCGOutput(
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const FVoxelBox& Bounds,
		const FVoxelWeakStackLayer& WeakLayer,
		const int32 LOD,
		const bool bResolveSmartSurfaceTypes,
		const float DistanceBetweenPoints,
		const float Looseness,
		const EVoxelHeightScatterType HeightScatterType,
		const float MinTriangleSize,
		const TVoxelObjectPtr<UPCGPointData>& WeakPointData,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery)
		: Layers(Layers)
		, SurfaceTypeTable(SurfaceTypeTable)
		, Bounds(Bounds)
		, WeakLayer(WeakLayer)
		, LOD(LOD)
		, bResolveSmartSurfaceTypes(bResolveSmartSurfaceTypes)
		, DistanceBetweenPoints(DistanceBetweenPoints)
		, Looseness(Looseness)
		, HeightScatterType(HeightScatterType)
		, MinCellArea(MinTriangleSize)
		, WeakPointData(WeakPointData)
		, MetadatasToQuery(MetadatasToQuery)
	{
	}

public:
	//~ Begin FVoxelPCGOutput Interface
	virtual FVoxelFuture Run() const override;
	//~ End FVoxelPCGOutput Interface

private:
	FVoxelFuture GeneratePoints(bool bHeight) const;
};