// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNodePinRef.h"
#include "VoxelScatterNodeRef.h"

class FVoxelLayers;
class FVoxelDependency3D;
class FVoxelSurfaceTypeTable;
class FVoxelScatterCacheManager;
struct FVoxelPointSet;

namespace FVoxelGraphParameters
{
	struct FScatterCacheManager : FUniformParameter
	{
		TSharedPtr<FVoxelScatterCacheManager> CacheManager;
		TVoxelObjectPtr<AVoxelScatterActor> WeakActor;
	};
}

class FVoxelScatterCacheManager : public TSharedFromThis<FVoxelScatterCacheManager>
{
private:
	struct FChunk
	{
		TSharedPtr<const FVoxelPointSet> Points;
		TSharedPtr<FVoxelDependencyTracker> Tracker;
		mutable double LastAccess = 0.;
	};
	struct FCache
	{
		TVoxelMap<uint32, FChunk> ChunkKeyToChunk;
		int64 AllocatedSize = 0;
	};
	FVoxelCriticalSection CriticalSection;
	TVoxelMap<TVoxelObjectPtr<AVoxelScatterActor>, TVoxelMap<FGuid, FCache>> ActorToNodeGuidToCache_RequiresLock;

	const TSharedRef<FVoxelDependency3D> Dependency;

public:
	FVoxelScatterCacheManager();

	TSharedRef<const FVoxelPointSet> GetCachedChunk(
		const FVoxelGraphEnvironment& Environment,
		const FVoxelCompiledTerminalGraph& TerminalGraph,
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelLayers& Layers,
		const FVoxelSurfaceTypeTable& SurfaceTypeTable,
		const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
		float BoundsExtension,
		const TVoxelSet<FName>& AttributesToKeep,
		const FVoxelBox& ChunkBounds,
		FGuid NodeGuid,
		uint32 ChunkKey,
		const FVoxelNode::TPinRef_Input<FVoxelPointSet>& Pin,
		bool& bOutWasCached);
	void ClearOldCache(
		const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
		FGuid NodeGuid,
		float CacheSize);
	void ResetCache(const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor);

public:
	uint64 GetAllocatedSize() const;
	uint64 GetAllocatedSize(FGuid NodeGuid) const;

private:
	TSharedPtr<const FVoxelPointSet> FindCachedChunk(
		const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
		FGuid NodeGuid,
		uint32 ChunkKey);
	void ComputePoints(
		const FVoxelGraphEnvironment& Environment,
		const FVoxelCompiledTerminalGraph& TerminalGraph,
		const FVoxelLayers& Layers,
		const FVoxelSurfaceTypeTable& SurfaceTypeTable,
		const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
		FGuid NodeGuid,
		uint32 ChunkKey,
		const FVoxelBox& Bounds,
		float BoundsExtension,
		const TVoxelSet<FName>& AttributesToKeep,
		const FVoxelNode::TPinRef_Input<FVoxelPointSet>& Pin,
		TSharedPtr<const FVoxelPointSet>& OutPoints,
		TSharedPtr<FVoxelDependencyTracker>& OutDependencyTracker,
		TSharedPtr<TVoxelAtomic<bool>>& OutWasInvalidated);
	void StorePoints(
		const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
		FGuid NodeGuid,
		uint32 ChunkKey,
		const TSharedRef<const FVoxelPointSet>& Points,
		const TSharedRef<FVoxelDependencyTracker>& DependencyTracker,
		const TSharedRef<TVoxelAtomic<bool>>& WasInvalidated);
	void RemoveFromCache(
		const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
		FGuid NodeGuid,
		uint32 ChunkKey);
};