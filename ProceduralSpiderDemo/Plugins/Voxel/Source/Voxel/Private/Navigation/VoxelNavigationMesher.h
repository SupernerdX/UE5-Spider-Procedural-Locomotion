// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "Buffer/VoxelBaseBuffers.h"

struct FVoxelConfig;
struct FVoxelSubsystem;
class FVoxelNavigationMesh;

class FVoxelNavigationMesher : public TSharedFromThis<FVoxelNavigationMesher>
{
public:
	static constexpr int32 MaxRelativeLOD = 3;

	const FVoxelSubsystem& Subsystem;
	const FVoxelWeakStackLayer WeakLayer;
	const FVector ChunkStart;
	const int32 ChunkSize;
	const double VoxelSize;

	const int32 DataSize;

	explicit FVoxelNavigationMesher(
		const FVoxelSubsystem& Subsystem,
		const FVoxelWeakStackLayer& WeakLayer,
		const FVector& ChunkStart,
		int32 ChunkSize,
		double VoxelSize);

	TSharedRef<FVoxelNavigationMesh> CreateMesh();

private:
	FVoxelDependencyCollector DependencyCollector = FVoxelDependencyCollector(STATIC_FNAME("FVoxelNavigationMesher"));
	FVoxelFloatBuffer DistancesBuffer;
	TConstVoxelArrayView<float> Distances;

	struct alignas(8) FCell
	{
		int16 X;
		int16 Y;
		int16 Z;
		int16 Padding;

		FCell() = default;
		FCell(
			const int32 X,
			const int32 Y,
			const int32 Z)
			: X(X)
			, Y(Y)
			, Z(Z)
			, Padding(0)
		{
			checkVoxelSlow(FVoxelUtilities::IsValidINT16(X));
			checkVoxelSlow(FVoxelUtilities::IsValidINT16(Y));
			checkVoxelSlow(FVoxelUtilities::IsValidINT16(Z));
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

	struct alignas(8) FEdge
	{
		int16 X;
		int16 Y;
		int16 Z;
		uint8 Direction;
		uint8 Sign;

		FORCEINLINE FEdge()
		{
			ReinterpretCastRef<uint64>(*this) = 0;
		}

		FORCEINLINE bool operator==(const FEdge& Other) const
		{
			return ReinterpretCastRef<uint64>(*this) == ReinterpretCastRef<uint64>(Other);
		}
	};
	TVoxelArray<FEdge> Edges;

	TVoxelArray<int32> Indices;
	TVoxelArray<FVector3f> Vertices;

	void Generate();
	void GenerateVertices();
	void GenerateTriangles();
	void RemoveUnusedVertices();
};