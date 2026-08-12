// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class UVoxelHeightmap_Height;
enum class EVoxelHeightmapPrecision : uint8;

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelHeightmap_HeightMemory, "Voxel Heightmap Height Memory");

struct VOXEL_API FVoxelHeightmap_HeightData
{
public:
	int32 SizeX = 0;
	int32 SizeY = 0;
	bool bIsUINT16 = false;
	float InternalScaleZ = 1.f;
	float InternalOffsetZ = 0.f;
	float DataMin = 0.f;
	float DataMax = 0.f;
	TVoxelArray64<uint8> RawData;

	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelHeightmap_HeightMemory);

public:
#if WITH_EDITOR
	bool bCustomData = false;
	TSoftObjectPtr<UTexture2D> Texture;
	EVoxelTextureChannel TextureChannel = {};
	bool bCompressTo16Bits = false;
	bool bEnableMinHeight = false;
	float MinHeight = 0;
	float MinHeightSlope = 0;

	static TSharedPtr<const FVoxelHeightmap_HeightData> Create(const UVoxelHeightmap_Height& Height);
#endif

public:
	void Serialize(FArchive& Ar, UObject* Owner);
	int64 GetAllocatedSize() const;

private:
	FByteBulkData BulkData;

#if WITH_EDITOR
	bool CreateImpl();
	void EncodeHeights(TConstVoxelArrayView<float> Heights);
#endif
};