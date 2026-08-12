// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolumeTreeBuilder.h"

TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>> FVoxelSculptVolumeTreeBuilder::Build(
	const TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>>& PreviousKeyToFarChunk,
	const FVoxelIntBox& BoundsToReplace,
	const TVoxelMap<FIntVector, FVoxelVolumeChunkData>& KeyToNewChunkData,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();

	for (const auto& It : KeyToNewChunkData)
	{
		checkVoxelSlow(BoundsToReplace.Contains(FVoxelIntBox(It.Key).Scale(ChunkSize)));
	}

	TVoxelChunkedArray<FIntVector> FarChunkKeys;
	{
		VOXEL_SCOPE_COUNTER("Build FarChunkKeys");

		const FVoxelIntBox FarChunkKeyBounds = BoundsToReplace.DivideBigger(ChunkSize * ChunkSize * ChunkSize);

		FarChunkKeyBounds.Iterate([&](const FIntVector& Key)
		{
			FarChunkKeys.Add(Key);
		});

		for (const auto& It : PreviousKeyToFarChunk)
		{
			if (!FarChunkKeyBounds.Contains(It.Key))
			{
				FarChunkKeys.Add(It.Key);
			}
		}
	}

	TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>> KeyToFarChunk;
	KeyToFarChunk.Reserve(FarChunkKeys.Num());

	for (const FIntVector& FarKey : FarChunkKeys)
	{
		const TVoxelBulkPtr<FVoxelVolumeFarChunk> NewFarChunk = BuildFarChunk(
			BoundsToReplace,
			KeyToNewChunkData,
			FarKey,
			PreviousKeyToFarChunk.FindRef(FarKey),
			MaxErrorPercentage);

		if (!NewFarChunk)
		{
			continue;
		}

		KeyToFarChunk.Add_EnsureNew(FarKey, NewFarChunk);
	}

	KeyToFarChunk.KeySort();

	return KeyToFarChunk;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelBulkPtr<FVoxelVolumeFarChunk> FVoxelSculptVolumeTreeBuilder::BuildFarChunk(
	const FVoxelIntBox& BoundsToReplace,
	const TVoxelMap<FIntVector, FVoxelVolumeChunkData>& KeyToNewChunkData,
	const FIntVector& FarChunkKey,
	const TVoxelBulkPtr<FVoxelVolumeFarChunk>& PreviousFarChunkPtr,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();

	if (!FVoxelIntBox(FarChunkKey).Scale(ChunkSize * ChunkSize * ChunkSize).Intersects(BoundsToReplace))
	{
		return PreviousFarChunkPtr;
	}

	if (PreviousFarChunkPtr &&
		!ensure(PreviousFarChunkPtr.IsLoaded()))
	{
		return PreviousFarChunkPtr;
	}

	const FVoxelVolumeFarChunk* PreviousFarChunk = nullptr;
	if (PreviousFarChunkPtr)
	{
		PreviousFarChunk = &PreviousFarChunkPtr.Get();
	}

	TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeMidChunk>> KeyToMidChunk;
	if (PreviousFarChunk)
	{
		KeyToMidChunk.Reserve(PreviousFarChunk->KeyToMidChunk.Num());
	}

	FVoxelIntBox(0, ChunkSize).Iterate([&](const FIntVector& ChunkKey)
	{
		TVoxelBulkPtr<FVoxelVolumeMidChunk> PreviousMidChunk;
		if (PreviousFarChunk)
		{
			PreviousMidChunk = PreviousFarChunk->KeyToMidChunk.FindRef(ChunkKey);
		}

		const TVoxelBulkPtr<FVoxelVolumeMidChunk> NewMidChunk = BuildMidChunk(
			BoundsToReplace,
			KeyToNewChunkData,
			FarChunkKey * ChunkSize + ChunkKey,
			PreviousMidChunk,
			MaxErrorPercentage);

		if (!NewMidChunk)
		{
			return;
		}

		KeyToMidChunk.Add_EnsureNew(ChunkKey, NewMidChunk);
	});

	return FVoxelVolumeFarChunk::Create(MoveTemp(KeyToMidChunk), MaxErrorPercentage);
}

TVoxelBulkPtr<FVoxelVolumeMidChunk> FVoxelSculptVolumeTreeBuilder::BuildMidChunk(
	const FVoxelIntBox& BoundsToReplace,
	const TVoxelMap<FIntVector, FVoxelVolumeChunkData>& KeyToNewChunkData,
	const FIntVector& MidChunkKey,
	const TVoxelBulkPtr<FVoxelVolumeMidChunk>& PreviousMidChunkPtr,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();

	if (!FVoxelIntBox(MidChunkKey).Scale(ChunkSize * ChunkSize).Intersects(BoundsToReplace))
	{
		return PreviousMidChunkPtr;
	}

	if (PreviousMidChunkPtr &&
		!ensure(PreviousMidChunkPtr.IsLoaded()))
	{
		return PreviousMidChunkPtr;
	}

	const FVoxelVolumeMidChunk* PreviousMidChunk = nullptr;
	if (PreviousMidChunkPtr)
	{
		PreviousMidChunk = &PreviousMidChunkPtr.Get();
	}

	TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeNearChunk>> KeyToNearChunk;
	if (PreviousMidChunk)
	{
		KeyToNearChunk.Reserve(PreviousMidChunk->KeyToNearChunk.Num());
	}

	FVoxelIntBox(0, ChunkSize).Iterate([&](const FIntVector& ChunkKey)
	{
		TVoxelBulkPtr<FVoxelVolumeNearChunk> PreviousNearChunk;
		if (PreviousMidChunk)
		{
			PreviousNearChunk = PreviousMidChunk->KeyToNearChunk.FindRef(ChunkKey);
		}

		const TVoxelBulkPtr<FVoxelVolumeNearChunk> NewNearChunk = BuildNearChunk(
			BoundsToReplace,
			KeyToNewChunkData,
			MidChunkKey * ChunkSize + ChunkKey,
			PreviousNearChunk);

		if (!NewNearChunk)
		{
			return;
		}

		KeyToNearChunk.Add_EnsureNew(ChunkKey, NewNearChunk);
	});

	return FVoxelVolumeMidChunk::Create(MoveTemp(KeyToNearChunk), MaxErrorPercentage);
}

TVoxelBulkPtr<FVoxelVolumeNearChunk> FVoxelSculptVolumeTreeBuilder::BuildNearChunk(
	const FVoxelIntBox& BoundsToReplace,
	const TVoxelMap<FIntVector, FVoxelVolumeChunkData>& KeyToNewChunkData,
	const FIntVector& NearChunkKey,
	const TVoxelBulkPtr<FVoxelVolumeNearChunk>& PreviousNearChunkPtr)
{
	VOXEL_FUNCTION_COUNTER();

	if (!FVoxelIntBox(NearChunkKey).Scale(ChunkSize).Intersects(BoundsToReplace))
	{
		return PreviousNearChunkPtr;
	}

	const FVoxelVolumeChunkData* ChunkData = KeyToNewChunkData.Find(NearChunkKey);
	if (!ChunkData)
	{
		return {};
	}

	const TSharedRef<FVoxelVolumeNearChunk> Result = MakeShared<FVoxelVolumeNearChunk>();
	Result->ChunkData = *ChunkData;
	return TVoxelBulkPtr<FVoxelVolumeNearChunk>(Result);
}