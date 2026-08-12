// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolumeInvalidationBuilder.h"
#include "VoxelAABBTree.h"

TSharedRef<const FVoxelAABBTree> FVoxelSculptVolumeInvalidationBuilder::Create(
	const TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>>& KeyToFarChunkA,
	const TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>>& KeyToFarChunkB)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelSculptVolumeInvalidationBuilder Builder;

	FVoxelUtilities::IterateSortedMaps(
		KeyToFarChunkA,
		KeyToFarChunkB,
		[&](
			const FIntVector& Key,
			const TVoxelBulkPtr<FVoxelVolumeFarChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelVolumeFarChunk>* ChunkBPtr)
		{
			Builder.Traverse(
				Key,
				ChunkAPtr ? *ChunkAPtr : nullptr,
				ChunkBPtr ? *ChunkBPtr : nullptr);
		});

	return FVoxelAABBTree::Create(Builder.BoundsToInvalidate);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSculptVolumeInvalidationBuilder::Traverse(
	const FIntVector& ChunkKey,
	const TVoxelBulkPtr<FVoxelVolumeFarChunk>& ChunkA,
	const TVoxelBulkPtr<FVoxelVolumeFarChunk>& ChunkB)
{
	ensure(ChunkA || ChunkB);

	if (!ChunkA ||
		!ChunkB)
	{
		BoundsToInvalidate.Add(FVoxelIntBox(ChunkKey).Scale(ChunkSize * ChunkSize * ChunkSize));
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
		BoundsToInvalidate.Add(FVoxelIntBox(ChunkKey).Scale(ChunkSize * ChunkSize * ChunkSize));
		return;
	}

	FVoxelUtilities::IterateSortedMaps(
		ChunkA->KeyToMidChunk,
		ChunkB->KeyToMidChunk,
		[&](
			const FVoxelVolumeChunkKey& Key,
			const TVoxelBulkPtr<FVoxelVolumeMidChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelVolumeMidChunk>* ChunkBPtr)
		{
			Traverse(
				ChunkKey * ChunkSize + Key.ToVector(),
				ChunkAPtr ? *ChunkAPtr : nullptr,
				ChunkBPtr ? *ChunkBPtr : nullptr);
		});
}

void FVoxelSculptVolumeInvalidationBuilder::Traverse(
	const FIntVector& ChunkKey,
	const TVoxelBulkPtr<FVoxelVolumeMidChunk>& ChunkA,
	const TVoxelBulkPtr<FVoxelVolumeMidChunk>& ChunkB)
{
	ensure(ChunkA || ChunkB);

	if (!ChunkA ||
		!ChunkB)
	{
		BoundsToInvalidate.Add(FVoxelIntBox(ChunkKey).Scale(ChunkSize * ChunkSize));
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
		BoundsToInvalidate.Add(FVoxelIntBox(ChunkKey).Scale(ChunkSize * ChunkSize));
		return;
	}

	FVoxelUtilities::IterateSortedMaps(
		ChunkA->KeyToNearChunk,
		ChunkB->KeyToNearChunk,
		[&](
			const FVoxelVolumeChunkKey& Key,
			const TVoxelBulkPtr<FVoxelVolumeNearChunk>* ChunkAPtr,
			const TVoxelBulkPtr<FVoxelVolumeNearChunk>* ChunkBPtr)
		{
			Traverse(
				ChunkKey * ChunkSize + Key.ToVector(),
				ChunkAPtr ? *ChunkAPtr : nullptr,
				ChunkBPtr ? *ChunkBPtr : nullptr);
		});
}

void FVoxelSculptVolumeInvalidationBuilder::Traverse(
	const FIntVector& ChunkKey,
	const TVoxelBulkPtr<FVoxelVolumeNearChunk>& ChunkA,
	const TVoxelBulkPtr<FVoxelVolumeNearChunk>& ChunkB)
{
	ensure(ChunkA || ChunkB);

	if (ChunkA &&
		ChunkB &&
		ChunkA.GetHash() == ChunkB.GetHash())
	{
		return;
	}

	BoundsToInvalidate.Add(FVoxelIntBox(ChunkKey).Scale(ChunkSize));
}