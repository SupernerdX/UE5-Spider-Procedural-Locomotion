// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;
class FVoxelSculptHeightCache;

class VOXEL_API IVoxelHeightSculptPreviousHeightProvider
{
public:
	IVoxelHeightSculptPreviousHeightProvider() = default;
	virtual ~IVoxelHeightSculptPreviousHeightProvider() = default;

	virtual void Query(
		const FIntPoint& ChunkKey,
		TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetHeights) = 0;
};

class VOXEL_API FVoxelHeightSculptPreviousHeightProvider_NaN : public IVoxelHeightSculptPreviousHeightProvider
{
public:
	//~ Begin IVoxelHeightSculptPreviousHeightProvider Interface
	virtual void Query(
		const FIntPoint& ChunkKey,
		TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetHeights) override;
	//~ End IVoxelHeightSculptPreviousHeightProvider Interface
};

class VOXEL_API FVoxelHeightSculptPreviousHeightProvider_Cache : public IVoxelHeightSculptPreviousHeightProvider
{
public:
	const TSharedRef<FVoxelSculptHeightCache> Cache;
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const FVoxelWeakStackLayer WeakLayer;

	FVoxelHeightSculptPreviousHeightProvider_Cache(
		const TSharedRef<FVoxelSculptHeightCache>& Cache,
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const FVoxelWeakStackLayer& WeakLayer);

	//~ Begin IVoxelHeightSculptPreviousHeightProvider Interface
	virtual void Query(
		const FIntPoint& ChunkKey,
		TVoxelFunctionRef<void(TConstVoxelArrayView<float>)> SetHeights) override;
	//~ End IVoxelHeightSculptPreviousHeightProvider Interface
};
