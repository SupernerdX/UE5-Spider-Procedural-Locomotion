// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadataRef.h"

class FVoxelMegaMaterialProxy;
class FVoxelTexturePool;
class UTexture2D;
class UTexture2DArray;

class FVoxelTextureManager
{
public:
	const TVoxelArray<FVoxelMetadataRef> MetadataIndexToMetadata;

	explicit FVoxelTextureManager(const FVoxelMegaMaterialProxy& MegaMaterialProxy);

public:
	FVoxelTexturePool& GetChunkIndicesBufferPool() const
	{
		return *ChunkIndicesBufferPool;
	}
	FVoxelTexturePool& GetMaterialBufferPool() const
	{
		return *MaterialBufferPool;
	}
	TSharedPtr<FVoxelTexturePool> GetMetadataBufferPool(const FVoxelMetadataRef Metadata) const
	{
		return MetadataToBufferPool.FindRef(Metadata);
	}
	const TVoxelMap<FVoxelMetadataRef, TSharedPtr<FVoxelTexturePool>>& GetMetadataToBufferPool() const
	{
		return MetadataToBufferPool;
	}

public:
	void UpdateInstance(UMaterialInstanceDynamic& Instance) const;
	
public:
	void ProcessUploads();
	void AddReferencedObjects(FReferenceCollector& Collector);

private:
	TSharedPtr<FVoxelTexturePool> ChunkIndicesBufferPool;
	TSharedPtr<FVoxelTexturePool> MaterialBufferPool;
	TVoxelMap<FVoxelMetadataRef, TSharedPtr<FVoxelTexturePool>> MetadataToBufferPool;

	// Cubic surface texture arrays (property name -> texture array)
	TVoxelMap<FName, TVoxelObjectPtr<UTexture2DArray>> CubicTextureArrays;

	// Lookup texture: render index -> cubic slice index (-1 if not cubic)
	// Texture is 256x1, R32_SINT format
	TVoxelObjectPtr<UTexture2D> CubicSliceLookupTexture;
};