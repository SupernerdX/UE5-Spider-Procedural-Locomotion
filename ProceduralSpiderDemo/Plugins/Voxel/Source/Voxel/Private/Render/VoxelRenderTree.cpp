// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelRenderTree.h"
#include "Render/VoxelRenderChunk.h"
#include "Render/VoxelRenderSubsystem.h"
#include "Render/VoxelScreenSizeHelper.h"
#include "Render/VoxelRenderInvokersContainer.h"
#include "Render/VoxelRenderNeighborSubdivider.h"
#include "VoxelMesh.h"
#include "VoxelMesher.h"
#include "VoxelSubsystem.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, int32, GVoxelMaxRenderChunks, 2 * 1024 * 1024,
	"voxel.render.MaxRenderChunks",
	"Max number of render chunks allowed before erroring out");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, int32, GVoxelMaxChunksToProcessPerPass, 32,
	"voxel.render.MaxChunksToProcessPerPass",
	"Max number of chunks to subdivide per pass when the Quality is good enough but not at max");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, float, GVoxelMinQualityDividerWhenEmpty, 4,
	"voxel.render.MinQualityDividerWhenEmpty",
	"Decrease this if some areas do not generate. Decreasing will reduce the amount of empty chunks skipped, reducing gen speed.");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRenderTree::FVoxelRenderTree(
	const FVoxelRenderSubsystem& Subsystem,
	const int32 Depth)
	: VoxelSize(Subsystem.GetConfig().VoxelSize)
	, Depth(Depth)
	, RootChunks(INLINE_LAMBDA
	{
		const FIntVector Min = FIntVector(-(1 << Depth));
		const FIntVector Max = FIntVector(0);

		return TVoxelStaticArray<TSharedRef<FVoxelRenderChunk>, 8>
		{
			MakeShared<FVoxelRenderChunk>(FVoxelChunkKey(Depth, FIntVector(Min.X, Min.Y, Min.Z))),
			MakeShared<FVoxelRenderChunk>(FVoxelChunkKey(Depth, FIntVector(Max.X, Min.Y, Min.Z))),
			MakeShared<FVoxelRenderChunk>(FVoxelChunkKey(Depth, FIntVector(Min.X, Max.Y, Min.Z))),
			MakeShared<FVoxelRenderChunk>(FVoxelChunkKey(Depth, FIntVector(Max.X, Max.Y, Min.Z))),
			MakeShared<FVoxelRenderChunk>(FVoxelChunkKey(Depth, FIntVector(Min.X, Min.Y, Max.Z))),
			MakeShared<FVoxelRenderChunk>(FVoxelChunkKey(Depth, FIntVector(Max.X, Min.Y, Max.Z))),
			MakeShared<FVoxelRenderChunk>(FVoxelChunkKey(Depth, FIntVector(Min.X, Max.Y, Max.Z))),
			MakeShared<FVoxelRenderChunk>(FVoxelChunkKey(Depth, FIntVector(Max.X, Max.Y, Max.Z)))
		};
	})
{
	VOXEL_FUNCTION_COUNTER();

	ChunkKeyToChunk.Reserve(1024);

	for (const TSharedRef<FVoxelRenderChunk>& RootChunk : RootChunks)
	{
		ChunkKeyToChunk.Add_EnsureNew(RootChunk->ChunkKey, RootChunk);
	}
}

