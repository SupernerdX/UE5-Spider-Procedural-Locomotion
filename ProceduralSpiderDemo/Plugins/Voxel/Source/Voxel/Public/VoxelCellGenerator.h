// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelVolumeLayer.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;
class FVoxelHeightLayer;

struct VOXEL_API FVoxelCellGeneratorHeights
{
	FVoxelIntBox2D Indices;
	TVoxelArray<float> Heights;
	FVoxelDependencyCollector DependencyCollector = FVoxelDependencyCollector(STATIC_FNAME("FVoxelCellGeneratorHeights"));
};

struct VOXEL_API FVoxelCellGeneratorHeightOutput
{
	TVoxelArray<float> Heights;
	FVoxelIntBox2D Bounds;
};
struct VOXEL_API FVoxelCellGeneratorVolumeOutput
{
	TVoxelArray<float> Distances;
	FVoxelIntBox Bounds;
};

struct VOXEL_API FVoxelCellGeneratorOutput
{
	TOptional<FVoxelCellGeneratorHeightOutput> HeightData;
	TOptional<FVoxelCellGeneratorVolumeOutput> VolumeData;
};

class VOXEL_API FVoxelCellGenerator
{
public:
	FVoxelDependencyCollector DependencyCollector = FVoxelDependencyCollector(STATIC_FNAME("CellGenerator"));

	const int32 LOD;
	const FVoxelLayers& Layers;
	const FVoxelSurfaceTypeTable& SurfaceTypeTable;
	const FVector Start;
	const FIntVector Size;
	const float CellSize;
	const FVoxelWeakStackLayer WeakLayer;
	const FVoxelBox Bounds;
	const FVoxelQuery Query;
	const TSharedPtr<const FVoxelHeightLayer> HeightLayer;
	const TSharedPtr<const FVoxelVolumeLayer> VolumeLayer;

	TSharedPtr<const FVoxelCellGeneratorHeights> Heights;

	explicit FVoxelCellGenerator(
		int32 LOD,
		const FVoxelLayers& Layers,
		const FVoxelSurfaceTypeTable& SurfaceTypeTable,
		const FVector& Start,
		const FIntVector& Size,
		float CellSize,
		const FVoxelWeakStackLayer& WeakLayer,
		const TSharedPtr<const FVoxelCellGeneratorHeights>& CachedHeights);

	void ForeachCell(
		FVoxelDependencyCollector& OutDependencyCollector,
		TVoxelFunctionRef<void(const FIntVector&, const TVoxelStaticArray<float, 8>&)> Lambda,
		FVoxelCellGeneratorOutput* OutData = nullptr);

private:
	template<bool bHasVolumeBounds>
	void ForeachHeightCell(
		TVoxelFunctionRef<void(const FIntVector&, const TVoxelStaticArray<float, 8>&)> Lambda,
		FVoxelCellGeneratorOutput* OutData,
		const FVoxelIntBox& VolumeBounds);

	static bool IsValidCellDistances(const TVoxelStaticArray<float, 8>& CellDistances);
};