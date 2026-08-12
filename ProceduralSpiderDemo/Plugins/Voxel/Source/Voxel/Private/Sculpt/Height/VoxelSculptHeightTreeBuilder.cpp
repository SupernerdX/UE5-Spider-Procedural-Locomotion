// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeightTreeBuilder.h"

TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>> FVoxelSculptHeightTreeBuilder::Build(
	const TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>>& PreviousKeyToFarChunk,
	const FVoxelIntBox2D& BoundsToReplace,
	const TVoxelMap<FIntPoint, FVoxelHeightChunkData>& KeyToNewChunkData,
	const float TargetPrecision)
{
	VOXEL_FUNCTION_COUNTER();

	for (const auto& It : KeyToNewChunkData)
	{
		checkVoxelSlow(BoundsToReplace.Contains(FVoxelIntBox2D(It.Key).Scale(ChunkSize)));
	}

	TVoxelChunkedArray<FIntPoint> FarChunkKeys;
	{
		VOXEL_SCOPE_COUNTER("Build FarChunkKeys");

		const FVoxelIntBox2D FarChunkKeyBounds = BoundsToReplace.DivideBigger(ChunkSize * ChunkSize * ChunkSize);

		FarChunkKeyBounds.Iterate([&](const FIntPoint& Key)
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

	TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>> KeyToFarChunk;
	KeyToFarChunk.Reserve(FarChunkKeys.Num());

	for (const FIntPoint& FarKey : FarChunkKeys)
	{
		const TVoxelBulkPtr<FVoxelHeightFarChunk> NewFarChunk = BuildFarChunk(
			BoundsToReplace,
			KeyToNewChunkData,
			FarKey,
			PreviousKeyToFarChunk.FindRef(FarKey),
			TargetPrecision);

		if (!NewFarChunk)
		{
			continue;
		}

		KeyToFarChunk.Add_EnsureNew(FarKey, NewFarChunk);
	}

	KeyToFarChunk.KeySort();

	return KeyToFarChunk;
}

TVoxelBulkPtr<FVoxelHeightFarChunk> FVoxelSculptHeightTreeBuilder::BuildFarChunk(
	const FVoxelIntBox2D& BoundsToReplace,
	const TVoxelMap<FIntPoint, FVoxelHeightChunkData>& KeyToNewChunkData,
	const FIntPoint& FarChunkKey,
	const TVoxelBulkPtr<FVoxelHeightFarChunk>& PreviousFarChunkPtr,
	const float TargetPrecision)
{
	VOXEL_FUNCTION_COUNTER();

	if (!FVoxelIntBox2D(FarChunkKey).Scale(ChunkSize * ChunkSize * ChunkSize).Intersects(BoundsToReplace))
	{
		return PreviousFarChunkPtr;
	}

	if (PreviousFarChunkPtr &&
		!ensure(PreviousFarChunkPtr.IsLoaded()))
	{
		return PreviousFarChunkPtr;
	}

	const FVoxelHeightFarChunk* PreviousFarChunk = nullptr;
	if (PreviousFarChunkPtr)
	{
		PreviousFarChunk = &PreviousFarChunkPtr.Get();
	}

	TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightMidChunk>> KeyToMidChunk;
	if (PreviousFarChunk)
	{
		KeyToMidChunk.Reserve(PreviousFarChunk->KeyToMidChunk.Num());
	}

	FVoxelIntBox2D(0, ChunkSize).Iterate([&](const FIntPoint& ChunkKey)
	{
		TVoxelBulkPtr<FVoxelHeightMidChunk> PreviousMidChunk;
		if (PreviousFarChunk)
		{
			PreviousMidChunk = PreviousFarChunk->KeyToMidChunk.FindRef(ChunkKey);
		}

		const TVoxelBulkPtr<FVoxelHeightMidChunk> NewMidChunk = BuildMidChunk(
			BoundsToReplace,
			KeyToNewChunkData,
			FarChunkKey * ChunkSize + ChunkKey,
			PreviousMidChunk,
			TargetPrecision);

		if (!NewMidChunk)
		{
			return;
		}

		KeyToMidChunk.Add_EnsureNew(ChunkKey, NewMidChunk);
	});

	return FVoxelHeightFarChunk::Create(MoveTemp(KeyToMidChunk), TargetPrecision);
}

TVoxelBulkPtr<FVoxelHeightMidChunk> FVoxelSculptHeightTreeBuilder::BuildMidChunk(
	const FVoxelIntBox2D& BoundsToReplace,
	const TVoxelMap<FIntPoint, FVoxelHeightChunkData>& KeyToNewChunkData,
	const FIntPoint& MidChunkKey,
	const TVoxelBulkPtr<FVoxelHeightMidChunk>& PreviousMidChunkPtr,
	const float TargetPrecision)
{
	VOXEL_FUNCTION_COUNTER();

	if (!FVoxelIntBox2D(MidChunkKey).Scale(ChunkSize * ChunkSize).Intersects(BoundsToReplace))
	{
		return PreviousMidChunkPtr;
	}

	if (PreviousMidChunkPtr &&
		!ensure(PreviousMidChunkPtr.IsLoaded()))
	{
		return PreviousMidChunkPtr;
	}

	const FVoxelHeightMidChunk* PreviousMidChunk = nullptr;
	if (PreviousMidChunkPtr)
	{
		PreviousMidChunk = &PreviousMidChunkPtr.Get();
	}

	TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightNearChunk>> KeyToNearChunk;
	if (PreviousMidChunk)
	{
		KeyToNearChunk.Reserve(PreviousMidChunk->KeyToNearChunk.Num());
	}

	FVoxelIntBox2D(0, ChunkSize).Iterate([&](const FIntPoint& ChunkKey)
	{
		TVoxelBulkPtr<FVoxelHeightNearChunk> PreviousNearChunk;
		if (PreviousMidChunk)
		{
			PreviousNearChunk = PreviousMidChunk->KeyToNearChunk.FindRef(ChunkKey);
		}

		const TVoxelBulkPtr<FVoxelHeightNearChunk> NewNearChunk = BuildNearChunk(
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

	return FVoxelHeightMidChunk::Create(MoveTemp(KeyToNearChunk), TargetPrecision);
}

TVoxelBulkPtr<FVoxelHeightNearChunk> FVoxelSculptHeightTreeBuilder::BuildNearChunk(
	const FVoxelIntBox2D& BoundsToReplace,
	const TVoxelMap<FIntPoint, FVoxelHeightChunkData>& KeyToNewChunkData,
	const FIntPoint& NearChunkKey,
	const TVoxelBulkPtr<FVoxelHeightNearChunk>& PreviousNearChunkPtr)
{
	VOXEL_FUNCTION_COUNTER();

	if (!FVoxelIntBox2D(NearChunkKey).Scale(ChunkSize).Intersects(BoundsToReplace))
	{
		return PreviousNearChunkPtr;
	}

	const FVoxelHeightChunkData* ChunkData = KeyToNewChunkData.Find(NearChunkKey);
	if (!ChunkData)
	{
		return {};
	}

	const TSharedRef<FVoxelHeightNearChunk> Result = MakeShared<FVoxelHeightNearChunk>();
	Result->ChunkData = *ChunkData;
	return TVoxelBulkPtr<FVoxelHeightNearChunk>(Result);
}
