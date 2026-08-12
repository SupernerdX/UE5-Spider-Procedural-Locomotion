// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkPtr.h"
#include "Sculpt/Height/VoxelHeightChunks.h"

class FVoxelSculptHeightTreeBuilder : public FVoxelHeightChunkDefinitions
{
public:
	static TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>> Build(
		const TVoxelMap<FIntPoint, TVoxelBulkPtr<FVoxelHeightFarChunk>>& PreviousKeyToFarChunk,
		const FVoxelIntBox2D& BoundsToReplace,
		const TVoxelMap<FIntPoint, FVoxelHeightChunkData>& KeyToNewChunkData,
		float TargetPrecision);

private:
	static TVoxelBulkPtr<FVoxelHeightFarChunk> BuildFarChunk(
		const FVoxelIntBox2D& BoundsToReplace,
		const TVoxelMap<FIntPoint, FVoxelHeightChunkData>& KeyToNewChunkData,
		const FIntPoint& FarChunkKey,
		const TVoxelBulkPtr<FVoxelHeightFarChunk>& PreviousFarChunkPtr,
		float TargetPrecision);

	static TVoxelBulkPtr<FVoxelHeightMidChunk> BuildMidChunk(
		const FVoxelIntBox2D& BoundsToReplace,
		const TVoxelMap<FIntPoint, FVoxelHeightChunkData>& KeyToNewChunkData,
		const FIntPoint& MidChunkKey,
		const TVoxelBulkPtr<FVoxelHeightMidChunk>& PreviousMidChunkPtr,
		float TargetPrecision);

	static TVoxelBulkPtr<FVoxelHeightNearChunk> BuildNearChunk(
		const FVoxelIntBox2D& BoundsToReplace,
		const TVoxelMap<FIntPoint, FVoxelHeightChunkData>& KeyToNewChunkData,
		const FIntPoint& NearChunkKey,
		const TVoxelBulkPtr<FVoxelHeightNearChunk>& PreviousNearChunkPtr);
};