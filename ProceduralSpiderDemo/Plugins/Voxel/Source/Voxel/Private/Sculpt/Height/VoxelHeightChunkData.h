// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"
#include "Sculpt/Height/VoxelHeightChunkKey.h"
#include "Sculpt/Height/VoxelHeightHeightChunk.h"
#include "Sculpt/Height/VoxelHeightMetadataChunk.h"
#include "Sculpt/Height/VoxelHeightSurfaceTypeChunk.h"

struct FVoxelHeightChunkTrees;

struct FVoxelHeightChunkData : public FVoxelHeightChunkDefinitions
{
public:
	TVoxelRefCountPtr<const FVoxelHeightHeightChunk> HeightChunk;
	TVoxelRefCountPtr<const FVoxelHeightSurfaceTypeChunk> SurfaceTypeChunk;

	struct FMetadataChunk
	{
		FVoxelMetadataRef MetadataRef;
		TVoxelRefCountPtr<const FVoxelHeightMetadataChunk> Chunk;
	};
	TVoxelArray<FMetadataChunk> MetadataChunks;

public:
	bool IsEmpty() const;
	void Serialize(FArchive& Ar);
	void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const;
	FDoubleInterval GetRange() const;

public:
	static FVoxelHeightChunkData Downsample(
		const TVoxelMap<FVoxelHeightChunkKey, const FVoxelHeightChunkData*>& KeyToChunkData,
		float TargetPrecision);
};