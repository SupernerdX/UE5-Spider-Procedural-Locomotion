// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPCGHelpers.h"
#include "VoxelStackLayer.h"
#include "VoxelMetadataRef.h"
#include "PCGVoxelQuery.generated.h"

class UPCGPointData;
class FVoxelLayers;
class FVoxelSurfaceTypeTable;

// Queries voxel world data from points locations
UCLASS(DisplayName = "Voxel Query")
class VOXELPCG_API UPCGVoxelQuerySettings : public UVoxelPCGSettings
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

	// New attribute name for height/distance output
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	FString HeightOrDistanceAttribute = "QueryValue";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bQuerySurfaceTypes = false;

	// Suffix for surface attributes
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, EditCondition = "bQuerySurfaceTypes"))
	FString SurfaceAttributeSuffix = "_Query";

	UPROPERTY()
	TArray<FName> MetadatasToQuery;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TArray<TObjectPtr<UVoxelMetadata>> NewMetadatasToQuery;

	// Suffix for metadata attributes
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	FString MetadataAttributeSuffix = "_Query";

	// The LOD to sample
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable, ClampMin = 0))
	int32 LOD = 0;

	// If false smart surface types won't be resolved
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", AdvancedDisplay, meta = (PCG_Overridable))
	bool bResolveSmartSurfaceTypes = true;
};

class FVoxelQueryPCGOutput : public FVoxelPCGOutput
{
public:
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const FVoxelWeakStackLayer WeakLayer;
	const int32 LOD;
	const FString HeightOrDistanceAttribute;
	const bool bQuerySurfaceTypes;
	const bool bResolveSmartSurfaceTypes;
	const FString SurfaceAttributeSuffix;
	const TVoxelArray<FVoxelMetadataRef> MetadatasToQuery;
	const FString MetadataAttributeSuffix;

	TVoxelMap<TVoxelObjectPtr<const UPCGPointData>, TVoxelObjectPtr<UPCGPointData>> SourceToResult;

	FVoxelQueryPCGOutput(
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const FVoxelWeakStackLayer& WeakLayer,
		const int32 LOD,
		const FString& HeightOrDistanceAttribute,
		const bool bQuerySurfaceTypes,
		const bool bResolveSmartSurfaceTypes,
		const FString& SurfaceAttributeSuffix,
		const TVoxelArray<FVoxelMetadataRef>& MetadatasToQuery,
		const FString& MetadataAttributeSuffix)
		: Layers(Layers)
		, SurfaceTypeTable(SurfaceTypeTable)
		, WeakLayer(WeakLayer)
		, LOD(LOD)
		, HeightOrDistanceAttribute(HeightOrDistanceAttribute)
		, bQuerySurfaceTypes(bQuerySurfaceTypes)
		, bResolveSmartSurfaceTypes(bResolveSmartSurfaceTypes)
		, SurfaceAttributeSuffix(SurfaceAttributeSuffix)
		, MetadatasToQuery(MetadatasToQuery)
		, MetadataAttributeSuffix(MetadataAttributeSuffix)
	{
	}

	//~ Begin FVoxelPCGOutput Interface
	virtual FVoxelFuture Run() const override;
	//~ End FVoxelPCGOutput Interface

private:
	FVoxelFuture Query(
		TVoxelArray<FPCGPoint> Points,
		const TVoxelObjectPtr<UPCGPointData>& WeakOutPointData) const;
};