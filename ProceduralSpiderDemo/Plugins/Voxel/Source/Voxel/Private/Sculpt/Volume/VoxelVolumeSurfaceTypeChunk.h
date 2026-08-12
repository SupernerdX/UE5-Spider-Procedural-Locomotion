// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Surface/VoxelSurfaceTypeBlend.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelVolumeSurfaceType_Memory, "Voxel Volume Surface Type Memory");

struct FVoxelVolumeSurfaceTypeChunk
	: FVoxelVolumeChunkDefinitions
	, TVoxelRefCountThis<FVoxelVolumeSurfaceTypeChunk>
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

	FVoxelVolumeSurfaceTypeChunk();
	UE_NONCOPYABLE(FVoxelVolumeSurfaceTypeChunk)

	VOXEL_COUNT_INSTANCES();
	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelVolumeSurfaceType_Memory);

public:
	void GetSurfaceType(
		int32 Index,
		float& OutAlpha,
		FVoxelSurfaceTypeBlend& OutSurfaceType) const;

	void GetAverageSurfaceType(
		float& OutAlpha,
		FVoxelSurfaceTypeBlend& OutSurfaceType) const;

	static TVoxelRefCountPtr<FVoxelVolumeSurfaceTypeChunk> Create(
		TConstVoxelArrayView<float> Alphas,
		TConstVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypes,
		const FIntVector& Offset,
		const FIntVector& Size);

public:
	void Serialize(
		FArchive& Ar,
		int32 Version);

	int64 GetAllocatedSize() const;
};