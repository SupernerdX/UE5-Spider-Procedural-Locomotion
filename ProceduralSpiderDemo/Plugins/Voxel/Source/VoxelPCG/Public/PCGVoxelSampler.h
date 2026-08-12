// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPCGHelpers.h"
#include "VoxelStackLayer.h"
#include "VoxelMetadataRef.h"
#include "PCGVoxelSampler.generated.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;
class UPCGPointData;

// Sample points on a voxel world
UCLASS(DisplayName = "Voxel Sampler")
class VOXELPCG_API UPCGVoxelSamplerSettings : public UVoxelPCGSettings
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
	float PointsPerSquaredMeter = 0.1f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	float CellSize = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(ClampMin="0", PCG_Overridable))
	float Looseness = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta=(ClampMin="0", PCG_Overridable))
	float Tolerance = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings|Points", meta = (PCG_Overridable))
	bool bApplyDensityToPoints = true;

	UPROPERTY()
	TArray<FName> MetadatasToQuery;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TArray<TObjectPtr<UVoxelMetadata>> NewMetadatasToQuery;

	// The LOD to sample
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, ClampMin = 0))
	int32 LOD = 0;

	// If false smart surface types won't be resolved
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable))
	bool bResolveSmartSurfaceTypes = true;
};

class FVoxelSamplerPCGOutput : public FVoxelPCGOutput
{
public:
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const FVoxelBox Bounds;

	const FVoxelWeakStackLayer WeakLayer;
	const int32 LOD;
	const bool bResolveSmartSurfaceTypes;
	const float PointsPerSquaredMeter;
	const float CellSize;
	const float Looseness;
	const float Tolerance;
	const bool bApplyDensityToPoints;
	const TVoxelObjectPtr<UPCGPointData> WeakPointData;
	const TVoxelArray<FVoxelMetadataRef> MetadatasToQuery;

	FVoxelSamplerPCGOutput(
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const FVoxelBox& Bounds,
		const FVoxelWeakStackLayer& WeakLayer,
		const int32 LOD,
		const bool bResolveSmartSurfaceTypes,
		const float PointsPerSquaredMeter,
		const float CellSize,
		const float Looseness,
		const float Tolerance,
		const bool bApplyDensityToPoints,
		const TVoxelObjectPtr<UPCGPointData>& WeakPointData,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery)
		: Layers(Layers)
		, SurfaceTypeTable(SurfaceTypeTable)
		, Bounds(Bounds)
		, WeakLayer(WeakLayer)
		, LOD(LOD)
		, bResolveSmartSurfaceTypes(bResolveSmartSurfaceTypes)
		, PointsPerSquaredMeter(PointsPerSquaredMeter)
		, CellSize(CellSize)
		, Looseness(Looseness)
		, Tolerance(Tolerance)
		, bApplyDensityToPoints(bApplyDensityToPoints)
		, WeakPointData(WeakPointData)
		, MetadatasToQuery(MetadatasToQuery)
	{
	}

public:
	//~ Begin FVoxelPCGOutput Interface
	virtual FVoxelFuture Run() const override;
	//~ End FVoxelPCGOutput Interface

private:
	FVoxelFuture Generate2D() const;
	FVoxelFuture Generate3D() const;
};