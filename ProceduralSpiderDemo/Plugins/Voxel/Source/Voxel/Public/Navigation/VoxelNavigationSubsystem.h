// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSubsystem.h"
#include "VoxelNavigationSubsystem.generated.h"

class FVoxelNavigationMesh;
class FVoxelNavigationInvokerView;

USTRUCT()
struct VOXEL_API FVoxelNavigationSubsystem : public FVoxelSubsystem
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_BODY()

public:
	//~ Begin FVoxelSubsystem Interface
	virtual void LoadFromPrevious(FVoxelSubsystem& InPreviousSubsystem) override;
	virtual void Initialize() override;
	virtual void Compute() override;
	virtual void Render(FVoxelRuntime& Runtime) override;
	//~ End FVoxelSubsystem Interface

private:
	TSharedPtr<FVoxelNavigationInvokerView> InvokerView;
	TSharedPtr<FVoxelDependencyTracker> InvokerViewDependencyTracker;

	TVoxelArray<FVoxelBox> NavMeshBounds;

	FVoxelCriticalSection CriticalSection;

	TVoxelMap<FIntVector, TSharedPtr<FVoxelNavigationMesh>> ChunkKeyToNavigationMesh_RequiresLock;
	TVoxelMap<FIntVector, TVoxelObjectPtr<USceneComponent>> ChunkKeyToNavigationComponent;

	TVoxelMap<FIntVector, TSharedPtr<FVoxelNavigationMesh>> PreviousChunkKeyToNavigationMesh;
	TVoxelMap<FIntVector, TVoxelObjectPtr<USceneComponent>> PreviousChunkKeyToNavigationComponent;

	TVoxelMap<FIntVector, double> ChunkKeyToLastRenderTime;
};