// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkPtr.h"
#include "Sculpt/Volume/VoxelVolumeChunks.h"

class FVoxelSculptVolumeTreeBuilder : public FVoxelVolumeChunkDefinitions
{
public:
	static TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>> Build(
		const TVoxelMap<FIntVector, TVoxelBulkPtr<FVoxelVolumeFarChunk>>& PreviousKeyToFarChunk,
		const FVoxelIntBox& BoundsToReplace,
		const TVoxelMap<FIntVector, FVoxelVolumeChunkData>& KeyToNewChunkData,
		float MaxErrorPercentage);

private:
	static TVoxelBulkPtr<FVoxelVolumeFarChunk> BuildFarChunk(
		const FVoxelIntBox& BoundsToReplace,
		const TVoxelMap<FIntVector, FVoxelVolumeChunkData>& KeyToNewChunkData,
		const FIntVector& FarChunkKey,
		const TVoxelBulkPtr<FVoxelVolumeFarChunk>& PreviousFarChunkPtr,
		float MaxErrorPercentage);

	static TVoxelBulkPtr<FVoxelVolumeMidChunk> BuildMidChunk(
		const FVoxelIntBox& BoundsToReplace,
		const TVoxelMap<FIntVector, FVoxelVolumeChunkData>& KeyToNewChunkData,
		const FIntVector& MidChunkKey,
		const TVoxelBulkPtr<FVoxelVolumeMidChunk>& PreviousMidChunkPtr,
		float MaxErrorPercentage);

	static TVoxelBulkPtr<FVoxelVolumeNearChunk> BuildNearChunk(
		const FVoxelIntBox& BoundsToReplace,
		const TVoxelMap<FIntVector, FVoxelVolumeChunkData>& KeyToNewChunkData,
		const FIntVector& NearChunkKey,
		const TVoxelBulkPtr<FVoxelVolumeNearChunk>& PreviousNearChunkPtr);
};