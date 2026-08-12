// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/VoxelVolumeChunks.h"

struct FVoxelSculptVolumeData;

class FVoxelSculptVolumeDiff
{
public:
	static TSharedRef<const FVoxelSculptVolumeDiff> Create(
		const TVoxelBulkRef<FVoxelSculptVolumeData>& OldData,
		const TVoxelBulkRef<FVoxelSculptVolumeData>& NewData);

	TVoxelBulkPtr<FVoxelSculptVolumeData> TryApply(
		const TVoxelBulkRef<FVoxelSculptVolumeData>& OldData,
		float MaxErrorPercentage) const;

private:
	struct FNearChunkDiff
	{
		FVoxelVolumeChunkKey ChunkKey;
		FVoxelBulkHash OldHash;
		TVoxelBulkPtr<FVoxelVolumeNearChunk> NewChunk;
	};
	struct FMidChunkDiff
	{
		FVoxelVolumeChunkKey ChunkKey;
		FVoxelBulkHash OldHash;
		TVoxelBulkPtr<FVoxelVolumeMidChunk> NewChunk;

		TVoxelArray<FNearChunkDiff> NearChunkDiffs;
	};
	struct FFarChunkDiff
	{
		FIntVector ChunkKey;
		FVoxelBulkHash OldHash;
		TVoxelBulkPtr<FVoxelVolumeFarChunk> NewChunk;

		TVoxelArray<FMidChunkDiff> MidChunkDiffs;
	};

	FVoxelBulkHash OldHash;
	TVoxelBulkPtr<FVoxelSculptVolumeData> NewData;

	TVoxelArray<FFarChunkDiff> FarChunkDiffs;

private:
	static TVoxelOptional<FFarChunkDiff> Create(
		const FIntVector& ChunkKey,
		TVoxelBulkPtr<FVoxelVolumeFarChunk> OldFarChunk,
		TVoxelBulkPtr<FVoxelVolumeFarChunk> NewFarChunk);

	static TVoxelOptional<FMidChunkDiff> Create(
		FVoxelVolumeChunkKey ChunkKey,
		TVoxelBulkPtr<FVoxelVolumeMidChunk> OldMidChunk,
		TVoxelBulkPtr<FVoxelVolumeMidChunk> NewMidChunk);

	static TVoxelOptional<FNearChunkDiff> Create(
		FVoxelVolumeChunkKey ChunkKey,
		TVoxelBulkPtr<FVoxelVolumeNearChunk> OldNearChunk,
		TVoxelBulkPtr<FVoxelVolumeNearChunk> NewNearChunk);

private:
	static TVoxelOptional<TVoxelBulkPtr<FVoxelVolumeFarChunk>> TryApply(
		const FFarChunkDiff& FarChunkDiff,
		const TVoxelBulkPtr<FVoxelVolumeFarChunk>& OldFarChunk,
		float MaxErrorPercentage);

	static TVoxelOptional<TVoxelBulkPtr<FVoxelVolumeMidChunk>> TryApply(
		const FMidChunkDiff& MidChunkDiff,
		const TVoxelBulkPtr<FVoxelVolumeMidChunk>& OldMidChunk,
		float MaxErrorPercentage);

	static TVoxelOptional<TVoxelBulkPtr<FVoxelVolumeNearChunk>> TryApply(
		const FNearChunkDiff& NearChunkDiff,
		const TVoxelBulkPtr<FVoxelVolumeNearChunk>& OldNearChunk);
};