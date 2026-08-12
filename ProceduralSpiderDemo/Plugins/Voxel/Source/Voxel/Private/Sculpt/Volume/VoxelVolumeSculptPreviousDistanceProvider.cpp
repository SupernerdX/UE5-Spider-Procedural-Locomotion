// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeSculptPreviousDistanceProvider.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"
#include "Sculpt/Volume/VoxelSculptVolumeCache.h"

void FVoxelVolumeSculptPreviousDistanceProvider_NaN::Query(
	const FIntVector& ChunkKey,
	const TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetDistances)
{
	const TVoxelStaticArray<float, FVoxelVolumeChunkDefinitions::ChunkCount> Distances{ FVoxelUtilities::NaNf() };
	SetDistances(Distances);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVolumeSculptPreviousDistanceProvider_Cache::FVoxelVolumeSculptPreviousDistanceProvider_Cache(
	const TSharedRef<FVoxelSculptVolumeCache>& Cache,
	const TSharedRef<FVoxelLayers>& Layers,
	const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
	const FVoxelWeakStackLayer& WeakLayer)
	: Cache(Cache)
	, Layers(Layers)
	, SurfaceTypeTable(SurfaceTypeTable)
	, WeakLayer(WeakLayer)
{
}

void FVoxelVolumeSculptPreviousDistanceProvider_Cache::Query(
	const FIntVector& ChunkKey,
	const TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetDistances)
{
	const TSharedRef<const FVoxelVolumeSculptPreviousChunk> Chunk = Cache->ComputeChunk(
		*Layers,
		*SurfaceTypeTable,
		WeakLayer,
		ChunkKey);

	SetDistances(Chunk->Distances);
}