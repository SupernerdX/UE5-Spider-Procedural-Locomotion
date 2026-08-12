// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelQuery.h"
#include "VoxelPointId.h"
#include "VoxelPointSet.h"
#include "VoxelStackLayer.h"
#include "Surface/VoxelSurfaceType.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;

// Generates scatter points at the center of each visible cubic mesh face
// that neighbors air (excludes faces between solid blocks with different surface types)
class FVoxelCubicFacePointGenerator
{
public:
	const FVoxelLayers& Layers;
	const FVoxelSurfaceTypeTable& SurfaceTypeTable;
	FVoxelDependencyCollector& DependencyCollector;
	const FVoxelWeakStackLayer WeakLayer;
	const FInt64Vector ChunkOffset;
	const int32 VoxelSize;
	const int32 ChunkSize;
	const uint64 Seed;
	const bool bQuerySurfaceTypes;
	const bool bResolveSurfaceTypes;
	const bool bSkipNaNFaces;
	const TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery;

	// DataSize = ChunkSize + 2 for neighbor sampling at boundaries
	const int32 DataSize;

	explicit FVoxelCubicFacePointGenerator(
		const FVoxelLayers& Layers,
		const FVoxelSurfaceTypeTable& SurfaceTypeTable,
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelWeakStackLayer& WeakLayer,
		const FInt64Vector& ChunkOffset,
		int32 VoxelSize,
		int32 ChunkSize,
		uint64 Seed,
		bool bQuerySurfaceTypes,
		bool bResolveSurfaceTypes,
		bool bSkipNaNFaces,
		TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery);

	FVoxelPointSet GeneratePoints();

private:
	// 3D grid of distances: NaN = unsampled, <0 = solid, >=0 = air
	TVoxelArray<float> DistanceGrid;

	// 3D grid of surface types (block types) for solid cells
	TVoxelArray<FVoxelSurfaceType> BlockTypeGrid;

	// Output point data
	TVoxelArray<FVoxelPointId> PointIds;
	TVoxelArray<double> PositionsX;
	TVoxelArray<double> PositionsY;
	TVoxelArray<double> PositionsZ;
	TVoxelArray<float> NormalsX;
	TVoxelArray<float> NormalsY;
	TVoxelArray<float> NormalsZ;
	TVoxelArray<FVoxelSurfaceTypeBlend> SurfaceTypes;

	// Core algorithm steps (adapted from FVoxelCubicMesher)
	void QueryDistancesAndSurfaceTypes();
	void QueryHeightLayerCells(const FVoxelQuery& Query, const FVoxelIntBox& VolumeBounds);
	void ResolveSmartSurfaceTypes();
	void GenerateFacePoints();

	// Add a point at the center of a face
	// Direction: 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z
	void AddFacePoint(
		const FIntVector& CellPosition,
		int32 FaceDirection,
		FVoxelSurfaceType BlockType);

	// Helpers
	FORCEINLINE int32 GetGridIndex(
		const int32 X,
		const int32 Y,
		const int32 Z) const
	{
		return X + DataSize * (Y + DataSize * Z);
	}

	FORCEINLINE float GetDistance(
		const int32 X,
		const int32 Y,
		const int32 Z) const
	{
		if (X < 0 ||
			Y < 0 ||
			Z < 0 ||
			X >= DataSize ||
			Y >= DataSize ||
			Z >= DataSize)
		{
			return FVoxelUtilities::NaNf();
		}
		return DistanceGrid[GetGridIndex(X, Y, Z)];
	}

	FORCEINLINE bool IsSolid(
		const int32 X,
		const int32 Y,
		const int32 Z) const
	{
		const float Distance = GetDistance(X, Y, Z);
		return !FVoxelUtilities::IsNaN(Distance) && Distance < 0;
	}

	FORCEINLINE bool IsNaN(
		const int32 X,
		const int32 Y,
		const int32 Z) const
	{
		return FVoxelUtilities::IsNaN(GetDistance(X, Y, Z));
	}
};
