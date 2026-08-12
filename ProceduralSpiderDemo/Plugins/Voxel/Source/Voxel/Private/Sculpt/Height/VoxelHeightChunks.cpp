// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightChunks.h"

void FVoxelHeightNearChunkHint::Serialize(FArchive& Ar)
{
	Ar << RootHash;
	Ar << FarChunkKey;
	Ar << MidChunkKey;
	Ar << NearChunkKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightNearChunk::Serialize(FArchive& Ar)
{
	ChunkData.Serialize(Ar);
}

void FVoxelHeightNearChunk::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
{
	ChunkData.GatherObjects(OutObjects);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightMidChunkHint::Serialize(FArchive& Ar)
{
	Ar << RootHash;
	Ar << FarChunkKey;
	Ar << MidChunkKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelBulkRef<FVoxelHeightMidChunk> FVoxelHeightMidChunk::Create(
	TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightNearChunk>> KeyToNearChunk,
	const float TargetPrecision)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<FVoxelHeightChunkKey, const FVoxelHeightChunkData*> KeyToChunkData;
	{
		KeyToChunkData.Reserve(KeyToNearChunk.Num());

		for (const auto& It : KeyToNearChunk)
		{
			KeyToChunkData.Add_EnsureNew(It.Key, &It.Value->ChunkData);
		}
	}

	const TSharedRef<FVoxelHeightMidChunk> Result = MakeShared<FVoxelHeightMidChunk>();
	Result->ChunkData = FVoxelHeightChunkData::Downsample(KeyToChunkData, TargetPrecision);
	Result->KeyToNearChunk = MoveTemp(KeyToNearChunk);
	return MakeVoxelBulkRef(Result);
}

void FVoxelHeightMidChunk::Serialize(FArchive& Ar)
{
	ChunkData.Serialize(Ar);
	Ar << KeyToNearChunk;

	checkVoxelSlow(KeyToNearChunk.AreKeySorted());
}

void FVoxelHeightMidChunk::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
{
	for (const auto& It : KeyToNearChunk)
	{
		It.Value.GatherObjects(OutObjects);
	}

	ChunkData.GatherObjects(OutObjects);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightFarChunkHint::Serialize(FArchive& Ar)
{
	Ar << RootHash;
	Ar << FarChunkKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelBulkRef<FVoxelHeightFarChunk> FVoxelHeightFarChunk::Create(
	TVoxelMap<FVoxelHeightChunkKey, TVoxelBulkPtr<FVoxelHeightMidChunk>> KeyToMidChunk,
	const float TargetPrecision)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<FVoxelHeightChunkKey, const FVoxelHeightChunkData*> KeyToChunkData;
	{
		KeyToChunkData.Reserve(KeyToMidChunk.Num());

		for (const auto& It : KeyToMidChunk)
		{
			KeyToChunkData.Add_EnsureNew(It.Key, &It.Value->ChunkData);
		}
	}

	const TSharedRef<FVoxelHeightFarChunk> Result = MakeShared<FVoxelHeightFarChunk>();
	Result->ChunkData = FVoxelHeightChunkData::Downsample(KeyToChunkData, TargetPrecision);
	Result->KeyToMidChunk = MoveTemp(KeyToMidChunk);
	return MakeVoxelBulkRef(Result);
}

void FVoxelHeightFarChunk::Serialize(FArchive& Ar)
{
	ChunkData.Serialize(Ar);
	Ar << KeyToMidChunk;

	checkVoxelSlow(KeyToMidChunk.AreKeySorted());
}

void FVoxelHeightFarChunk::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
{
	for (const auto& It : KeyToMidChunk)
	{
		It.Value.GatherObjects(OutObjects);
	}

	ChunkData.GatherObjects(OutObjects);
}