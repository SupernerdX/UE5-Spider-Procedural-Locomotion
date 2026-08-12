// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelRenderNeighborSubdivider.h"
#include "Render/VoxelRenderTree.h"
#include "Render/VoxelRenderChunk.h"
#include "Render/VoxelRenderSubsystem.h"
#include "VoxelMesher.h"

FVoxelRenderNeighborSubdivider::FVoxelRenderNeighborSubdivider(
	FVoxelRenderSubsystem& Subsystem,
	FVoxelRenderTree& Tree,
	const TVoxelStaticArray<TSharedRef<FVoxelRenderChunk>, 8>& RootChunks)
	: Subsystem(Subsystem)
	, Tree(Tree)
	, RootChunks(RootChunks)
	, MaxLOD(RootChunks[0]->ChunkKey.LOD)
{
	InitializeChunks();
}

void FVoxelRenderNeighborSubdivider::Traverse()
{
	VOXEL_FUNCTION_COUNTER();

	ON_SCOPE_EXIT
	{
		Scope.FlushTasks();
	};

	TVoxelArray<FVoxelRenderChunk*> ChunksToTraverse;
	ChunksToTraverse.Reserve(8 * (MaxLOD + 1));

	for (const TSharedRef<FVoxelRenderChunk>& RootChunk : RootChunks)
	{
		ChunksToTraverse.Add_EnsureNoGrow(&RootChunk.Get());
	}

	while (ChunksToTraverse.Num() > 0)
	{
		FVoxelRenderChunk& Chunk = *ChunksToTraverse.Pop();

		if (Chunk.Children.Num() > 0)
		{
			for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.Children)
			{
				ChunksToTraverse.Add_EnsureNoGrow(Child.Get());
			}
		}
		else
		{
			ChunksToCheck.Add(&Chunk);

			if (ChunksToCheck.Num() == MaxChunksToCheck)
			{
				if (ShouldCancel ||
					HasReachedMaxChunks.Get())
				{
					return;
				}

				FlushChunksToCheck();
			}
		}
	}

	FlushChunksToCheck();
}

