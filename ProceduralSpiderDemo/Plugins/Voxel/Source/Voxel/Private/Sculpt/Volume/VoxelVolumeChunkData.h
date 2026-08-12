// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "Sculpt/Volume/VoxelVolumeChunkKey.h"
#include "Sculpt/Volume/VoxelVolumeDistanceChunk.h"
#include "Sculpt/Volume/VoxelVolumeMetadataChunk.h"
#include "Sculpt/Volume/VoxelVolumeSurfaceTypeChunk.h"

struct FVoxelVolumeChunkTrees;

struct FVoxelVolumeChunkData : public FVoxelVolumeChunkDefinitions
{
public:
	TVoxelRefCountPtr<const FVoxelVolumeDistanceChunk> DistanceChunk;
	TVoxelRefCountPtr<const FVoxelVolumeSurfaceTypeChunk> SurfaceTypeChunk;

	struct FMetadataChunk
	{
		FVoxelMetadataRef MetadataRef;
		TVoxelRefCountPtr<const FVoxelVolumeMetadataChunk> Chunk;
	};
	TVoxelArray<FMetadataChunk> MetadataChunks;

public:
	bool IsEmpty() const;
	void Serialize(FArchive& Ar);
	void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const;

public:
	static FVoxelVolumeChunkData Downsample(
		const TVoxelMap<FVoxelVolumeChunkKey, const FVoxelVolumeChunkData*>& KeyToChunkData,
		float MaxErrorPercentage);
};