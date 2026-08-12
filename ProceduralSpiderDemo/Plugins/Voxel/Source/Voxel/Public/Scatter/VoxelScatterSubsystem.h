// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSubsystem.h"
#include "Scatter/VoxelScatterNodeRef.h"
#include "VoxelScatterSubsystem.generated.h"

class FVoxelScatterNodeRuntime;
class FVoxelScatterInvokersView;
class FVoxelScatterCacheManager;

USTRUCT()
struct VOXEL_API FVoxelScatterSubsystem : public FVoxelSubsystem
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

	FORCEINLINE TSharedPtr<FVoxelScatterCacheManager> GetCacheManager() const
	{
		return CacheManager;
	}

private:
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;
	TVoxelMap<FVoxelScatterNodeWeakRef, TSharedPtr<FVoxelScatterNodeRuntime>> NodeRefToRuntime;
	TVoxelMap<FVoxelScatterNodeWeakRef, TSharedPtr<FVoxelScatterNodeRuntime>> PreviousNodeRefToRuntime;
	TSharedPtr<FVoxelScatterInvokersView> InvokerView;
	TSharedPtr<FVoxelDependencyTracker> InvokersDependencyTracker;
	TSharedPtr<FVoxelScatterCacheManager> CacheManager;
};