FVoxelRenderTree::~FVoxelRenderTree()
{
	VOXEL_FUNCTION_COUNTER();

	// TODO Async

	// Clear references to not keep everything alive & destroy everything within the scope counter
	for (const TSharedRef<FVoxelRenderChunk>& RootChunk : RootChunks)
	{
		ReinterpretCastRef<TSharedPtr<FVoxelRenderChunk>>(ConstCast(RootChunk)).Reset();
	}

	ChunkKeyToChunk.Empty();
	CachedChunkKeysToRender.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderTree::Update(
	FVoxelRenderSubsystem& Subsystem,
	const FVoxelRenderInvokersContainer& InvokersContainer)
{
	VOXEL_FUNCTION_COUNTER();

	Traverse(Subsystem, InvokersContainer.CameraInvokers);
	Collapse(Subsystem, InvokersContainer.CameraInvokers);
	Subdivide(Subsystem, InvokersContainer.CameraInvokers);
	TraverseLODInvokers(InvokersContainer);
	SubdivideNeighbors(Subsystem);
	FinalizeTraversal(Subsystem);
}

void FVoxelRenderTree::DestroyAllRenderDatas(FVoxelRenderSubsystem& Subsystem)
{
	VOXEL_FUNCTION_COUNTER();

	for (const auto& It : ChunkKeyToChunk)
	{
		Subsystem.DestroyRenderData(It.Value->RenderData);
	}
}

TConstVoxelArrayView<FVoxelChunkKey> FVoxelRenderTree::GetChunkKeysToRender() const
{
	if (!ensure(CachedChunkKeysToRender))
	{
		return {};
	}

	return *CachedChunkKeysToRender;
}

FVoxelRenderChunk* FVoxelRenderTree::FindNeighbor(
	const FVoxelChunkKey& ChunkKey,
	const int32 DirectionX,
	const int32 DirectionY,
	const int32 DirectionZ) const
{
	checkVoxelSlow(-1 <= DirectionX && DirectionX <= 1);
	checkVoxelSlow(-1 <= DirectionY && DirectionY <= 1);
	checkVoxelSlow(-1 <= DirectionZ && DirectionZ <= 1);

	FVoxelChunkKey Neighbor = ChunkKey;
	{
		const int32 Step = 1 << ChunkKey.LOD;

		Neighbor.ChunkKey.X += DirectionX * Step;
		Neighbor.ChunkKey.Y += DirectionY * Step;
		Neighbor.ChunkKey.Z += DirectionZ * Step;
	}

	FVoxelChunkKey Parent = ChunkKey;

	while (true)
	{
		if (const TSharedPtr<FVoxelRenderChunk>* NeighborChunkPtr = ChunkKeyToChunk.Find(Neighbor))
		{
			FVoxelRenderChunk* NeighborChunk = NeighborChunkPtr->Get();

			if (NeighborChunk->Children.Num() == 0)
			{
				return NeighborChunk;
			}
			else
			{
				// Actual neighbor is one of NeighborChunk children and is higher res than us
				return nullptr;
			}
		}

		if (Neighbor.LOD == Depth)
		{
			// No neighbor or neighbor is higher res than us
			return nullptr;
		}

		checkVoxelSlow(Parent.LOD == Neighbor.LOD);

		if (Parent == Neighbor)
		{
			// No neighbor or neighbor is higher res than us: we walked up the tree until we reached back our own roots
			return nullptr;
		}

		Neighbor = Neighbor.GetParent();
		Parent = Parent.GetParent();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderTree::CollapseChunk(FVoxelRenderChunk& Chunk)
{
	ensure(Chunk.Children.Num() == 8);

	for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.Children)
	{
		if (Child->Children.Num() > 0)
		{
			CollapseChunk(*Child);
		}
		ensure(Child->Children.Num() == 0);

		ensure(ChunkKeyToChunk.Remove(Child->ChunkKey));
	}

	ensure(Chunk.ChildrenToDestroy.Num() == 0);
	Chunk.ChildrenToDestroy = MoveTemp(Chunk.Children);

	ensure(Chunk.Children.Num() == 0);
}

void FVoxelRenderTree::SubdivideChunk(FVoxelRenderChunk& Chunk)
{
	ensure(Chunk.Children.Num() == 0);

	if (Chunk.ChildrenToDestroy.Num() > 0)
	{
		// Previously collapsed chunk, reuse existing children
		Chunk.Children = MoveTemp(Chunk.ChildrenToDestroy);
	}
	else
	{
		FVoxelUtilities::SetNum(Chunk.Children, 8);

		for (int32 Index = 0; Index < 8; Index++)
		{
			Chunk.Children[Index] = MakeShared<FVoxelRenderChunk>(Chunk.ChunkKey.GetChild(Index));
		}
	}

	ensure(Chunk.Children.Num() == 8);

	for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.Children)
	{
		ensure(Child->Children.Num() == 0);
		ChunkKeyToChunk.Add_EnsureNew(Child->ChunkKey, Child);
	}
}

void FVoxelRenderTree::DestroyChunk(
	FVoxelRenderSubsystem& Subsystem,
	FVoxelRenderChunk& Chunk)
{
	ensure(!ChunkKeyToChunk.Contains(Chunk.ChunkKey));
	ensure(Chunk.Children.Num() == 0);

	Subsystem.DestroyRenderData(Chunk.RenderData);

	for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.ChildrenToDestroy)
	{
		DestroyChunk(Subsystem, *Child);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderTree::Traverse(
	const FVoxelRenderSubsystem& Subsystem,
	const TVoxelArray<FVector>& CameraInvokers)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelScreenSizeHelper ScreenSizeHelper = FVoxelScreenSizeHelper(Subsystem.GetConfig());

	TVoxelArray<FVoxelRenderChunk*> ChunksToTraverse;
	ChunksToTraverse.Reserve(8 * (Depth + 1));

	for (const TSharedRef<FVoxelRenderChunk>& RootChunk : RootChunks)
	{
		ChunksToTraverse.Add_EnsureNoGrow(&RootChunk.Get());
	}

	while (ChunksToTraverse.Num() > 0)
	{
		FVoxelRenderChunk& Chunk = *ChunksToTraverse.Pop();
		ensure(Chunk.ChildrenToDestroy.Num() == 0);

		if (Chunk.ChunkKey.LOD == 0)
		{
			continue;
		}

		const double ChunkQuality = ScreenSizeHelper.GetChunkQuality(CameraInvokers, Chunk.ChunkKey);

		if (ChunkQuality > ScreenSizeHelper.MaxQuality &&
			Chunk.Children.Num() > 0)
		{
			// Collapse

			CollapseChunk(Chunk);
		}
		else if (
			ChunkQuality < ScreenSizeHelper.MinQuality &&
			Chunk.Children.Num() == 0)
		{
			// Subdivide

			if (ChunkKeyToChunk.Num() > GVoxelMaxRenderChunks)
			{
				VOXEL_MESSAGE(Error, "voxel.MaxRenderChunks reached");
			}
			else
			{
				SubdivideChunk(Chunk);
			}
		}

		for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.Children)
		{
			ChunksToTraverse.Add_EnsureNoGrow(Child.Get());
		}
	}
}

