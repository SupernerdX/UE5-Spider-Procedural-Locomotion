// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampTree.h"

class FVoxelLayerTracker;
class FVoxelStampLayerManager;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

constexpr int32 GVoxelMaxStampLOD = 20;

class FVoxelLayerTrackerStamps : public TSharedFromThis<FVoxelLayerTrackerStamps>
{
public:
	const bool bIs2D;
	const float StackMaxDistance;
	const TVoxelObjectPtr<const UVoxelLayer> Layer;
	const TSharedRef<FVoxelStampLayerManager> LayerManager;
	const TSharedRef<FVoxelCounter64> LayerTimestamp = MakeShared<FVoxelCounter64>();
	const TSharedRef<FVoxelCounter64> TrackerTimestamp;
	const FTransform QueryToWorld;
	const FTransform WorldToQuery;

	static TSharedRef<FVoxelLayerTrackerStamps> Create(
		const FVoxelLayerTracker& Tracker,
		float StackMaxDistance,
		UVoxelLayer& Layer);

public:
	static constexpr int32 NumLODs = GVoxelMaxStampLOD + 1;

	FVoxelBox IntersectBounds = FVoxelBox::Infinite;
	TVoxelArray<TSharedRef<FVoxelFutureStampTree>> LODToTree;
	TVoxelArray<bool> ShouldSplit;

	bool UpdateStampsIfNeeded(const FVoxelLayerTrackerStamps* PreviousStamps);

private:
	int64 Timestamp = -1;
	const TVoxelStaticArray<TSharedRef<FVoxelDependency3D>, NumLODs> LODToDependency;

	explicit FVoxelLayerTrackerStamps(
		const FVoxelLayerTracker& Tracker,
		float StackMaxDistance,
		UVoxelLayer& Layer);

	void Initialize();

	TSharedRef<FVoxelStampTree> CreateTree(
		int32 LOD,
		TVoxelArray<TSharedRef<const FVoxelStampRuntime>> Stamps,
		const TSharedPtr<FVoxelStampTree>& PreviousTree) const;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelLayerTracker : public TSharedFromThis<FVoxelLayerTracker>
{
public:
	const TVoxelObjectPtr<UWorld> World;
	const TVoxelObjectPtr<const AActor> WeakActor;
	const TSharedRef<FVoxelCounter64> Timestamp = MakeShared<FVoxelCounter64>();
	FTransform QueryToWorld = FTransform::Identity;

	static TSharedRef<FVoxelLayerTracker> Create(
		TVoxelObjectPtr<UWorld> World,
		const AActor* Actor);

	void UpdateLayers();

	// Return a snapshot of layers
	TSharedRef<FVoxelLayers> GetLayers();

private:
	const TSharedRef<FVoxelDependency> Dependency;
	TSharedPtr<FVoxelLayers> Layers;
	TVoxelMap<TVoxelObjectPtr<const UVoxelLayer>, TSharedPtr<FVoxelLayerTrackerStamps>> LayerToStamps;

	FVoxelLayerTracker(
		TVoxelObjectPtr<UWorld> World,
		TVoxelObjectPtr<const AActor> WeakActor);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelLayerTrackerSubsystem : public IVoxelWorldSubsystem
{
public:
	GENERATED_VOXEL_WORLD_SUBSYSTEM_BODY(FVoxelLayerTrackerSubsystem);

	TSharedRef<FVoxelLayerTracker> GetLayerTracker();
	TSharedRef<FVoxelLayerTracker> GetLayerTracker(const AActor& Actor);

	//~ Begin IVoxelWorldSubsystem Interface
	virtual void Tick() override;
	//~ End IVoxelWorldSubsystem Interface

private:
	TSharedPtr<FVoxelLayerTracker> RootLayerTracker;
	TVoxelMap<TVoxelObjectPtr<const AActor>, TSharedPtr<FVoxelLayerTracker>> ActorToLayerTracker;
};