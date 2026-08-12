// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLayer.h"
#include "VoxelStamp.h"
#include "VoxelStampQuery.h"
#include "VoxelStampRuntime.h"
#include "VoxelStampTransform.h"
#include "VoxelHeightBlendMode.h"
#include "VoxelHeightStamp.generated.h"

class FVoxelQuery;
class FVoxelMetadataView;

USTRUCT(meta = (Abstract))
struct VOXEL_API FVoxelHeightStamp : public FVoxelStamp
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	// Layer that this stamps belong to
	// You can control the order of layers in Layer Stacks
	// You can select the layer stack to use in your Voxel World or PCG Sampler settings
	UPROPERTY(EditAnywhere, Category = "Config", meta = (EditCondition = "!bDisableEditingLayers", HideEditConditionToggle))
	TObjectPtr<UVoxelHeightLayer> Layer;

	UPROPERTY()
	TArray<TObjectPtr<UVoxelHeightLayer>> VoxelLayers;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (EditCondition = "!bDisableEditingBlendMode", HideEditConditionToggle))
	EVoxelHeightBlendMode BlendMode = EVoxelHeightBlendMode::Max;

	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay, meta = (EditCondition = "!bDisableEditingLayers", HideEditConditionToggle))
	TArray<TObjectPtr<UVoxelHeightLayer>> AdditionalLayers;

	// By how much to extend the bounds, relative to the bounds size
	// Increase this if you are using a high smoothness
	// Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay, meta = (UIMin = 0, UIMax = 5))
	float HeightPaddingMultiplier = 0.1f;

public:
	UPROPERTY(VisibleAnywhere, Category = "Config", SkipSerialization, meta = (NoK2, NoCopyEditor))
	bool bDisableEditingLayers = false;

	UPROPERTY(VisibleAnywhere, Category = "Config", SkipSerialization, meta = (NoK2, NoCopyEditor))
	bool bDisableEditingBlendMode = false;

public:
	FVoxelHeightStamp();

	//~ Begin FVoxelStamp Interface
	virtual void FixupProperties() override;
	virtual void PostSerialize(const FArchive& Ar) override;
#if WITH_EDITOR
	virtual void GetPropertyInfo(FPropertyInfo& Info) const override;
#endif
	//~ End FVoxelStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelHeightStampRuntime : public FVoxelStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_ABSTRACT_BODY(FVoxelHeightStamp)

public:
	FORCEINLINE EVoxelHeightBlendMode GetBlendMode() const
	{
		return GetStamp().BlendMode;
	}
	FORCEINLINE float GetSmoothness() const
	{
		return GetStamp().Smoothness;
	}

public:
	virtual bool HasRelativeHeightRange() const
	{
		return false;
	}

public:
	virtual void CollectDependencies(
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelHeightTransform& StampToQuery,
		const FVoxelBox2D& Bounds) const
	{
	}

	virtual void Apply(
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& StampToQuery) const;

	virtual void Apply(
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery) const
	{
	}
};