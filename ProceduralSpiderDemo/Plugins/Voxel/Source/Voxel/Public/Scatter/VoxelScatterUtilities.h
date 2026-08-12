// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelMetadataRef.h"
#include "VoxelScatterUtilities.generated.h"

class FVoxelQuery;
struct FVoxelPointSet;
struct FVoxelDoubleVector2DBuffer;

UENUM()
enum class EVoxelHeightScatterType : uint8
{
	Grid,
	Sobol,
	Halton
};

struct VOXEL_API FVoxelScatterUtilities
{
public:
	static FVoxelPointSet ScatterPoints3D(
		const FVoxelQuery& Query,
		const FVoxelBox& Bounds,
		const TOptional<FVoxelBox>& OptionalInitialBounds,
		float DistanceBetweenPoints,
		uint64 Seed,
		float Looseness,
		float MinCellArea,
		const FVoxelWeakStackLayer& WeakLayer,
		bool bQuerySurfaceTypes,
		bool bResolveSurfaceTypes,
		TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery);
	static FVoxelPointSet ScatterPoints2D(
		const FVoxelQuery& Query,
		const FVoxelBox& Bounds,
		const TOptional<FVoxelBox>& OptionalInitialBounds,
		float DistanceBetweenPoints,
		uint64 Seed,
		float Looseness,
		EVoxelHeightScatterType Type,
		const FVoxelWeakStackLayer& WeakLayer,
		bool bQuerySurfaceTypes,
		bool bResolveSurfaceTypes,
		TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery);

	static FVoxelPointSet ScatterPointsCubicFaces(
		const FVoxelQuery& Query,
		const FVoxelBox& Bounds,
		int32 VoxelSize,
		int32 ChunkSize,
		uint64 Seed,
		const FVoxelWeakStackLayer& WeakLayer,
		bool bQuerySurfaceTypes,
		bool bResolveSurfaceTypes,
		bool bSkipNaNFaces,
		TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery);

public:
	static bool PrepareChunksForPositionsGenerator(
		const FVoxelBox2D& Bounds,
		const double DistanceBetweenPositions,
		FVoxelDoubleVector2DBuffer& OutPositions,
		const TFunction<void(const FVoxelIntBox2D&, const int64, int64&)>& Lambda);

	static bool GenerateHaltonPositions(
		const FVoxelBox& Bounds,
		const double DistanceBetweenPositions,
		uint32 Seed,
		FVoxelDoubleVector2DBuffer& OutPositions);
	static bool GenerateGridPositions(
		const FVoxelBox& Bounds,
		const double DistanceBetweenPositions,
		const double JitterDistance,
		uint32 Seed,
		FVoxelDoubleVector2DBuffer& OutPositions);
	static bool GenerateSobolPositions(
		const FVoxelBox& Bounds,
		const double DistanceBetweenPositions,
		uint32 Seed,
		FVoxelDoubleVector2DBuffer& OutPositions);
};