// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelHeightStamp.h"
#include "VoxelFloatMetadataRef.h"
#include "VoxelHeightmapUtilities.h"
#include "Surface/VoxelSurfaceType.h"
#include "VoxelHeightmapStamp.generated.h"

class UVoxelHeightmap;
class UVoxelSurfaceTypeInterface;
struct FVoxelHeightmap_HeightData;
struct FVoxelHeightmap_WeightData;
enum class EVoxelHeightmapWeightType;

USTRUCT(BlueprintType)
struct FVoxelHeightmapStampWeightmapSurfaceType
{
	GENERATED_BODY()

	// Index of the weightmap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (ClampMin = 0))
	int32 Index = 0;

	UPROPERTY()
	TObjectPtr<UObject> Material;

	// The new surface to apply
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TObjectPtr<UVoxelSurfaceTypeInterface> SurfaceType;
};

USTRUCT(meta = (ShortName = "Heightmap", Icon = "LandscapeEditor.Target_Heightmap", SortOrder = 0))
struct VOXEL_API FVoxelHeightmapStamp final : public FVoxelHeightStamp
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelHeightmap> Heightmap;

	UPROPERTY()
	TObjectPtr<UObject> DefaultMaterial;

	// Default surface used as base when no weightmap is applied
	UPROPERTY(EditAnywhere, Category = "Config")
	TObjectPtr<UVoxelSurfaceTypeInterface> DefaultSurfaceType;

	// Use this to override weightmap surfaces
	UPROPERTY(EditAnywhere, Category = "Config", meta = (NoAutoGen))
	TArray<FVoxelHeightmapStampWeightmapSurfaceType> WeightmapSurfaceTypes;

public:
	//~ Begin FVoxelHeightStamp Interface
	virtual UObject* GetAsset() const override;
	virtual void FixupProperties() override;
	//~ End FVoxelHeightStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelHeightmapStampRuntime : public FVoxelHeightStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_BODY(FVoxelHeightmapStamp)

public:
	float ScaleXY = 0;
	FVoxelSurfaceType DefaultSurfaceType;
	float ScaleZ = 0;
	float OffsetZ = 0;
	bool bUseBicubic = true;

	TSharedPtr<const FVoxelHeightmap_HeightData> HeightData;
	TSharedPtr<FVoxelRuntimeMetadataOverrides> MetadataOverrides;
	TVoxelArray<FVoxelHeightmapUtilities::FWeightmap> Weightmaps;

public:
	//~ Begin FVoxelHeightStampRuntime Interface
	virtual bool Initialize(FVoxelDependencyCollector& DependencyCollector) override;
	virtual FVoxelBox GetLocalBounds() const override;

	virtual void Apply(
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& StampToQuery) const override;

	virtual void Apply(
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery) const override;
	//~ End FVoxelHeightStampRuntime Interface
};