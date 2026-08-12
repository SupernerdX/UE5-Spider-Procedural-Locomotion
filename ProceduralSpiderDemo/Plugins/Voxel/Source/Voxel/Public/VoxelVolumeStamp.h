// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLayer.h"
#include "VoxelStamp.h"
#include "VoxelStampQuery.h"
#include "VoxelStampRuntime.h"
#include "VoxelStampTransform.h"
#include "VoxelVolumeBlendMode.h"
#include "VoxelVolumeStamp.generated.h"

class FVoxelQuery;
class FVoxelMetadataView;

USTRUCT(meta = (Abstract))
struct VOXEL_API FVoxelVolumeStamp : public FVoxelStamp
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	// Layer that this stamps belong to
	// You can control the order of layers in Layer Stacks
	// You can select the layer stack to use in your Voxel World or PCG Sampler settings
	UPROPERTY(EditAnywhere, Category = "Config", meta = (EditCondition = "!bDisableEditingLayers", HideEditConditionToggle))
	TObjectPtr<UVoxelVolumeLayer> Layer;

	UPROPERTY()
	TArray<TObjectPtr<UVoxelVolumeLayer>> VoxelLayers;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (EditCondition = "!bDisableEditingBlendMode", HideEditConditionToggle))
	EVoxelVolumeBlendMode BlendMode = EVoxelVolumeBlendMode::Additive;

	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay, meta = (EditCondition = "!bDisableEditingLayers", HideEditConditionToggle))
	TArray<TObjectPtr<UVoxelVolumeLayer>> AdditionalLayers;

	// By how much to extend the bounds, relative to the bounds size
	// Increase this if you are using a high smoothness
	// Increasing this will lead to more stamps being sampled per voxel, increasing generation cost
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay, meta = (UIMin = 0, UIMax = 5))
	float BoundsExtensionMultiplier = 1.f;

	// Bounds Extension will be limited to this value in cm, regardless of what the multiplier is set to
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay, meta = (ClampMin = 0))
	float MaximumBoundsExtension = 1000.f;

public:
	UPROPERTY(VisibleAnywhere, Category = "Config", SkipSerialization, meta = (NoK2, NoCopyEditor))
	bool bDisableEditingLayers = false;

	UPROPERTY(VisibleAnywhere, Category = "Config", SkipSerialization, meta = (NoK2, NoCopyEditor))
	bool bDisableEditingBlendMode = false;

public:
	FVoxelVolumeStamp();

	//~ Begin FVoxelStamp Interface
	virtual void FixupProperties() override;
	virtual void PostSerialize(const FArchive& Ar) override;
#if WITH_EDITOR
	virtual void GetPropertyInfo(FPropertyInfo& Info) const override;
#endif
	//~ End FVoxelStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelVolumeStampRuntime : public FVoxelStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_ABSTRACT_BODY(FVoxelVolumeStamp)

public:
	FORCEINLINE EVoxelVolumeBlendMode GetBlendMode() const
	{
		return GetStamp().BlendMode;
	}
	FORCEINLINE float GetSmoothness() const
	{
		return GetStamp().Smoothness;
	}

public:
	virtual void CollectDependencies(
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelVolumeTransform& StampToQuery,
		const FVoxelBox& Bounds) const
	{
	}

	virtual void Apply(
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& StampToQuery) const;

	virtual void Apply(
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery) const
	{
	}
};