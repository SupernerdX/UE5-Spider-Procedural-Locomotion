// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightSculptPreviousHeightProvider.h"
#include "Sculpt/Height/VoxelSculptHeightCache.h"
#include "Sculpt/Height/VoxelHeightChunkDefinitions.h"

void FVoxelHeightSculptPreviousHeightProvider_NaN::Query(
	const FIntPoint& ChunkKey,
	const TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetHeights)
{
	const TVoxelStaticArray<float, FVoxelHeightChunkDefinitions::ChunkCount> Distances{ FVoxelUtilities::NaNf() };
	SetHeights(Distances);
}

FVoxelHeightSculptPreviousHeightProvider_Cache::FVoxelHeightSculptPreviousHeightProvider_Cache(
	const TSharedRef<FVoxelSculptHeightCache>& Cache,
	const TSharedRef<FVoxelLayers>& Layers,
	const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
	const FVoxelWeakStackLayer& WeakLayer)
	: Cache(Cache)
	, Layers(Layers)
	, SurfaceTypeTable(SurfaceTypeTable)
	, WeakLayer(WeakLayer)
{
}

void FVoxelHeightSculptPreviousHeightProvider_Cache::Query(
	const FIntPoint& ChunkKey,
	const TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetHeights)
{
	const TSharedRef<const FVoxelHeightSculptPreviousChunk> Chunk = Cache->ComputeChunk(
		*Layers,
		*SurfaceTypeTable,
		WeakLayer,
		ChunkKey);

	SetHeights(Chunk->Heights);
}
