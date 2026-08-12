// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMesh.h"
#include "VoxelQuery.h"
#include "VoxelStackLayer.h"
#include "VoxelCellGenerator.h"
#include "VoxelFloatMetadataRef.h"
#include "Buffer/VoxelFloatBuffers.h"

class FVoxelCellGenerator;
class FVoxelMegaMaterialProxy;
struct FVoxelCellGeneratorOutput;
struct FVoxelCellGeneratorHeights;

class FVoxelMesher
{
public:
	static constexpr int32 MaxRelativeLOD = 3;

	FVoxelLayers& Layers;
	FVoxelSurfaceTypeTable& SurfaceTypeTable;
	FVoxelDependencyCollector& DependencyCollector;
	const FVoxelWeakStackLayer WeakLayer;
	const int32 ChunkLOD;
	const FInt64Vector ChunkOffset;
	const int32 VoxelSize;
	const int32 ChunkSize;
	const FTransform LocalToWorld;
	const FVoxelMegaMaterialProxy& MegaMaterialProxy;
	const FVoxelFloatMetadataRef BlockinessMetadata;
	const bool bGenerateDistanceFields;
	const float MeshDistanceFieldBias;
	const int32 MeshDistanceFieldBricksPerChunk;

	const int32 DataSize;

	bool bQueryMetadata = false;
	TSharedPtr<FVoxelCellGenerator> CellGenerator;

	explicit FVoxelMesher(
		FVoxelLayers& Layers,
		FVoxelSurfaceTypeTable& SurfaceTypeTable,
		FVoxelDependencyCollector& DependencyCollector,
		const FVoxelWeakStackLayer& WeakLayer,
		int32 ChunkLOD,
		const FInt64Vector& ChunkOffset,
		int32 VoxelSize,
		int32 ChunkSize,
		const FTransform& LocalToWorld,
		const FVoxelMegaMaterialProxy& MegaMaterialProxy,
		FVoxelFloatMetadataRef BlockinessMetadata,
		bool bGenerateDistanceFields,
		float MeshDistanceFieldBias,
		int32 MeshDistanceFieldBricksPerChunk);

	TSharedPtr<FVoxelMesh> CreateMesh(const TSharedPtr<const FVoxelCellGeneratorHeights>& CachedHeights);

private:
	struct alignas(8) FCell
	{
		int16 X;
		int16 Y;
		int16 Z;
		int16 Padding;

		FCell() = default;
		explicit FCell(const FIntVector& Position)
			: X(Position.X)
			, Y(Position.Y)
			, Z(Position.Z)
			, Padding(0)
		{
			checkVoxelSlow(FVoxelUtilities::IsValidINT16(Position.X));
			checkVoxelSlow(FVoxelUtilities::IsValidINT16(Position.Y));
			checkVoxelSlow(FVoxelUtilities::IsValidINT16(Position.Z));
		}

		FORCEINLINE bool operator==(const FCell& Other) const
		{
			checkVoxelSlow(Padding == 0);
			checkVoxelSlow(Other.Padding == 0);

			return ReinterpretCastRef<uint64>(*this) == ReinterpretCastRef<uint64>(Other);
		}
		FORCEINLINE friend uint32 GetTypeHash(const FCell& Cell)
		{
			checkVoxelSlow(Cell.Padding == 0);
			return FVoxelUtilities::MurmurHash64(ReinterpretCastRef<uint64>(Cell));
		}
	};
	TVoxelMap<FCell, int32> CellToVertexIndex;

	struct FEdge
	{
		union
		{
			struct
			{
				int16 X;
				int16 Y;
				int16 Z;
				uint8 Direction;
				uint8 Sign;
			};
			uint64 RawData;
		};
		uint32 MortonCode = 0;

		FORCEINLINE FEdge()
		{
			RawData = 0;
		}
		FORCEINLINE bool operator==(const FEdge& Other) const
		{
			return RawData == Other.RawData;
		}
	};
	TVoxelArray<FEdge> Edges;

	TVoxelArray<int32> Indices;
	TVoxelArray<FCell> VertexCells;
	TVoxelArray<FVector3f> Vertices;

	FVoxelCellGeneratorOutput CellGeneratorOutput;


private:
	void GenerateVertices();
	void GenerateTriangles();

private:
	struct FPermutation
	{
		FVoxelBitArray ValidVertices;
		TVoxelArray<int32> OldToNewVertices;
		int32 NewNumVertices = 0;

		void Update();
	};
	FPermutation GetPermutation() const;

private:
	TVoxelArray<int32> ComputeIndices(const FPermutation& Permutation) const;
	TVoxelArray<FVector3f> ComputeVertices(const FPermutation& Permutation) const;
	FVoxelVectorBuffer ComputeNormals(const FPermutation& Permutation) const;
	TVoxelArray<FVoxelMesh::FCell> ComputeCells(const FPermutation& Permutation) const;

	FVoxelMesh::FLOD ComputeLOD(
		const FPermutation& Permutation,
		int32 RelativeLOD) const;

private:
	TSharedPtr<FVoxelMesh> Finalize();
};