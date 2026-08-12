// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;
class FVoxelSculptVolumeCache;

class VOXEL_API IVoxelVolumeSculptPreviousDistanceProvider
{
public:
	IVoxelVolumeSculptPreviousDistanceProvider() = default;
	virtual	~IVoxelVolumeSculptPreviousDistanceProvider() = default;

	virtual void Query(
		const FIntVector& ChunkKey,
		TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetDistances) = 0;
};

class VOXEL_API FVoxelVolumeSculptPreviousDistanceProvider_NaN : public IVoxelVolumeSculptPreviousDistanceProvider
{
public:
	//~ Begin IVoxelVolumeSculptPreviousDistanceProvider Interface
	virtual void Query(
		const FIntVector& ChunkKey,
		TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetDistances) override;
	//~ End IVoxelVolumeSculptPreviousDistanceProvider Interface
};

class VOXEL_API FVoxelVolumeSculptPreviousDistanceProvider_Cache : public IVoxelVolumeSculptPreviousDistanceProvider
{
public:
	const TSharedRef<FVoxelSculptVolumeCache> Cache;
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const FVoxelWeakStackLayer WeakLayer;

	FVoxelVolumeSculptPreviousDistanceProvider_Cache(
		const TSharedRef<FVoxelSculptVolumeCache>& Cache,
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const FVoxelWeakStackLayer& WeakLayer);

	//~ Begin IVoxelVolumeSculptPreviousDistanceProvider Interface
	virtual void Query(
		const FIntVector& ChunkKey,
		TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetDistances) override;
	//~ End IVoxelVolumeSculptPreviousDistanceProvider Interface
};