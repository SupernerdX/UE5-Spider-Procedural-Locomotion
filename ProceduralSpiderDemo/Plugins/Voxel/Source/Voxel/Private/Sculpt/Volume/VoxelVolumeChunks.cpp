// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeChunks.h"

void FVoxelVolumeNearChunkHint::Serialize(FArchive& Ar)
{
	Ar << RootHash;
	Ar << FarChunkKey;
	Ar << MidChunkKey;
	Ar << NearChunkKey;
}

void FVoxelVolumeNearChunk::Serialize(FArchive& Ar)
{
	ChunkData.Serialize(Ar);
}

void FVoxelVolumeNearChunk::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
{
	ChunkData.GatherObjects(OutObjects);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeMidChunkHint::Serialize(FArchive& Ar)
{
	Ar << RootHash;
	Ar << FarChunkKey;
	Ar << MidChunkKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelBulkRef<FVoxelVolumeMidChunk> FVoxelVolumeMidChunk::Create(
	TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeNearChunk>> KeyToNearChunk,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<FVoxelVolumeChunkKey, const FVoxelVolumeChunkData*> KeyToChunkData;
	{
		KeyToChunkData.Reserve(KeyToNearChunk.Num());

		for (const auto& It : KeyToNearChunk)
		{
			KeyToChunkData.Add_EnsureNew(It.Key, &It.Value->ChunkData);
		}
	}

	const TSharedRef<FVoxelVolumeMidChunk> Result = MakeShared<FVoxelVolumeMidChunk>();
	Result->ChunkData = FVoxelVolumeChunkData::Downsample(KeyToChunkData, MaxErrorPercentage);
	Result->KeyToNearChunk = MoveTemp(KeyToNearChunk);
	return MakeVoxelBulkRef(Result);
}

void FVoxelVolumeMidChunk::Serialize(FArchive& Ar)
{
	ChunkData.Serialize(Ar);
	Ar << KeyToNearChunk;

	checkVoxelSlow(KeyToNearChunk.AreKeySorted());
}

void FVoxelVolumeMidChunk::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
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

void FVoxelVolumeFarChunkHint::Serialize(FArchive& Ar)
{
	Ar << RootHash;
	Ar << FarChunkKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelBulkRef<FVoxelVolumeFarChunk> FVoxelVolumeFarChunk::Create(
	TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeMidChunk>> KeyToMidChunk,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<FVoxelVolumeChunkKey, const FVoxelVolumeChunkData*> KeyToChunkData;
	{
		KeyToChunkData.Reserve(KeyToMidChunk.Num());

		for (const auto& It : KeyToMidChunk)
		{
			KeyToChunkData.Add_EnsureNew(It.Key, &It.Value->ChunkData);
		}
	}

	const TSharedRef<FVoxelVolumeFarChunk> Result = MakeShared<FVoxelVolumeFarChunk>();
	Result->ChunkData = FVoxelVolumeChunkData::Downsample(KeyToChunkData, MaxErrorPercentage);
	Result->KeyToMidChunk = MoveTemp(KeyToMidChunk);
	return MakeVoxelBulkRef(Result);
}

void FVoxelVolumeFarChunk::Serialize(FArchive& Ar)
{
	ChunkData.Serialize(Ar);
	Ar << KeyToMidChunk;

	checkVoxelSlow(KeyToMidChunk.AreKeySorted());
}

void FVoxelVolumeFarChunk::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
{
	for (const auto& It : KeyToMidChunk)
	{
		It.Value.GatherObjects(OutObjects);
	}

	ChunkData.GatherObjects(OutObjects);
}