// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterCacheManager.h"
#include "Scatter/VoxelScatterFunctionLibrary.h"
#include "VoxelQuery.h"
#include "VoxelDependency.h"
#include "VoxelInvalidationQueue.h"
#include "Graphs/VoxelStampGraphParameters.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelScatterCacheDebug, false,
	"voxel.scatter.cache.Debug",
	"Enables cache debugging, which will always compute input and compare it with cached one.");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelScatterCacheDebugInvalidations, false,
	"voxel.scatter.cache.DebugInvalidations",
	"When enabled, draws a box around each scatter cache chunk whose entry is invalidated/removed.");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelScatterCacheManager::FVoxelScatterCacheManager()
	: Dependency(FVoxelDependency3D::Create("VoxelScatterCacheManager"))
{
}

TSharedRef<const FVoxelPointSet> FVoxelScatterCacheManager::GetCachedChunk(
	const FVoxelGraphEnvironment& Environment,
	const FVoxelCompiledTerminalGraph& TerminalGraph,
	FVoxelDependencyCollector& DependencyCollector,
	const FVoxelLayers& Layers,
	const FVoxelSurfaceTypeTable& SurfaceTypeTable,
	const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
	const float BoundsExtension,
	const TVoxelSet<FName>& AttributesToKeep,
	const FVoxelBox& ChunkBounds,
	const FGuid NodeGuid,
	const uint32 ChunkKey,
	const FVoxelNode::TPinRef_Input<FVoxelPointSet>& Pin,
	bool& bOutWasCached)
{
	DependencyCollector.AddDependency(*Dependency, ChunkBounds.Extend(BoundsExtension));

	bOutWasCached = false;
	if (const TSharedPtr<const FVoxelPointSet> Points = FindCachedChunk(
		WeakActor,
		NodeGuid,
		ChunkKey))
	{
		bOutWasCached = true;
		if (GVoxelScatterCacheDebug)
		{
			TSharedPtr<const FVoxelPointSet> ComputedPoints;
			TSharedPtr<FVoxelDependencyTracker> DependencyTracker;
			TSharedPtr<TVoxelAtomic<bool>> WasInvalidated;
			ComputePoints(
				Environment,
				TerminalGraph,
				Layers,
				SurfaceTypeTable,
				WeakActor,
				NodeGuid,
				ChunkKey,
				ChunkBounds,
				BoundsExtension,
				AttributesToKeep,
				Pin,
				ComputedPoints,
				DependencyTracker,
				WasInvalidated);

			ensure(Points->Equals(*ComputedPoints));
		}

		return Points.ToSharedRef();
	}

	TSharedPtr<const FVoxelPointSet> Points;
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;
	TSharedPtr<TVoxelAtomic<bool>> WasInvalidated;
	ComputePoints(
		Environment,
		TerminalGraph,
		Layers,
		SurfaceTypeTable,
		WeakActor,
		NodeGuid,
		ChunkKey,
		ChunkBounds,
		BoundsExtension,
		AttributesToKeep,
		Pin,
		Points,
		DependencyTracker,
		WasInvalidated);

	StorePoints(
		WeakActor,
		NodeGuid,
		ChunkKey,
		Points.ToSharedRef(),
		DependencyTracker.ToSharedRef(),
		WasInvalidated.ToSharedRef());

	return Points.ToSharedRef();
}

void FVoxelScatterCacheManager::ClearOldCache(
	const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
	const FGuid NodeGuid,
	const float CacheSize)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);

	TVoxelMap<FGuid, FCache>* NodeGuidToCache = ActorToNodeGuidToCache_RequiresLock.Find(WeakActor);
	if (!NodeGuidToCache)
	{
		return;
	}

	FCache* Cache = NodeGuidToCache->Find(NodeGuid);
	if (!Cache)
	{
		return;
	}

	if (Cache->AllocatedSize < CacheSize * 1024 * 1024)
	{
		return;
	}

	struct FKey
	{
		uint32 Key;
		double LastAccess;
	};
	TVoxelArray<FKey> OrderedKeys;
	OrderedKeys.Reserve(Cache->ChunkKeyToChunk.Num());
	for (const auto& It : Cache->ChunkKeyToChunk)
	{
		OrderedKeys.Add_GetRef(FKey(It.Key, It.Value.LastAccess));
	}
	OrderedKeys.Sort([](const FKey& A, const FKey& B)
	{
		return A.LastAccess < B.LastAccess;
	});

	const float ClearCacheUpTo = CacheSize * 1024 * 1024 * 0.5f;
	for (const FKey& Key : OrderedKeys)
	{
		FChunk Chunk;
		if (!ensure(Cache->ChunkKeyToChunk.RemoveAndCopyValue(Key.Key, Chunk)))
		{
			continue;
		}

		if (Chunk.Points)
		{
			Cache->AllocatedSize -= Chunk.Points->GetAllocatedSize();
		}

		if (Cache->AllocatedSize <= ClearCacheUpTo * 1024 * 1024)
		{
			break;
		}
	}
}