void FVoxelRenderNeighborSubdivider::ProcessNewChunks()
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_READ_LOCK(CriticalSection);

	while (NewChunks.Num() > 0)
	{
		VOXEL_SCOPE_COUNTER_FORMAT("Process new chunks Num=%d", NewChunks.Num());

		if (ShouldCancel ||
			HasReachedMaxChunks.Get())
		{
			return;
		}

		const TVoxelChunkedArray<const FVoxelRenderChunk*> ChunksToProcess = MoveTemp(NewChunks);
		check(NewChunks.Num() == 0);

		for (const FVoxelRenderChunk* Chunk : ChunksToProcess)
		{
			if (Chunk->Children.Num() > 0)
			{
				// Subdivided since we added it
				continue;
			}

			CheckChunk(*Chunk);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderNeighborSubdivider::InitializeChunks()
{
	VOXEL_FUNCTION_COUNTER_NUM(Tree.ChunkKeyToChunk.Num(), 1);

	// Keep some space for new chunks
	Chunks.Reserve(Tree.ChunkKeyToChunk.Num() * 1.2);
	IsLeaf.Reserve(Tree.ChunkKeyToChunk.Num() * 1.2);

	IsLeaf.SetNum(Tree.ChunkKeyToChunk.Num(), false);

	for (const auto& It : Tree.ChunkKeyToChunk)
	{
		const FVoxelSetIndex Index = Chunks.Add_CheckNew(It.Key);

		if (It.Value.Get()->Children.Num() == 0)
		{
			IsLeaf[Index.GetIndex()] = true;
		}
	}
}

void FVoxelRenderNeighborSubdivider::FlushChunksToCheck()
{
	Scope.AddTask([this, LocalChunksToCheck = ChunksToCheck]
	{
		VOXEL_SCOPE_COUNTER("Check chunks");
		VOXEL_SCOPE_READ_LOCK(CriticalSection);

		if (ShouldCancel)
		{
			return;
		}

		for (const FVoxelRenderChunk* Chunk : LocalChunksToCheck)
		{
			CheckChunk(*Chunk);
		}
	});

	ChunksToCheck.Reset();
}

void FVoxelRenderNeighborSubdivider::CheckChunk(const FVoxelRenderChunk& Chunk)
{
	checkVoxelSlow(CriticalSection.IsLocked_Read());
	checkVoxelSlow(Chunk.ChunkKey.ChunkKey % (1 << Chunk.ChunkKey.LOD) == 0);

	const FVoxelChunkKey BaseChunkKey = Chunk.ChunkKey;

	for (int32 X = -1; X <= 1; X++)
	{
		for (int32 Y = -1; Y <= 1; Y++)
		{
			for (int32 Z = -1; Z <= 1; Z++)
			{
			Loop:
#if VOXEL_DEBUG
				const FVoxelRenderChunk* Neighbor = Tree.FindNeighbor(Chunk.ChunkKey, X, Y, Z);
#endif

				FVoxelChunkKey ChunkKey = BaseChunkKey;

				FVoxelChunkKey NeighborChunkKey = BaseChunkKey;
				NeighborChunkKey.ChunkKey.X += X << BaseChunkKey.LOD;
				NeighborChunkKey.ChunkKey.Y += Y << BaseChunkKey.LOD;
				NeighborChunkKey.ChunkKey.Z += Z << BaseChunkKey.LOD;

				while (true)
				{
					const FVoxelSetIndex Index = Chunks.Find(NeighborChunkKey);
					if (Index.IsValid())
					{
						if (IsLeaf[Index.GetIndex()])
						{
							checkVoxelSlow(Neighbor);
							checkVoxelSlow(Neighbor->ChunkKey == NeighborChunkKey);

							if (NeighborChunkKey.LOD - BaseChunkKey.LOD <= FVoxelMesher::MaxRelativeLOD)
							{
								goto Next;
							}

							SubdivideChunk(NeighborChunkKey);

							if (HasReachedMaxChunks.Get(std::memory_order::relaxed))
							{
								return;
							}

							goto Loop;
						}
						else
						{
							// Actual neighbor is one of NeighborChunk children and is higher res than us
							goto Next;
						}
					}

					checkVoxelSlow(ChunkKey.LOD == NeighborChunkKey.LOD);

					ChunkKey = ChunkKey.GetParent();
					NeighborChunkKey = NeighborChunkKey.GetParent();

					if (ChunkKey.LOD > MaxLOD)
					{
						// No neighbor or neighbor is higher res than us
						goto Next;
					}

					if (ChunkKey == NeighborChunkKey)
					{
						// No neighbor or neighbor is higher res than us: we walked up the tree until we reached back our own roots
						goto Next;
					}
				}

			Next:
				checkVoxelSlow(!Neighbor || (Neighbor->ChunkKey.LOD - BaseChunkKey.LOD <= FVoxelMesher::MaxRelativeLOD));
				continue;
			}
		}
	}
}

void FVoxelRenderNeighborSubdivider::SubdivideChunk(const FVoxelChunkKey& ChunkKey)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_WRITE_LOCK_PROMOTED(CriticalSection);

	FVoxelRenderChunk& Chunk = *Tree.ChunkKeyToChunk[ChunkKey];

	if (Chunk.Children.Num() > 0)
	{
		// Another thread has already subdivided this chunk while we were trying to lock
		return;
	}

	extern VOXEL_API int32 GVoxelMaxRenderChunks;

	if (Tree.ChunkKeyToChunk.Num() > GVoxelMaxRenderChunks)
	{
		if (!HasReachedMaxChunks.Set_ReturnOld(true))
		{
			VOXEL_MESSAGE(Error, "voxel.MaxRenderChunks reached");
		}
		return;
	}

	IsLeaf[Chunks.Find(Chunk.ChunkKey).GetIndex()] = false;

	Tree.SubdivideChunk(Chunk);

	for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.Children)
	{
		const FVoxelSetIndex Index = Chunks.Add(Child->ChunkKey);
		ensure(Index.GetIndex() == IsLeaf.Add(true));

		checkVoxelSlow(!NewChunks.Contains(Child.Get()));
		NewChunks.Add(Child.Get());
	}
}