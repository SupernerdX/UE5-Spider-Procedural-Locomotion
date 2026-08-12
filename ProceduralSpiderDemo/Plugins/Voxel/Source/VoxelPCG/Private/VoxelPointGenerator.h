// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelMetadataRef.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"

class FVoxelState;
class FVoxelLayers;
class FVoxelSurfaceTypeTable;

class FVoxelPointGenerator : public TSharedFromThis<FVoxelPointGenerator>
{
public:
	const TSharedRef<FVoxelDependencyCollector> DependencyCollector;
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const FVector Start;
	const FIntVector Size;
	const float CellSize;
	const FVoxelWeakStackLayer WeakLayer;
	const int32 LOD;
	const bool bResolveSmartSurfaceTypes;

	FVoxelPointGenerator(
		const TSharedRef<FVoxelDependencyCollector>& DependencyCollector,
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const FVector& Start,
		const FIntVector& Size,
		float CellSize,
		const FVoxelWeakStackLayer& WeakLayer,
		int32 LOD,
		bool bResolveSmartSurfaceTypes);

	FVoxelFuture Initialize(float Tolerance);

	struct FPoint
	{
		FVector Position;
		FVector3f Normal;
		int32 Seed;
		float Density;
	};
	TVoxelArray<FPoint> Generate(
		const FVoxelBox& Bounds,
		int32 Seed,
		float Looseness,
		float Ratio,
		bool bApplyDensityToPoints,
		TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery,
		FVoxelSurfaceTypeBlendBuffer& OutSurfaceTypes,
		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>>& OutMetadataToBuffer);

private:
	static constexpr int32 ChunkSize = 16;
	static constexpr int32 ChunkCount = FMath::Cube(ChunkSize);

	FIntVector SizeInChunks = FIntVector(ForceInit);

	struct FChunk
	{
		FIntVector Size = FIntVector(ForceInit);
		FVoxelFloatBuffer Distances;
	};
	TVoxelArray<FChunk> Chunks;

	FORCEINLINE float GetDistance(
		const int32 X,
		const int32 Y,
		const int32 Z)
	{
		checkVoxelSlow(0 <= X && X < Size.X);
		checkVoxelSlow(0 <= X && Y < Size.Y);
		checkVoxelSlow(0 <= X && Z < Size.Z);

		const FIntVector ChunkPosition = FVoxelUtilities::DivideFloor(FIntVector(X, Y, Z), ChunkSize);
		const FChunk& Chunk = Chunks[FVoxelUtilities::Get3DIndex<int32>(SizeInChunks, ChunkPosition)];

		if (Chunk.Distances.Num() == 0)
		{
			return FVoxelUtilities::NaNf();
		}

		const int32 Index = FVoxelUtilities::Get3DIndex<int32>(Chunk.Size, X % ChunkSize, Y % ChunkSize, Z % ChunkSize);

		return Chunk.Distances[Index];
	}
};