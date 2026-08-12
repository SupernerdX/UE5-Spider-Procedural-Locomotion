// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FNavigationInvokerRaw;

class VOXEL_API FVoxelNavigationInvokerView : public TSharedFromThis<FVoxelNavigationInvokerView>
{
public:
	FVoxelNavigationInvokerView(
		int32 ChunkSize,
		const FTransform& LocalToWorld);

	TVoxelFuture<const TVoxelSet<FIntVector>> GetChunks(FVoxelDependencyCollector& DependencyCollector) const;

	void Tick(const TArray<FNavigationInvokerRaw>& InvokerLocations);

private:
	const int32 ChunkSize;
	const FTransform LocalToWorld;
	const TSharedRef<FVoxelDependency> Dependency;

	FVoxelFuture Future;
	TVoxelArray<FSphere> LastInvokers;

	FVoxelCriticalSection CriticalSection;
	TSharedPtr<const TVoxelSet<FIntVector>> Chunks_RequiresLock;
};

class VOXEL_API FVoxelNavigationInvokerManager : public IVoxelWorldSubsystem
{
public:
	GENERATED_VOXEL_WORLD_SUBSYSTEM_BODY(FVoxelNavigationInvokerManager);

	TSharedRef<FVoxelNavigationInvokerView> MakeView(
		int32 ChunkSize,
		const FTransform& LocalToWorld);

	//~ Begin IVoxelWorldSubsystem Interface
	virtual void Tick() override;
	//~ End IVoxelWorldSubsystem Interface

private:
	double LastTickTime = 0;
	TVoxelArray<TWeakPtr<FVoxelNavigationInvokerView>> WeakViews;
};