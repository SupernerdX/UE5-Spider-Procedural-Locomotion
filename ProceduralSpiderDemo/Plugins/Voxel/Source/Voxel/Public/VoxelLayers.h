// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"

class FVoxelQuery;
class FVoxelHeightLayer;
class FVoxelVolumeLayer;
class FVoxelMetadataView;
struct FVoxelStampDelta;

class VOXEL_API FVoxelLayers : public TSharedFromThis<FVoxelLayers>
{
public:
	static FVoxelLayers& Empty();

	static TSharedRef<FVoxelLayers> Get(TVoxelObjectPtr<UWorld> World);

	static TSharedRef<FVoxelLayers> Get(
		TVoxelObjectPtr<UWorld> World,
		const AActor& Actor);

public:
	const TVoxelObjectPtr<UWorld> World;

	~FVoxelLayers();
	UE_NONCOPYABLE(FVoxelLayers);

public:
	bool HasLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		FVoxelDependencyCollector& DependencyCollector) const;

	TSharedPtr<const FVoxelHeightLayer> FindHeightLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		FVoxelDependencyCollector& DependencyCollector) const;

	TSharedPtr<const FVoxelVolumeLayer> FindVolumeLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		FVoxelDependencyCollector& DependencyCollector) const;

public:
	FVoxelOptionalBox GetBoundsToGenerate(
		const FVoxelWeakStackLayer& WeakLayer,
		FVoxelDependencyCollector& DependencyCollector) const;

	TVoxelArray<FVoxelStampDelta> GetStampDeltas(
		const FVoxelWeakStackLayer& WeakLayer,
		const FVector& Position,
		int32 LOD) const;

private:
	const TSharedRef<FVoxelDependency> Dependency;
	const int64 Timestamp;
	const TVoxelMap<FVoxelWeakStackLayer, TSharedPtr<const FVoxelHeightLayer>> WeakLayerToHeightLayer;
	const TVoxelMap<FVoxelWeakStackLayer, TSharedPtr<const FVoxelVolumeLayer>> WeakLayerToVolumeLayer;

	FVoxelLayers(
		const TVoxelObjectPtr<UWorld> World,
		const TSharedRef<FVoxelDependency>& Dependency,
		const int64 Timestamp,
		TVoxelMap<FVoxelWeakStackLayer, TSharedPtr<const FVoxelHeightLayer>>&& WeakLayerToHeightLayer,
		TVoxelMap<FVoxelWeakStackLayer, TSharedPtr<const FVoxelVolumeLayer>>&& WeakLayerToVolumeLayer)
		: World(World)
		, Dependency(Dependency)
		, Timestamp(Timestamp)
		, WeakLayerToHeightLayer(MoveTemp(WeakLayerToHeightLayer))
		, WeakLayerToVolumeLayer(MoveTemp(WeakLayerToVolumeLayer))
	{
	}

	friend class FVoxelLayerTracker;
	friend class FVoxelLayerTrackerSubsystem;
};