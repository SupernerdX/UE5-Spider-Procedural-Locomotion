// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeightCache.h"
#include "Sculpt/Height/VoxelHeightSculptStamp.h"
#include "VoxelQuery.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelHeightSculptCache_Memory);

int64 FVoxelHeightSculptPreviousChunk::GetAllocatedSize() const
{
	return sizeof(*this);
}

FVoxelHeightSculptPreviousChunk::FVoxelHeightSculptPreviousChunk()
{
	UpdateStats();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSculptHeightCache::FVoxelSculptHeightCache(
	const FTransform2d& SculptToWorld,
	const float ScaleZ,
	const float OffsetZ,
	const TVoxelObjectPtr<const AVoxelSculptHeight> SculptActor)
	: SculptToWorld(SculptToWorld)
	, ScaleZ(ScaleZ)
	, OffsetZ(OffsetZ)
	, WeakSculptActor(SculptActor)
{
}

FVoxelSculptHeightCache::~FVoxelSculptHeightCache()
{
	VOXEL_FUNCTION_COUNTER();

	ChunkKeyToChunk_RequiresLock.Empty();
}

TSharedRef<const FVoxelHeightSculptPreviousChunk> FVoxelSculptHeightCache::ComputeChunk(
	const FVoxelLayers& Layers,
	const FVoxelSurfaceTypeTable& SurfaceTypeTable,
	const FVoxelWeakStackLayer& WeakLayer,
	const FIntPoint& ChunkKey)
{
	VOXEL_FUNCTION_COUNTER();

	{
		VOXEL_SCOPE_READ_LOCK(CriticalSection);

		const TSharedPtr<const FVoxelHeightSculptPreviousChunk> Chunk = ChunkKeyToChunk_RequiresLock.FindRef(ChunkKey);
		if (Chunk &&
			!Chunk->DependencyTracker->IsInvalidated())
		{
			return Chunk.ToSharedRef();
		}
	}

	FVoxelDoubleVector2DBuffer Positions;
	{
		VOXEL_SCOPE_COUNTER("Positions");

		Positions.Allocate(ChunkCount);

		for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
		{
			for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
			{
				const int32 Index = FVoxelUtilities::Get2DIndex<int32>(
					ChunkSize,
					IndexX,
					IndexY);

				const FVector2D Position = SculptToWorld.TransformPoint(FVector2D(
					ChunkKey.X * ChunkSize + IndexX,
					ChunkKey.Y * ChunkSize + IndexY));

				Positions.Set(Index, Position);
			}
		}
	}

	const TSharedRef<FVoxelHeightSculptPreviousChunk> NewChunk = MakeShareable(new FVoxelHeightSculptPreviousChunk());

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelHeightSculptPreviousChunk"));

	const TVoxelUniqueFunction<bool(const FVoxelStampRuntime&)> ShouldStopTraversal_BeforeStamp = [&](const FVoxelStampRuntime& NextStamp)
	{
		const FVoxelHeightSculptStampRuntime* SculptStamp = NextStamp.As<FVoxelHeightSculptStampRuntime>();
		if (!SculptStamp ||
			!SculptStamp->SculptData)
		{
			return false;
		}

		return SculptStamp->WeakSculptActor == WeakSculptActor;
	};

	FVoxelQuery Query(
		0,
		Layers,
		SurfaceTypeTable,
		DependencyCollector);

	Query.ShouldStopTraversal_BeforeStamp = &ShouldStopTraversal_BeforeStamp;

	const FVoxelFloatBuffer QueriedHeights = Query.SampleHeightLayer(WeakLayer, Positions);

	FVoxelUtilities::Memcpy(NewChunk->Heights, QueriedHeights.View());

	NewChunk->DependencyTracker = DependencyCollector.Finalize(nullptr, {});

	{
		VOXEL_SCOPE_WRITE_LOCK(CriticalSection);
		ChunkKeyToChunk_RequiresLock.FindOrAdd(ChunkKey) = NewChunk;
	}

	return NewChunk;
}