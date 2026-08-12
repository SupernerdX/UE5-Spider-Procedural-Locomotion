// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPCGHelpers.h"
#include "VoxelStackLayer.h"
#include "VoxelMetadataRef.h"
#include "PCGVoxelProjection.generated.h"

class UPCGPointData;
class FVoxelLayers;
class FVoxelSurfaceTypeTable;

// Project points onto a voxel world
UCLASS(DisplayName = "Voxel Projection")
class VOXELPCG_API UPCGVoxelProjectionSettings : public UVoxelPCGSettings
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
	virtual void Serialize(FArchive& Ar) override;
	//~ End UVoxelPCGSettings Interface

public:
	// Layer to sample
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	FVoxelStackLayer Layer;

	// If a point is further away than this distance from the surface
	// at the end of the raymarching, its density will be set to 0
	// Ignored if sampling height layer
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = 0, Units = cm))
	float KillDistance = 1000;

	// If true, will set the new point rotation to match the distance field gradient at the new position
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bUpdateRotation = false;

	// If true, will look for a new surface in the point downward directly, according to its rotation
	// Ignored if sampling height layer
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bForceDirection = true;

	// The LOD to sample
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, ClampMin = 0))
	int32 LOD = 0;

	// Max number of steps to do during raymarching
	// Ignored if sampling height layer
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, ClampMin = 1))
	int32 MaxSteps = 10;

	// If distance to surface is less than this, stop the raymarching
	// Ignored if sampling height layer
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, Units = cm))
	float Tolerance = 10;

	// How "fast" to converge to the surface, between 0 and 1
	// NewPoint = OldPoint + DistanceToSurface * Direction * Speed
	// Decrease if the raymarching is imprecise
	// Ignored if sampling height layer
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, ClampMin = 0, ClampMax = 1))
	float Speed = 0.8f;

	// Distance between points when sampling gradients
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, ClampMin = 0, ClampMax = 1, Units = cm))
	float GradientStep = 100.f;

	// If true, will draw debug points for every step taken
	// Very expensive if used on too many points!
	// Ignored if sampling height layer
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable))
	bool bDebugSteps = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bQuerySurfaceTypes = false;

	UPROPERTY()
	TArray<FName> MetadatasToQuery;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TArray<TObjectPtr<UVoxelMetadata>> NewMetadatasToQuery;

	// If false smart surface types won't be resolved
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable))
	bool bResolveSmartSurfaceTypes = true;
};

class FVoxelProjectionPCGOutput : public FVoxelPCGOutput
{
public:
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const FVoxelWeakStackLayer WeakLayer;
	const int32 LOD;
	const float KillDistance;
	const bool bUpdateRotation;
	const bool bForceDirection;
	const int32 MaxSteps;
	const float Tolerance;
	const float Speed;
	const float GradientStep;
	const bool bDebugSteps;
	const bool bQuerySurfaceTypes;
	const bool bResolveSmartSurfaceTypes;
	const TVoxelArray<FVoxelMetadataRef> MetadatasToQuery;

	TVoxelMap<TVoxelObjectPtr<const UPCGPointData>, TVoxelObjectPtr<UPCGPointData>> SourceToResult;

	FVoxelProjectionPCGOutput(
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const FVoxelWeakStackLayer& WeakLayer,
		const int32 LOD,
		const float KillDistance,
		const bool bUpdateRotation,
		const bool bForceDirection,
		const int32 MaxSteps,
		const float Tolerance,
		const float Speed,
		const float GradientStep,
		const bool bDebugSteps,
		const bool bQuerySurfaceTypes,
		const bool bResolveSmartSurfaceTypes,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery)
		: Layers(Layers)
		, SurfaceTypeTable(SurfaceTypeTable)
		, WeakLayer(WeakLayer)
		, LOD(LOD)
		, KillDistance(KillDistance)
		, bUpdateRotation(bUpdateRotation)
		, bForceDirection(bForceDirection)
		, MaxSteps(MaxSteps)
		, Tolerance(Tolerance)
		, Speed(Speed)
		, GradientStep(GradientStep)
		, bDebugSteps(bDebugSteps)
		, bQuerySurfaceTypes(bQuerySurfaceTypes)
		, bResolveSmartSurfaceTypes(bResolveSmartSurfaceTypes)
		, MetadatasToQuery(MetadatasToQuery)
	{
	}

	//~ Begin FVoxelPCGOutput Interface
	virtual FVoxelFuture Run() const override;
	//~ End FVoxelPCGOutput Interface

private:
	FVoxelFuture Project2D(
		TVoxelArray<FPCGPoint> Points,
		const TVoxelObjectPtr<UPCGPointData>& WeakOutPointData) const;

	FVoxelFuture Project3D(
		TVoxelArray<FPCGPoint> Points,
		const TVoxelObjectPtr<UPCGPointData>& WeakOutPointData) const;
};