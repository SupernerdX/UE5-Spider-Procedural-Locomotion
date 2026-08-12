// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Height/VoxelHeightChunks.h"

struct FVoxelSculptHeightData;

class FVoxelSculptHeightDiff
{
public:
	static TSharedRef<const FVoxelSculptHeightDiff> Create(
		const TVoxelBulkRef<FVoxelSculptHeightData>& OldData,
		const TVoxelBulkRef<FVoxelSculptHeightData>& NewData);

	TVoxelBulkPtr<FVoxelSculptHeightData> TryApply(
		const TVoxelBulkRef<FVoxelSculptHeightData>& OldData,
		float TargetPrecision) const;

private:
	struct FNearChunkDiff
	{
		FVoxelHeightChunkKey ChunkKey;
		FVoxelBulkHash OldHash;
		TVoxelBulkPtr<FVoxelHeightNearChunk> NewChunk;
	};
	struct FMidChunkDiff
	{
		FVoxelHeightChunkKey ChunkKey;
		FVoxelBulkHash OldHash;
		TVoxelBulkPtr<FVoxelHeightMidChunk> NewChunk;

		TVoxelArray<FNearChunkDiff> NearChunkDiffs;
	};
	struct FFarChunkDiff
	{
		FIntPoint ChunkKey;
		FVoxelBulkHash OldHash;
		TVoxelBulkPtr<FVoxelHeightFarChunk> NewChunk;

		TVoxelArray<FMidChunkDiff> MidChunkDiffs;
	};

	FVoxelBulkHash OldHash;
	TVoxelBulkPtr<FVoxelSculptHeightData> NewData;

	TVoxelArray<FFarChunkDiff> FarChunkDiffs;

private:
	static TVoxelOptional<FFarChunkDiff> Create(
		const FIntPoint& ChunkKey,
		TVoxelBulkPtr<FVoxelHeightFarChunk> OldFarChunk,
		TVoxelBulkPtr<FVoxelHeightFarChunk> NewFarChunk);

	static TVoxelOptional<FMidChunkDiff> Create(
		FVoxelHeightChunkKey ChunkKey,
		TVoxelBulkPtr<FVoxelHeightMidChunk> OldMidChunk,
		TVoxelBulkPtr<FVoxelHeightMidChunk> NewMidChunk);

	static TVoxelOptional<FNearChunkDiff> Create(
		FVoxelHeightChunkKey ChunkKey,
		TVoxelBulkPtr<FVoxelHeightNearChunk> OldNearChunk,
		TVoxelBulkPtr<FVoxelHeightNearChunk> NewNearChunk);

private:
	static TVoxelOptional<TVoxelBulkPtr<FVoxelHeightFarChunk>> TryApply(
		const FFarChunkDiff& FarChunkDiff,
		const TVoxelBulkPtr<FVoxelHeightFarChunk>& OldFarChunk,
		float TargetPrecision);

	static TVoxelOptional<TVoxelBulkPtr<FVoxelHeightMidChunk>> TryApply(
		const FMidChunkDiff& MidChunkDiff,
		const TVoxelBulkPtr<FVoxelHeightMidChunk>& OldMidChunk,
		float TargetPrecision);

	static TVoxelOptional<TVoxelBulkPtr<FVoxelHeightNearChunk>> TryApply(
		const FNearChunkDiff& NearChunkDiff,
		const TVoxelBulkPtr<FVoxelHeightNearChunk>& OldNearChunk);
};
