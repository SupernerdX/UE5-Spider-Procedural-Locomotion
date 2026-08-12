// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class UVoxelHeightmap_Weight;

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelHeightmap_WeightMemory, "Voxel Heightmap Weight Memory");

struct VOXEL_API FVoxelHeightmap_WeightData
{
public:
	int32 SizeX = 0;
	int32 SizeY = 0;
	TVoxelArray64<uint8> Weights;

	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelHeightmap_WeightMemory);

public:
#if WITH_EDITOR
	bool bCustomData = false;
	TSoftObjectPtr<UTexture2D> Texture;
	EVoxelTextureChannel TextureChannel = {};

	static TSharedPtr<const FVoxelHeightmap_WeightData> Create(const UVoxelHeightmap_Weight& Weightmap);
#endif

public:
	void Serialize(FArchive& Ar, UObject* Owner);
	int64 GetAllocatedSize() const;

private:
	FByteBulkData BulkData;

#if WITH_EDITOR
	bool CreateImpl();
#endif
};