void FVoxelRenderTree::Collapse(
	const FVoxelRenderSubsystem& Subsystem,
	const TVoxelArray<FVector>& CameraInvokers)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelScreenSizeHelper ScreenSizeHelper = FVoxelScreenSizeHelper(Subsystem.GetConfig());

	TVoxelArray<FVoxelChunkKey> ChunkKeysToCollapse;
	TVoxelMap<FVoxelChunkKey, int32> ChunkKeyToNumInvalidatedChildren;
	ChunkKeyToNumInvalidatedChildren.Reserve(ChunkKeyToChunk.Num());

	while (true)
	{
		ChunkKeyToNumInvalidatedChildren.Reset_KeepHashSize();

		int32 NumMeshesToCompute = 0;

		for (const auto& It : ChunkKeyToChunk)
		{
			FVoxelRenderChunk& Chunk = *It.Value;
			if (Chunk.Children.Num() > 0)
			{
				continue;
			}

			if (Chunk.Mesh.IsSet() &&
				!Chunk.bMeshInvalidated)
			{
				continue;
			}

			NumMeshesToCompute++;

			FVoxelChunkKey ChunkKey = Chunk.ChunkKey;

			while (true)
			{
				const FVoxelChunkKey ParentChunkKey = ChunkKey.GetParent();
				if (!ChunkKeyToChunk.Contains(ParentChunkKey) ||
					ScreenSizeHelper.GetChunkQuality(CameraInvokers, ParentChunkKey) < ScreenSizeHelper.MinQuality)
				{
					break;
				}

				ChunkKeyToNumInvalidatedChildren.FindOrAdd(ParentChunkKey)++;

				ChunkKey = ParentChunkKey;
			}
		}

		ChunkKeysToCollapse.Reset(ChunkKeyToNumInvalidatedChildren.Num());

		for (const auto& It : ChunkKeyToNumInvalidatedChildren)
		{
			if (It.Value == 1)
			{
				// Not worth collapsing
				continue;
			}

			ChunkKeysToCollapse.Add(It.Key);
		}

		if (ChunkKeysToCollapse.Num() == 0)
		{
			break;
		}

		{
			VOXEL_SCOPE_COUNTER("Sort");

			ChunkKeysToCollapse.Sort([&](const FVoxelChunkKey& ChunkKeyA, const FVoxelChunkKey& ChunkKeyB)
			{
				return
					ScreenSizeHelper.GetChunkQuality(CameraInvokers, ChunkKeyA) <
					ScreenSizeHelper.GetChunkQuality(CameraInvokers, ChunkKeyB);
			});
		}

		while (
			NumMeshesToCompute > GVoxelMaxChunksToProcessPerPass &&
			ChunkKeysToCollapse.Num() > 0)
		{
			const FVoxelChunkKey ChunkKey = ChunkKeysToCollapse.Pop();
			FVoxelRenderChunk& Chunk = *ChunkKeyToChunk[ChunkKey];

			CollapseChunk(Chunk);

			// -1 because we still need to compute the parent
			NumMeshesToCompute -= ChunkKeyToNumInvalidatedChildren[ChunkKey] - 1;
		}

		if (NumMeshesToCompute <= GVoxelMaxChunksToProcessPerPass)
		{
			break;
		}
	}
}

