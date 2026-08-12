// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolumeCache.h"
#include "Sculpt/Volume/VoxelVolumeSculptStamp.h"
#include "VoxelQuery.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelVolumeSculptCache_Memory);

int64 FVoxelVolumeSculptPreviousChunk::GetAllocatedSize() const
{
	return sizeof(*this);
}

FVoxelVolumeSculptPreviousChunk::FVoxelVolumeSculptPreviousChunk()
{
	UpdateStats();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSculptVolumeCache::FVoxelSculptVolumeCache(
	const FMatrix& SculptToWorld,
	const TVoxelObjectPtr<const AVoxelSculptVolume> SculptActor)
	: SculptToWorld(SculptToWorld)
	, WeakSculptActor(SculptActor)
{
}

FVoxelSculptVolumeCache::~FVoxelSculptVolumeCache()
{
	VOXEL_FUNCTION_COUNTER();

	ChunkKeyToChunk_RequiresLock.Empty();
}

TSharedRef<const FVoxelVolumeSculptPreviousChunk> FVoxelSculptVolumeCache::ComputeChunk(
	const FVoxelLayers& Layers,
	const FVoxelSurfaceTypeTable& SurfaceTypeTable,
	const FVoxelWeakStackLayer& WeakLayer,
	const FIntVector& ChunkKey)
{
	VOXEL_FUNCTION_COUNTER();

	{
		VOXEL_SCOPE_READ_LOCK(CriticalSection);

		const TSharedPtr<const FVoxelVolumeSculptPreviousChunk> Chunk = ChunkKeyToChunk_RequiresLock.FindRef(ChunkKey);
		if (Chunk &&
			!Chunk->DependencyTracker->IsInvalidated())
		{
			return Chunk.ToSharedRef();
		}
	}

	FVoxelDoubleVectorBuffer Positions;
	{
		VOXEL_SCOPE_COUNTER("Positions");

		Positions.Allocate(ChunkCount);

		for (int32 IndexZ = 0; IndexZ < ChunkSize; IndexZ++)
		{
			for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
			{
				for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
				{
					const int32 Index = FVoxelUtilities::Get3DIndex<int32>(
						ChunkSize,
						IndexX,
						IndexY,
						IndexZ);

					const FVector Position = SculptToWorld.TransformPosition(FVector(
						ChunkKey.X * ChunkSize + IndexX,
						ChunkKey.Y * ChunkSize + IndexY,
						ChunkKey.Z * ChunkSize + IndexZ));

					Positions.Set(Index, Position);
				}
			}
		}
	}

	const TSharedRef<FVoxelVolumeSculptPreviousChunk> NewChunk = MakeShareable(new FVoxelVolumeSculptPreviousChunk());

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelVolumeSculptPreviousChunk"));

	const TVoxelUniqueFunction<bool(const FVoxelStampRuntime&)> ShouldStopTraversal_BeforeStamp = [&](const FVoxelStampRuntime& NextStamp)
	{
		const FVoxelVolumeSculptStampRuntime* SculptStamp = NextStamp.As<FVoxelVolumeSculptStampRuntime>();
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

	const FVoxelFloatBuffer QueriedDistances = Query.SampleVolumeLayer(WeakLayer, Positions);

	FVoxelUtilities::Memcpy(NewChunk->Distances, QueriedDistances.View());

	const float DistanceScale = SculptToWorld.GetScaleVector().GetAbsMax();

	for (float& Distance : NewChunk->Distances)
	{
		Distance /= DistanceScale;
	}

	NewChunk->DependencyTracker = DependencyCollector.Finalize(nullptr, {});

	{
		VOXEL_SCOPE_WRITE_LOCK(CriticalSection);
		ChunkKeyToChunk_RequiresLock.FindOrAdd(ChunkKey) = NewChunk;
	}

	return NewChunk;
}