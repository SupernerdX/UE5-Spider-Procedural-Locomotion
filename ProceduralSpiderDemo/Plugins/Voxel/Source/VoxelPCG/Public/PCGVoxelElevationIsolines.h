// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPCGHelpers.h"
#include "VoxelStackLayer.h"
#include "PCGVoxelElevationIsolines.generated.h"

class FVoxelLayers;
class UPCGPointData;
class UVoxelMetadata;
class FVoxelSurfaceTypeTable;

// Compute the elevation isolines of a surface, can output either points or splines.
// Currently only work with Height Layers
UCLASS(DisplayName = "Voxel Elevation Isolines")
class VOXELPCG_API UPCGVoxelElevationIsolines : public UVoxelPCGSettings
{
	GENERATED_BODY()

public:
	//~ Begin UPCGSettings interface
#if WITH_EDITOR
	virtual EPCGSettingsType GetType() const override
	{
		return EPCGSettingsType::Spatial;
	}
	virtual EPCGChangeType GetChangeTypeForProperty(const FName& InPropertyName) const override
	{
		return Super::GetChangeTypeForProperty(InPropertyName) | EPCGChangeType::Cosmetic;
	}
#endif

	virtual FString GetAdditionalTitleInformation() const override;
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	//~ End UPCGSettings interface

	//~ Begin UVoxelPCGSettings Interface
	virtual TSharedPtr<FVoxelPCGOutput> CreateOutput(FPCGContext& Context) const override;
	virtual FString GetNodeDebugInfo() const override;
	//~ End UVoxelPCGSettings Interface

public:
	// Layer to sample
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	FVoxelStackHeightLayer Layer;

	// The LOD to sample
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, ClampMin = 0))
	int32 LOD = 0;

	// Minimum elevation of the isolines.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Settings, meta = (PCG_Overridable))
	float ElevationStart = 0.0;

	// Maximum elevation of the isolines.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float ElevationEnd = 1000.0;

	// Increment elevation between each isolines.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float ElevationIncrement = 100.0;

	// Resolution of the grid for the discretization of the surface. This is the size of one cell, in cm.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	float Resolution = 100.0;

	// Can add a tag (integer) to group output data that are at the same elevation.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	bool bAddTagOnOutputForSameElevation = false;

	// Option to either have Z up or project the surface normal at this position (similar to project rotations on the projection node).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable))
	bool bProjectSurfaceNormal = false;

	// Will output splines rather than points.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	bool bOutputAsSpline = false;

	// Spline can either be curved or linear.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable, EditCondition = "bOutputAsSpline", EditConditionHides))
	bool bLinearSpline = false;
};

class FVoxelElevationIsolinesPCGOutput : public FVoxelPCGOutput
{
public:
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const FVoxelBox Bounds;

	const FVoxelWeakStackLayer WeakLayer;
	const int32 LOD;

	const float ElevationStart;
	const float ElevationEnd;
	const float ElevationIncrement;
	const float Resolution;
	const bool bAddTagOnOutputForSameElevation;
	const bool bProjectSurfaceNormal;
	const bool bOutputAsSpline;
	const bool bLinearSpline;

	FVoxelElevationIsolinesPCGOutput(
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const FVoxelBox& Bounds,
		const FVoxelWeakStackLayer& WeakLayer,
		const int32 LOD,
		const float ElevationStart,
		const float ElevationEnd,
		const float ElevationIncrement,
		const float Resolution,
		const bool bAddTagOnOutputForSameElevation,
		const bool bProjectSurfaceNormal,
		const bool bOutputAsSpline,
		const bool bLinearSpline)
		: Layers(Layers)
		, SurfaceTypeTable(SurfaceTypeTable)
		, Bounds(Bounds)
		, WeakLayer(WeakLayer)
		, LOD(LOD)
		, ElevationStart(ElevationStart)
		, ElevationEnd(ElevationEnd)
		, ElevationIncrement(ElevationIncrement)
		, Resolution(Resolution)
		, bAddTagOnOutputForSameElevation(bAddTagOnOutputForSameElevation)
		, bProjectSurfaceNormal(bProjectSurfaceNormal)
		, bOutputAsSpline(bOutputAsSpline)
		, bLinearSpline(bLinearSpline)
	{}

public:
	//~ Begin FVoxelPCGOutput Interface
	virtual FVoxelFuture Run() const override;
	//~ End FVoxelPCGOutput Interface

private:
	FVoxelFuture Generate2D() const;
	FVoxelFuture Generate3D() const;
};