void FVoxelRenderTree::Subdivide(
	const FVoxelRenderSubsystem& Subsystem,
	const TVoxelArray<FVector>& CameraInvokers)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelScreenSizeHelper ScreenSizeHelper = FVoxelScreenSizeHelper(Subsystem.GetConfig());

	int32 NumMeshesToCompute = 0;

	TVoxelArray<FVoxelChunkKey> ChunkKeysToSubdivide;
	ChunkKeysToSubdivide.Reserve(ChunkKeyToChunk.Num());

	for (const auto& It : ChunkKeyToChunk)
	{
		FVoxelRenderChunk& Chunk = *It.Value;
		if (Chunk.Children.Num() > 0)
		{
			continue;
		}

		if (Chunk.Mesh.IsSet() &&
			!Chunk.bMeshInvalidated)
		{
			if (Chunk.ChunkKey.LOD > 0 &&
				ScreenSizeHelper.GetChunkQuality(CameraInvokers, Chunk.ChunkKey) <= ScreenSizeHelper.MaxQuality)
			{
				ChunkKeysToSubdivide.Add(Chunk.ChunkKey);
			}

			continue;
		}

		NumMeshesToCompute++;
	}

	{
		VOXEL_SCOPE_COUNTER("Sort");

		ChunkKeysToSubdivide.Sort([&](const FVoxelChunkKey& ChunkKeyA, const FVoxelChunkKey& ChunkKeyB)
		{
			return
				ScreenSizeHelper.GetChunkQuality(CameraInvokers, ChunkKeyA) >
				ScreenSizeHelper.GetChunkQuality(CameraInvokers, ChunkKeyB);
		});
	}

	while (
		NumMeshesToCompute < GVoxelMaxChunksToProcessPerPass &&
		ChunkKeysToSubdivide.Num() > 0)
	{
		const FVoxelChunkKey ChunkKey = ChunkKeysToSubdivide.Pop();
		FVoxelRenderChunk& Chunk = *ChunkKeyToChunk[ChunkKey];

		SubdivideChunk(Chunk);

		NumMeshesToCompute--;

		for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.Children)
		{
			checkVoxelSlow(Child->Children.Num() == 0);

			if (!Child->Mesh.IsSet() ||
				Child->bMeshInvalidated)
			{
				NumMeshesToCompute++;
			}
		}
	}
}

