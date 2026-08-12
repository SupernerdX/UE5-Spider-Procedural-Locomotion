// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeightInvalidationBuilder.h"
#include "VoxelAABBTree2D.h"

TSharedRef<const FVoxelAABBTree2D> FVoxelSculptHeightInvalidationBuilder::Create(
	const TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>>& KeyToFarChunkA,
	const TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>>& KeyToFarChunkB)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelSculptHeightInvalidationBuilder Builder;

	FVoxelUtilities::IterateSortedMaps(
		KeyToFarChunkA,
		KeyToFarChunkB,
		[&](
			const FIntPoint& Key,
			const TVoxelBulkPtr<FVoxelHeightFarChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelHeightFarChunk>* ChunkBPtr)
		{
			Builder.Traverse(
				Key,
				ChunkAPtr ? *ChunkAPtr : nullptr,
				ChunkBPtr ? *ChunkBPtr : nullptr);
		});

	return FVoxelAABBTree2D::Create(Builder.BoundsToInvalidate);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptHeightInvalidationBuilder::Traverse(
	const FIntPoint& ChunkKey,
	const TVoxelBulkPtr<FVoxelHeightFarChunk>& ChunkA,
	const TVoxelBulkPtr<FVoxelHeightFarChunk>& ChunkB)
{
	ensure(ChunkA || ChunkB);

	if (!ChunkA ||
		!ChunkB)
	{
		BoundsToInvalidate.Add(FVoxelIntBox2D(ChunkKey).Scale(ChunkSize * ChunkSize * ChunkSize));
		return;
	}

	if (ChunkA.GetHash() == ChunkB.GetHash())
	{
		return;
	}

	if (!ChunkA.IsLoaded() ||
		!ChunkB.IsLoaded())
	{
		// Can't traverse deeper, have to fully invalidate
		BoundsToInvalidate.Add(FVoxelIntBox2D(ChunkKey).Scale(ChunkSize * ChunkSize * ChunkSize));
		return;
	}

	FVoxelUtilities::IterateSortedMaps(
		ChunkA->KeyToMidChunk,
		ChunkB->KeyToMidChunk,
		[&](
			const FVoxelHeightChunkKey& Key,
			const TVoxelBulkPtr<FVoxelHeightMidChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelHeightMidChunk>* ChunkBPtr)
		{
			Traverse(
				ChunkKey * ChunkSize + Key.ToPoint(),
				ChunkAPtr ? *ChunkAPtr : nullptr,
				ChunkBPtr ? *ChunkBPtr : nullptr);
		});
}

void FVoxelSculptHeightInvalidationBuilder::Traverse(
	const FIntPoint& ChunkKey,
	const TVoxelBulkPtr<FVoxelHeightMidChunk>& ChunkA,
	const TVoxelBulkPtr<FVoxelHeightMidChunk>& ChunkB)
{
	ensure(ChunkA || ChunkB);

	if (!ChunkA ||
		!ChunkB)
	{
		BoundsToInvalidate.Add(FVoxelIntBox2D(ChunkKey).Scale(ChunkSize * ChunkSize));
		return;
	}

	if (ChunkA.GetHash() == ChunkB.GetHash())
	{
		return;
	}

	if (!ChunkA.IsLoaded() ||
		!ChunkB.IsLoaded())
	{
		// Can't traverse deeper, have to fully invalidate
		BoundsToInvalidate.Add(FVoxelIntBox2D(ChunkKey).Scale(ChunkSize * ChunkSize));
		return;
	}

	FVoxelUtilities::IterateSortedMaps(
		ChunkA->KeyToNearChunk,
		ChunkB->KeyToNearChunk,
		[&](
			const FVoxelHeightChunkKey& Key,
			const TVoxelBulkPtr<FVoxelHeightNearChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelHeightNearChunk>* ChunkBPtr)
		{
			Traverse(
				ChunkKey * ChunkSize + Key.ToPoint(),
				ChunkAPtr ? *ChunkAPtr : nullptr,
				ChunkBPtr ? *ChunkBPtr : nullptr);
		});
}

void FVoxelSculptHeightInvalidationBuilder::Traverse(
	const FIntPoint& ChunkKey,
	const TVoxelBulkPtr<FVoxelHeightNearChunk>& ChunkA,
	const TVoxelBulkPtr<FVoxelHeightNearChunk>& ChunkB)
{
	ensure(ChunkA || ChunkB);

	if (ChunkA &&
		ChunkB &&
		ChunkA.GetHash() == ChunkB.GetHash())
	{
		return;
	}

	BoundsToInvalidate.Add(FVoxelIntBox2D(ChunkKey).Scale(ChunkSize));
}