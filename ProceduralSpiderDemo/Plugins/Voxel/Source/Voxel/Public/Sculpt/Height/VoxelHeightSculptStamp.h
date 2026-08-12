// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelHeightStamp.h"
#include "Bulk/VoxelBulkPtr.h"
#include "VoxelHeightSculptStamp.generated.h"

class IVoxelBulkLoader;
class AVoxelSculptHeight;
class FVoxelSculptHeightCache;
struct FVoxelHeightModifier;
struct FVoxelSculptHeightData;

USTRUCT(meta = (Internal))
struct VOXEL_API FVoxelHeightSculptStamp final : public FVoxelHeightStamp
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	float ScaleXY = 100;

	// If true, stores the relative height field, based on existing height field while sculpting
	// This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bStoreRelativeHeights = true;

	// Maximum allowed height error in world units after packing.
	// A value of 1 means packed heights will be within 1cm of their original value.
	// Lower values preserve more detail but use more bits per sample.
	// For reference, a 10m height range at precision 1cm requires 10 bits.
	// This is calculated per chunk.
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay, meta = (ClampMin = 0, Units = "cm"))
	float TargetPrecision = 1.f;

	// Set this to true for runtime stamps
	// Will ensure only updated chunks are invalidated when sculpting outside the existing stamp bounds
	// Do not set this to true on too many stamps, this adds a small overhead to all computed chunks
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bIsInfinite = false;

	// Any chunk whose LOD is <= to this will use the Near quality
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 NearMaxLOD = 3;

	// Any chunk whose LOD is <= to this will use the Mid quality
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 MidMaxLOD = 6;

	// Use this if this stamp is not rendered in the Voxel World stack
	// This stack will be used during sculpting to query the distances before any sculpt is applied
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	TObjectPtr<UVoxelLayerStack> StackOverride;

public:
	TVoxelObjectPtr<const AVoxelSculptHeight> WeakSculptActor;

	FVoxelHeightSculptStamp();

	TVoxelOptional<FVoxelWeakStackLayer> GetWeakStackLayer(const UWorld& World) const;

public:
	//~ Begin FVoxelHeightStamp Interface
#if WITH_EDITOR
	virtual void GetPropertyInfo(FPropertyInfo& Info) const override;
#endif
	//~ End FVoxelHeightStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelHeightSculptStampRuntime : public FVoxelHeightStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_BODY(FVoxelHeightSculptStamp)

	TVoxelObjectPtr<const AVoxelSculptHeight> WeakSculptActor;
	TSharedPtr<FVoxelDependency2D> Dependency;
	TSharedPtr<IVoxelBulkLoader> BulkLoader;
	TSharedPtr<const FVoxelSculptHeightData> SculptData;
	FVoxelBulkHash RootHash;

	//~ Begin FVoxelHeightStampRuntime Interface
	virtual bool Initialize(FVoxelDependencyCollector& DependencyCollector) override;
	virtual FVoxelBox GetLocalBounds() const override;
	virtual bool HasCollectDependencies() const override;
	virtual bool CanPartiallyInvalidate() const override;
	virtual bool HasRelativeHeightRange() const override;

	virtual bool TryToPartiallyInvalidate(
		const FVoxelStampRuntime& PreviousRuntime,
		TVoxelArray<FVoxelBox>& OutLocalBoundsToInvalidate) const override;

	virtual void CollectDependencies(
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelHeightTransform& StampToQuery,
		const FVoxelBox2D& Bounds) const override;

	virtual void Apply(
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& StampToQuery) const override;
	//~ End FVoxelHeightStampRuntime Interface
};