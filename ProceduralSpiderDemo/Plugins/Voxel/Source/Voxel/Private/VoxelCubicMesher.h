// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMesh.h"
#include "VoxelQuery.h"
#include "VoxelStackLayer.h"
#include "VoxelCellGenerator.h"

class FVoxelLayers;
class FVoxelMegaMaterialProxy;
class FVoxelSurfaceTypeTable;
class FVoxelHeightLayer;

// Minecraft-style cubic mesher that generates axis-aligned cube faces
// Uses existing surface types as block types
class FVoxelCubicMesher
{
public:
	FVoxelLayers& Layers;
	FVoxelSurfaceTypeTable& SurfaceTypeTable;
	FVoxelDependencyCollector& DependencyCollector;
	const FVoxelWeakStackLayer WeakLayer;
	const FInt64Vector ChunkOffset;
	const int32 VoxelSize;
	const int32 ChunkSize;
	const FTransform LocalToWorld;
	const FVoxelMegaMaterialProxy& MegaMaterialProxy;

	// DataSize = ChunkSize + 2 for neighbor sampling at boundaries
	const int32 DataSize;

	explicit FVoxelCubicMesher(
		FVoxelLayers& Layers,
		FVoxelSurfaceTypeTable& SurfaceTypeTable,
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelWeakStackLayer& WeakLayer,
		const FInt64Vector& ChunkOffset,
		int32 VoxelSize,
		int32 ChunkSize,
		const FTransform& LocalToWorld,
		const FVoxelMegaMaterialProxy& MegaMaterialProxy,
		const TSharedPtr<const FVoxelCellGeneratorHeights>& CachedHeights = nullptr);

	TSharedPtr<FVoxelMesh> CreateMesh();

	// Returns 1-2 meshes split by blend mode:
	// [0] = Masked mesh (contains opaque + masked surfaces)
	// [1] = Translucent mesh (if any translucent surfaces exist)
	TVoxelArray<TSharedPtr<FVoxelMesh>> CreateMeshes();

private:
	// Cached heights from caller (optional)
	TSharedPtr<const FVoxelCellGeneratorHeights> CachedHeights;

	// Locally computed heights (if not cached)
	TSharedPtr<const FVoxelCellGeneratorHeights> Heights;

	// 3D grid of distances: NaN = unsampled, <0 = solid, >=0 = air
	TVoxelArray<float> DistanceGrid;

	// 3D grid of surface types (block types) for solid cells
	TVoxelArray<FVoxelSurfaceType> BlockTypeGrid;

	// Output mesh data
	TVoxelArray<int32> Indices;
	TVoxelArray<FVector3f> Vertices;
	TVoxelArray<FVector3f> Normals;
	TVoxelArray<FVoxelMesh::FCell> Cells;
	TVoxelArray<FVoxelSurfaceTypeBlend> SurfaceTypes;

	// Core algorithm steps
	void QueryDistancesAndSurfaceTypes();
	void QueryHeightLayerCells(const FVoxelQuery& Query, const FVoxelIntBox& VolumeBounds);
	void ResolveSmartSurfaceTypes();
	// When bAirFacesOnly is true, only generates faces adjacent to air cells
	// (skips faces between solid blocks with different surface types)
	void GenerateFaces(bool bAirFacesOnly = false);
	void FilterInvisibleSurfaceTypes();

	// Add a single quad face for a cell in the given direction
	// Direction: 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z
	void AddFace(
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