void FVoxelRenderTree::TraverseLODInvokers(const FVoxelRenderInvokersContainer& LODInvokers)
{
	if (LODInvokers.LODVolumes.Num() == 0)
	{
		return;
	}

	const auto ShouldSubdivideChunk = [&](const FVoxelChunkKey& ChunkKey, int32& OutTargetLOD) -> bool
	{
		int32 TargetLOD = ChunkKey.LOD;
		const FVoxelBox ChunkBounds = ChunkKey.GetChunkKeyBounds().ToVoxelBox();
		LODInvokers.Tree.TraverseBounds(FVoxelFastBox(ChunkBounds), [&](const int32 Index)
		{
			const FVoxelLODVolume& Invoker = LODInvokers.LODVolumes[Index];
			if (ChunkKey.LOD <= Invoker.MinLOD)
			{
				return;
			}

			switch (Invoker.Type)
			{
			case EVoxelLODInvokerType::Sphere:
			{
				if (ChunkBounds.SquaredDistanceToPoint(Invoker.Position) > Invoker.Radius * Invoker.Radius)
				{
					return;
				}

				break;
			}
			case EVoxelLODInvokerType::Box:
			{
				const FVector AABBCenter = ChunkBounds.GetCenter();
				const FVector AABBExtent = ChunkBounds.GetExtent();

				const FVector LocalAABBCenter = Invoker.InvRotation.RotateVector(AABBCenter - Invoker.Position);

				// No more Abs() calls needed!
				const FMatrix& AbsRot = Invoker.AbsRotationMatrix;
				const FVector RotatedExtent = FVector(
					AbsRot.M[0][0] * AABBExtent.X + AbsRot.M[0][1] * AABBExtent.Y + AbsRot.M[0][2] * AABBExtent.Z,
					AbsRot.M[1][0] * AABBExtent.X + AbsRot.M[1][1] * AABBExtent.Y + AbsRot.M[1][2] * AABBExtent.Z,
					AbsRot.M[2][0] * AABBExtent.X + AbsRot.M[2][1] * AABBExtent.Y + AbsRot.M[2][2] * AABBExtent.Z
				);

				if (FMath::Abs(LocalAABBCenter.X) > Invoker.Extents.X + RotatedExtent.X ||
					FMath::Abs(LocalAABBCenter.Y) > Invoker.Extents.Y + RotatedExtent.Y ||
					FMath::Abs(LocalAABBCenter.Z) > Invoker.Extents.Z + RotatedExtent.Z)
				{
					return;
				}

				break;
			}
			default: ensure(false); return;
			}

			TargetLOD = FMath::Min(TargetLOD, Invoker.MinLOD);
		});

		OutTargetLOD = TargetLOD;
		return TargetLOD < ChunkKey.LOD;
	};

	struct FData
	{
		FVoxelChunkKey ChunkKey;
		int32 TargetLOD;
	};
	TVoxelArray<FData> ChunkKeysToSubdivide;
	ChunkKeysToSubdivide.Reserve(ChunkKeyToChunk.Num());

	for (const auto& It : ChunkKeyToChunk)
	{
		FVoxelRenderChunk& Chunk = *It.Value;
		if (Chunk.Children.Num() > 0)
		{
			continue;
		}

		int32 TargetLOD = -1;
		if (!ShouldSubdivideChunk(It.Key, TargetLOD))
		{
			continue;
		}

		ChunkKeysToSubdivide.Add({ It.Key, TargetLOD });
	}

	while (ChunkKeysToSubdivide.Num() > 0)
	{
		TVoxelArray<FData> NewChunkKeysToSubdivide;
		NewChunkKeysToSubdivide.Reserve(ChunkKeysToSubdivide.Num());

		for (const FData& Data : ChunkKeysToSubdivide)
		{
			FVoxelRenderChunk& Chunk = *ChunkKeyToChunk[Data.ChunkKey];
			SubdivideChunk(Chunk);

			if (Data.ChunkKey.LOD - 1 <= Data.TargetLOD)
			{
				continue;
			}

			for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.Children)
			{
				int32 TargetLOD = -1;
				if (!ShouldSubdivideChunk(Child->ChunkKey, TargetLOD))
				{
					continue;
				}

				NewChunkKeysToSubdivide.Add({ Child->ChunkKey, TargetLOD });
			}
		}

		ChunkKeysToSubdivide = MoveTemp(NewChunkKeysToSubdivide);
	}
}

