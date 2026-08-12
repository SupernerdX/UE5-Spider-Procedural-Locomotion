// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelVolumeStamp.h"
#include "Bulk/VoxelBulkPtr.h"
#include "VoxelVolumeSculptStamp.generated.h"

class IVoxelBulkLoader;
class AVoxelSculptVolume;
class FVoxelSculptVolumeCache;
struct FVoxelVolumeModifier;
struct FVoxelSculptVolumeData;

USTRUCT(meta = (Internal))
struct VOXEL_API FVoxelVolumeSculptStamp final : public FVoxelVolumeStamp
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	float Scale = 100;

	// If true, stores the distance field into two floats: a Additive and a Subtractive distance
	// This allows changing the underlying world (eg, moving a stamp) without obvious chunks being left over
	// Will use 2-4x more memory/disk space
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bStoreMovableDistances = true;

	// Max error percentage allowed when saving
	// This is used to pick the bit depth to use when saving a chunk
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay, meta = (UIMin = 0, UIMax = 100, Units = "Percent", EditCondition = "!bStoreMovableDistances"))
	float MaxErrorPercentage = 0.5f;

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
	TVoxelObjectPtr<const AVoxelSculptVolume> WeakSculptActor;

	FVoxelVolumeSculptStamp();

	TVoxelOptional<FVoxelWeakStackLayer> GetWeakStackLayer(const UWorld& World) const;

public:
	//~ Begin FVoxelVolumeStamp Interface
#if WITH_EDITOR
	virtual void GetPropertyInfo(FPropertyInfo& Info) const override;
#endif
	//~ End FVoxelVolumeStamp Interface
};

USTRUCT()
struct VOXEL_API FVoxelVolumeSculptStampRuntime : public FVoxelVolumeStampRuntime
{
	GENERATED_BODY()
	GENERATED_VOXEL_RUNTIME_STAMP_BODY(FVoxelVolumeSculptStamp)

	TVoxelObjectPtr<const AVoxelSculptVolume> WeakSculptActor;
	TSharedPtr<FVoxelDependency3D> Dependency;
	TSharedPtr<IVoxelBulkLoader> BulkLoader;
	TSharedPtr<const FVoxelSculptVolumeData> SculptData;
	FVoxelBulkHash RootHash;

	//~ Begin FVoxelVolumeStampRuntime Interface
	virtual bool Initialize(FVoxelDependencyCollector& DependencyCollector) override;
	virtual FVoxelBox GetLocalBounds() const override;
	virtual bool HasCollectDependencies() const override;
	virtual bool CanPartiallyInvalidate() const override;

	virtual bool TryToPartiallyInvalidate(
		const FVoxelStampRuntime& PreviousRuntime,
		TVoxelArray<FVoxelBox>& OutLocalBoundsToInvalidate) const override;

	virtual void CollectDependencies(
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelVolumeTransform& StampToQuery,
		const FVoxelBox& Bounds) const override;

	virtual void Apply(
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& StampToQuery) const override;
	//~ End FVoxelVolumeStampRuntime Interface
};