void FVoxelScatterCacheManager::ResetCache(const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);

	ActorToNodeGuidToCache_RequiresLock.Remove(WeakActor);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

uint64 FVoxelScatterCacheManager::GetAllocatedSize() const
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);

	uint64 AllocatedSize = 0;
	for (const auto& It : ActorToNodeGuidToCache_RequiresLock)
	{
		for (const auto& InnerIt : It.Value)
		{
			AllocatedSize += InnerIt.Value.AllocatedSize;
		}
	}
	return AllocatedSize;
}

uint64 FVoxelScatterCacheManager::GetAllocatedSize(const FGuid NodeGuid) const
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);

	uint64 AllocatedSize = 0;
	for (const auto& It : ActorToNodeGuidToCache_RequiresLock)
	{
		const FCache* NodeCache = It.Value.Find(NodeGuid);
		if (!NodeCache)
		{
			continue;
		}

		AllocatedSize += NodeCache->AllocatedSize;
	}
	return AllocatedSize;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<const FVoxelPointSet> FVoxelScatterCacheManager::FindCachedChunk(
	const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
	const FGuid NodeGuid,
	const uint32 ChunkKey)
{
	VOXEL_SCOPE_LOCK(CriticalSection);
	const TVoxelMap<FGuid, FCache>* NodeGuidToCache = ActorToNodeGuidToCache_RequiresLock.Find(WeakActor);
	if (!NodeGuidToCache)
	{
		return nullptr;
	}

	const FCache* Cache = NodeGuidToCache->Find(NodeGuid);
	if (!Cache)
	{
		return nullptr;
	}

	const FChunk* Chunk = Cache->ChunkKeyToChunk.Find(ChunkKey);
	if (!Chunk)
	{
		return nullptr;
	}

	Chunk->LastAccess = FPlatformTime::Seconds();
	return Chunk->Points;
}

void FVoxelScatterCacheManager::ComputePoints(
	const FVoxelGraphEnvironment& Environment,
	const FVoxelCompiledTerminalGraph& TerminalGraph,
	const FVoxelLayers& Layers,
	const FVoxelSurfaceTypeTable& SurfaceTypeTable,
	const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
	const FGuid NodeGuid,
	const uint32 ChunkKey,
	const FVoxelBox& Bounds,
	const float BoundsExtension,
	const TVoxelSet<FName>& AttributesToKeep,
	const FVoxelNode::TPinRef_Input<FVoxelPointSet>& Pin,
	TSharedPtr<const FVoxelPointSet>& OutPoints,
	TSharedPtr<FVoxelDependencyTracker>& OutDependencyTracker,
	TSharedPtr<TVoxelAtomic<bool>>& OutWasInvalidated)
{
	// Per-compute invalidation queue. Captures any Dependency::Invalidate that
	// happens between now and Finalize, so they can be replayed against the
	// freshly-created tracker. Without this, an invalidation arriving while we
	// run on the worker thread is delivered to the (not-yet-existing) tracker
	// and silently lost - leaving the cache entry permanently stale until the
	// next invalidation that overlaps the same chunk happens to arrive.
	//
	// bFlush=false: the flushing path asserts game-thread. We only need this
	// queue to capture in-window invalidations; we don't need a consistent
	// snapshot point relative to the stamp manager's pending-updates view.
	const TSharedRef<FVoxelInvalidationQueue> InvalidationQueue = FVoxelInvalidationQueue::Create(false);

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelNode_CachePoints Chunk"));

	FVoxelGraphContext Context(
		Environment,
		TerminalGraph,
		DependencyCollector);
	FVoxelGraphQueryImpl& Query = Context.MakeQuery();
	FVoxelQuery VoxelQuery = FVoxelQuery(
		0,
		Layers,
		SurfaceTypeTable,
		DependencyCollector);
	Query.AddParameter<FVoxelGraphParameters::FQuery>(VoxelQuery);
	Query.AddParameter<FVoxelGraphParameters::FPointSetChunkBounds>(Bounds, true);
	{
		FVoxelGraphParameters::FScatterCacheManager& Parameter = Query.AddParameter<FVoxelGraphParameters::FScatterCacheManager>();
		Parameter.CacheManager = SharedThis(this);
		Parameter.WeakActor = WeakActor;
	}

	OutPoints = INLINE_LAMBDA -> TSharedRef<const FVoxelPointSet>
	{
		const TSharedRef<const FVoxelPointSet> Points = Pin.GetSynchronous(Query);
		if (Points->Num() == 0)
		{
			return MakeShared<FVoxelPointSet>();
		}

		const FVoxelDoubleVectorBuffer* PositionBuffer = Points->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
		if (!PositionBuffer)
		{
			return MakeShared<FVoxelPointSet>();
		}

		const FVoxelBoolBuffer* IsNeighborBuffer = Points->Find<FVoxelBoolBuffer>(FVoxelPointAttributes::IsNeighbor);

		TVoxelArray<int32> IndicesToKeep;
		IndicesToKeep.Reserve(Points->Num());

		const FVoxelBox ExtendedBounds = Bounds.Extend(BoundsExtension);
		for (int32 Index = 0; Index < Points->Num(); Index++)
		{
			if (!ExtendedBounds.Contains((*PositionBuffer)[Index]))
			{
				continue;
			}

			if (IsNeighborBuffer &&
				(*IsNeighborBuffer)[Index])
			{
				continue;
			}

			IndicesToKeep.Add(Index);
		}

		TSharedRef<FVoxelPointSet> NewPoints = Points->Gather(IndicesToKeep);
		NewPoints->KeepAttributes(AttributesToKeep);

		if (NewPoints->Num() == 0)
		{
			return MakeShared<FVoxelPointSet>();
		}

		return NewPoints;
	};

	OutWasInvalidated = MakeShared<TVoxelAtomic<bool>>(false);

	OutDependencyTracker = DependencyCollector.Finalize(
		&InvalidationQueue.Get(),
		MakeWeakPtrLambda(this, [this, Bounds, WeakActor, NodeGuid, ChunkKey, WasInvalidated = OutWasInvalidated](const FVoxelInvalidationCallstack& Callstack)
		{
			// Set before taking the cache lock so a racing StorePoints sees it under
			// its own lock acquisition. The tracker's OnInvalidated has been moved
			// out at this point - if StorePoints inserts after this fires, the entry
			// would be permanently stale (no callback left to ever remove it).
			WasInvalidated->Set(true);

			RemoveFromCache(
				WeakActor,
				NodeGuid,
				ChunkKey);
			Dependency->Invalidate(Bounds);

			if (GVoxelScatterCacheDebugInvalidations)
			{
				FVoxelDebugDrawer()
				.Color(FLinearColor::Red)
				.LifeTime(1.f)
				.DrawBox(Bounds, FTransform::Identity);
			}
		}));
}