void FVoxelRenderTree::SubdivideNeighbors(FVoxelRenderSubsystem& Subsystem)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelRenderNeighborSubdivider Subdivider(Subsystem, *this, RootChunks);
	Subdivider.Traverse();
	Subdivider.ProcessNewChunks();

	bHasReachedMaxChunks = Subdivider.HasReachedMaxChunks.Get();

	if (bHasReachedMaxChunks)
	{
		// Don't check
		return;
	}

#if VOXEL_DEBUG
	VOXEL_SCOPE_COUNTER("Checks");

	for (const auto& It : ChunkKeyToChunk)
	{
		if (It.Value->Children.Num() > 0)
		{
			continue;
		}

		for (int32 X = -1; X <= 1; X++)
		{
			for (int32 Y = -1; Y <= 1; Y++)
			{
				for (int32 Z = -1; Z <= 1; Z++)
				{
					const FVoxelRenderChunk* NeighborChunk = FindNeighbor(It.Key, X, Y, Z);
					if (!NeighborChunk)
					{
						continue;
					}

					ensure(NeighborChunk->ChunkKey.LOD - It.Key.LOD <= FVoxelMesher::MaxRelativeLOD);
				}
			}
		}
	}
#endif
}

void FVoxelRenderTree::FinalizeTraversal(FVoxelRenderSubsystem& Subsystem)
{
	VOXEL_FUNCTION_COUNTER();

	if (!CachedChunkKeysToRender)
	{
		CachedChunkKeysToRender.Emplace();
	}

	TVoxelArray<FVoxelChunkKey>& ChunkKeysToRender = *CachedChunkKeysToRender;
	ChunkKeysToRender.Reset();
	ChunkKeysToRender.Reserve(ChunkKeyToChunk.Num());

	for (const auto& It : ChunkKeyToChunk)
	{
		FVoxelRenderChunk& Chunk = *It.Value;

		for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.ChildrenToDestroy)
		{
			DestroyChunk(Subsystem, *Child);
		}
		Chunk.ChildrenToDestroy.Empty();

		if (Chunk.Children.Num() == 0)
		{
			ChunkKeysToRender.Add(It.Key);
		}
		else
		{
			Chunk.Mesh.Reset();
			Chunk.MeshDependencyTracker = {};
			Subsystem.DestroyRenderData(Chunk.RenderData);
		}
	}

#if VOXEL_DEBUG
	{
		VOXEL_SCOPE_COUNTER("Checks");

		TVoxelArray<FVoxelRenderChunk*> ChunksToTraverse;
		ChunksToTraverse.Reserve(8 * (Depth + 1));

		for (const TSharedRef<FVoxelRenderChunk>& RootChunk : RootChunks)
		{
			ChunksToTraverse.Add_EnsureNoGrow(&RootChunk.Get());
		}

		TVoxelSet<FVoxelRenderChunk*> ValidChunks;
		ValidChunks.Reserve(ChunkKeyToChunk.Num());

		while (ChunksToTraverse.Num() > 0)
		{
			FVoxelRenderChunk& Chunk = *ChunksToTraverse.Pop();
			ensure(ChunkKeyToChunk.Contains(Chunk.ChunkKey));
			ensure(Chunk.ChildrenToDestroy.Num() == 0);

			ValidChunks.Add(&Chunk);

			for (const TSharedPtr<FVoxelRenderChunk>& Child : Chunk.Children)
			{
				ChunksToTraverse.Add_EnsureNoGrow(Child.Get());
			}
		}

		for (const auto& It : ChunkKeyToChunk)
		{
			ensure(It.Value->ChunkKey == It.Key);
			ensure(ValidChunks.Contains(It.Value.Get()));
		}

		ensure(ChunkKeyToChunk.Num() == ValidChunks.Num());
	}
#endif
}