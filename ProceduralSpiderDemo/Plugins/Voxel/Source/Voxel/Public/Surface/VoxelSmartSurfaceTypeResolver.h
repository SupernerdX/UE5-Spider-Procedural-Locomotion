// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;

class VOXEL_API FVoxelSmartSurfaceTypeResolver
{
public:
	const int32 LOD;
	const FVoxelWeakStackLayer WeakLayer;
	const FVoxelLayers& Layers;
	const FVoxelSurfaceTypeTable& SurfaceTypeTable;
	FVoxelDependencyCollector& DependencyCollector;

	const int32 NumVertices;
	const FVoxelDoubleVectorBuffer VertexPositions;
	const FVoxelVectorBuffer VertexNormals;
	const TVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypeBlends;

	FVoxelSmartSurfaceTypeResolver(
		int32 LOD,
		const FVoxelWeakStackLayer& WeakLayer,
		const FVoxelLayers& Layers,
		const FVoxelSurfaceTypeTable& SurfaceTypeTable,
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelDoubleVectorBuffer& VertexPositions,
		const FVoxelVectorBuffer& VertexNormals,
		TVoxelArrayView<FVoxelSurfaceTypeBlend> SurfaceTypeBlends);

	void Resolve();

private:
	FVoxelBitArray IncompleteVertices;

	mutable FVoxelDoubleVectorBuffer PositionBuffer;
	mutable FVoxelVectorBuffer NormalBuffer;

	bool TryFindIncompleteVertices();
	bool TryResolve();

	FVoxelSurfaceTypeBlendBuffer ComputeSurfaceBlends(
		const FVoxelSurfaceType& SurfaceType,
		TConstVoxelArrayView<int32> VerticesToCompute) const;
};