void FVoxelScatterCacheManager::StorePoints(
	const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
	const FGuid NodeGuid,
	const uint32 ChunkKey,
	const TSharedRef<const FVoxelPointSet>& Points,
	const TSharedRef<FVoxelDependencyTracker>& DependencyTracker,
	const TSharedRef<TVoxelAtomic<bool>>& WasInvalidated)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);

	if (WasInvalidated->Get())
	{
		// The dependency tracker already fired its invalidation callback -
		// either synchronously inside Finalize (queue replayed an in-window
		// invalidation) or via the live tracker path racing us to the lock.
		// The tracker's OnInvalidated has been moved out and will never fire
		// again, so inserting now would leak a permanently stale entry. The
		// callback already re-invalidated the outer Dependency, so the caller
		// will re-query and recompute against fresh inputs.
		return;
	}

	FCache& Cache = ActorToNodeGuidToCache_RequiresLock.FindOrAdd(WeakActor).FindOrAdd(NodeGuid);
	if (Cache.ChunkKeyToChunk.Contains(ChunkKey))
	{
		return;
	}

	FChunk& NewChunk = Cache.ChunkKeyToChunk.Add_EnsureNew(ChunkKey);
	NewChunk.Points = Points;
	NewChunk.Tracker = DependencyTracker;
	NewChunk.LastAccess = FPlatformTime::Seconds();
	Cache.AllocatedSize += NewChunk.Points->GetAllocatedSize();
}

void FVoxelScatterCacheManager::RemoveFromCache(
	const TVoxelObjectPtr<AVoxelScatterActor>& WeakActor,
	const FGuid NodeGuid,
	const uint32 ChunkKey)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);

	TVoxelMap<FGuid, FCache>* NodeGuidToCache = ActorToNodeGuidToCache_RequiresLock.Find(WeakActor);
	if (!NodeGuidToCache)
	{
		return;
	}

	FCache* Cache = NodeGuidToCache->Find(NodeGuid);
	if (!Cache)
	{
		return;
	}

	FChunk Chunk;
	if (!Cache->ChunkKeyToChunk.RemoveAndCopyValue(ChunkKey, Chunk))
	{
		return;
	}

	Cache->AllocatedSize -= Chunk.Points->GetAllocatedSize();
}