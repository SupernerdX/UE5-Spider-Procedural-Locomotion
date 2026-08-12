// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Sculpt/Height/VoxelHeightChunkDefinitions.h"

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelHeightSurfaceType_Memory, "Voxel Height Surface Type Memory");

struct FVoxelHeightSurfaceTypeChunk
	: FVoxelHeightChunkDefinitions
	, TVoxelRefCountThis<FVoxelHeightSurfaceTypeChunk>
{
public:
	struct FLayer
	{
		TVoxelStaticArray<uint8, ChunkCount> Types{ NoInit };
		// Not normalized
		TVoxelStaticArray<uint8, ChunkCount> Weights{ NoInit };

		friend FArchive& operator<<(FArchive& Ar, FLayer& Layer)
		{
			Ar << Layer.Types;
			Ar << Layer.Weights;
			return Ar;
		}
	};

	TVoxelStaticArray<uint8, ChunkCount> Alphas{ NoInit };
	TVoxelArray<FLayer> Layers;
	TVoxelArray<FVoxelSurfaceType> UsedSurfaceTypes;

	FVoxelHeightSurfaceTypeChunk();
	UE_NONCOPYABLE(FVoxelHeightSurfaceTypeChunk)

	VOXEL_COUNT_INSTANCES();
	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelHeightSurfaceType_Memory);

public:
	void GetSurfaceType(
		int32 Index,
		float& OutAlpha,
		FVoxelSurfaceTypeBlend& OutSurfaceType) const;

	void GetAverageSurfaceType(
		float& OutAlpha,
		FVoxelSurfaceTypeBlend& OutSurfaceType) const;

	static TVoxelRefCountPtr<FVoxelHeightSurfaceTypeChunk> Create(
		TConstVoxelArrayView<float> Alphas,
		TConstVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypes,
		const FIntPoint& Offset,
		const FIntPoint& Size);

public:
	void Serialize(
		FArchive& Ar,
		int32 Version);

	int64 GetAllocatedSize() const;
};