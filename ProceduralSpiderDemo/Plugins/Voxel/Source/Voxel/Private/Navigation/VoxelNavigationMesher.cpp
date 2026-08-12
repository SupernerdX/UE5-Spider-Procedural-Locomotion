// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Navigation/VoxelNavigationMesher.h"
#include "Navigation/VoxelNavigationMesh.h"
#include "VoxelQuery.h"
#include "VoxelSubsystem.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"

FVoxelNavigationMesher::FVoxelNavigationMesher(
	const FVoxelSubsystem& Subsystem,
	const FVoxelWeakStackLayer& WeakLayer,
	const FVector& ChunkStart,
	const int32 ChunkSize,
	const double VoxelSize)
	: Subsystem(Subsystem)
	, WeakLayer(WeakLayer)
	, ChunkStart(ChunkStart)
	, ChunkSize(ChunkSize)
	, VoxelSize(VoxelSize)
	, DataSize(ChunkSize + 2)
{
	VOXEL_FUNCTION_COUNTER();

	const int32 EstimatedNumCells = 4 * FMath::Square(ChunkSize);

	CellToVertexIndex.Reserve(EstimatedNumCells);
	Edges.Reserve(3 * EstimatedNumCells);
	Vertices.Reserve(EstimatedNumCells);
}

TSharedRef<FVoxelNavigationMesh> FVoxelNavigationMesher::CreateMesh()
{
	VOXEL_FUNCTION_COUNTER();

	Generate();

	Indices.Shrink();
	Vertices.Shrink();

	return MakeShared<FVoxelNavigationMesh>(
		MoveTemp(Indices),
		MoveTemp(Vertices),
		Subsystem.Finalize(DependencyCollector));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNavigationMesher::Generate()
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelQuery Query(
		0,
		Subsystem.GetLayers(),
		Subsystem.GetSurfaceTypeTable(),
		DependencyCollector);

	DistancesBuffer = Query.SampleVolumeLayer(
		WeakLayer,
		ChunkStart,
		FIntVector(DataSize),
		VoxelSize);

	Distances = DistancesBuffer.View();

	GenerateVertices();

	if (Vertices.Num() == 0)
	{
		return;
	}

	GenerateTriangles();

	if (Indices.Num() == 0)
	{
		return;
	}

	RemoveUnusedVertices();
}

void FVoxelNavigationMesher::GenerateVertices()
{
	VOXEL_FUNCTION_COUNTER();

	for (int32 Z = 0; Z < ChunkSize + 1; Z++)
	{
		for (int32 Y = 0; Y < ChunkSize + 1; Y++)
		{
			const int32 BaseIndex = FVoxelUtilities::Get3DIndex<int32>(DataSize, 0, Y, Z);
			for (int32 X = 0; X < ChunkSize + 1; X++)
			{
				const int32 IndexOffset = BaseIndex + X;

#define INDEX(A, B, C) IndexOffset + A + B * DataSize + C * DataSize * DataSize

				checkVoxelSlow(INDEX(0, 0, 0) == FVoxelUtilities::Get3DIndex<int32>(DataSize, X + 0, Y + 0, Z + 0));
				checkVoxelSlow(INDEX(1, 0, 0) == FVoxelUtilities::Get3DIndex<int32>(DataSize, X + 1, Y + 0, Z + 0));
				checkVoxelSlow(INDEX(0, 1, 0) == FVoxelUtilities::Get3DIndex<int32>(DataSize, X + 0, Y + 1, Z + 0));
				checkVoxelSlow(INDEX(1, 1, 0) == FVoxelUtilities::Get3DIndex<int32>(DataSize, X + 1, Y + 1, Z + 0));
				checkVoxelSlow(INDEX(0, 0, 1) == FVoxelUtilities::Get3DIndex<int32>(DataSize, X + 0, Y + 0, Z + 1));
				checkVoxelSlow(INDEX(1, 0, 1) == FVoxelUtilities::Get3DIndex<int32>(DataSize, X + 1, Y + 0, Z + 1));
				checkVoxelSlow(INDEX(0, 1, 1) == FVoxelUtilities::Get3DIndex<int32>(DataSize, X + 0, Y + 1, Z + 1));
				checkVoxelSlow(INDEX(1, 1, 1) == FVoxelUtilities::Get3DIndex<int32>(DataSize, X + 1, Y + 1, Z + 1));

				const float Distance0 = Distances[INDEX(0, 0, 0)];
				const float Distance1 = Distances[INDEX(1, 0, 0)];
				const float Distance2 = Distances[INDEX(0, 1, 0)];
				const float Distance3 = Distances[INDEX(1, 1, 0)];
				const float Distance4 = Distances[INDEX(0, 0, 1)];
				const float Distance5 = Distances[INDEX(1, 0, 1)];
				const float Distance6 = Distances[INDEX(0, 1, 1)];
				const float Distance7 = Distances[INDEX(1, 1, 1)];

#undef INDEX

				if (Distance0 >= 0)
				{
					if (Distance1 >= 0 &&
						Distance2 >= 0 &&
						Distance3 >= 0 &&
						Distance4 >= 0 &&
						Distance5 >= 0 &&
						Distance6 >= 0 &&
						Distance7 >= 0)
					{
						continue;
					}
				}
				else
				{
					if (Distance1 < 0 &&
						Distance2 < 0 &&
						Distance3 < 0 &&
						Distance4 < 0 &&
						Distance5 < 0 &&
						Distance6 < 0 &&
						Distance7 < 0)
					{
						continue;
					}
				}

				if (FVoxelUtilities::IsNaN(Distance0) ||
					FVoxelUtilities::IsNaN(Distance1) ||
					FVoxelUtilities::IsNaN(Distance2) ||
					FVoxelUtilities::IsNaN(Distance3) ||
					FVoxelUtilities::IsNaN(Distance4) ||
					FVoxelUtilities::IsNaN(Distance5) ||
					FVoxelUtilities::IsNaN(Distance6) ||
					FVoxelUtilities::IsNaN(Distance7))
				{
					continue;
				}

				const TVoxelStaticArray<float, 8> CellDistances =
				{
					Distance0,
					Distance1,
					Distance2,
					Distance3,
					Distance4,
					Distance5,
					Distance6,
					Distance7,
				};

				int32 NumVertices = 0;
				FVector3f VertexSum = FVector3f(ForceInit);

				for (int32 EdgeIndex = 0; EdgeIndex < 12; EdgeIndex++)
				{
					const int32 Direction = EdgeIndex / 4;
					const int32 VertexIndex = EdgeIndex % 4;

					const int32 IndexA = INLINE_LAMBDA
					{
						switch (Direction)
						{
						default: VOXEL_ASSUME(false);
						case 0: return bool(VertexIndex & 0x0) + 2 * bool(VertexIndex & 0x1) + 4 * bool(VertexIndex & 0x2);
						case 1: return bool(VertexIndex & 0x1) + 2 * bool(VertexIndex & 0x0) + 4 * bool(VertexIndex & 0x2);
						case 2: return bool(VertexIndex & 0x1) + 2 * bool(VertexIndex & 0x2) + 4 * bool(VertexIndex & 0x0);
						}
					};
					const int32 IndexB = IndexA + (1 << Direction);

					const float DistanceA = CellDistances[IndexA];
					const float DistanceB = CellDistances[IndexB];

					if ((DistanceA >= 0) == (DistanceB >= 0))
					{
						continue;
					}

					if (IndexA == 0)
					{
						checkVoxelSlow(FVoxelUtilities::IsValidINT16(X));
						checkVoxelSlow(FVoxelUtilities::IsValidINT16(Y));
						checkVoxelSlow(FVoxelUtilities::IsValidINT16(Z));

						FEdge Edge;
						Edge.X = X;
						Edge.Y = Y;
						Edge.Z = Z;
						Edge.Direction = Direction;
						Edge.Sign = DistanceA < 0;

						checkVoxelSlow(!Edges.Contains(Edge));
						Edges.Add(Edge);
					}

					FVector3f Vertex = FVector3f(
						IndexA & 0x1 ? 1.f : 0.f,
						IndexA & 0x2 ? 1.f : 0.f,
						IndexA & 0x4 ? 1.f : 0.f);

					const float Alpha = DistanceA / (DistanceA - DistanceB);
					ensureVoxelSlow(0.f <= Alpha && Alpha <= 1.f);
					Vertex[Direction] = Alpha;

					NumVertices++;
					VertexSum += Vertex;
				}

				const FVector3f Alpha = VertexSum / NumVertices;
				ensureVoxelSlow(-KINDA_SMALL_NUMBER < Alpha.X && Alpha.X < 1.f + KINDA_SMALL_NUMBER);
				ensureVoxelSlow(-KINDA_SMALL_NUMBER < Alpha.Y && Alpha.Y < 1.f + KINDA_SMALL_NUMBER);
				ensureVoxelSlow(-KINDA_SMALL_NUMBER < Alpha.Z && Alpha.Z < 1.f + KINDA_SMALL_NUMBER);

				const FVector3f Vertex = FVector3f(X, Y, Z) + Alpha;

				const int32 VertexIndex = Vertices.Add(Vertex);

				CellToVertexIndex.Add_CheckNew(FCell(X, Y, Z), VertexIndex);
			}
		}
	}
}

void FVoxelNavigationMesher::GenerateTriangles()
{
	VOXEL_FUNCTION_COUNTER();

	Indices.Reserve(6 * Edges.Num());

	for (const FEdge& Edge : Edges)
	{
		const int32 AxisX = (Edge.Direction + 1) % 3;
		const int32 AxisY = (Edge.Direction + 2) % 3;

		FIntVector BasePosition(Edge.X, Edge.Y, Edge.Z);
		BasePosition[AxisX]--;
		BasePosition[AxisY]--;

		const auto FindVertex = [&](const int32 X, const int32 Y)
		{
			FIntVector Position = BasePosition;
			Position[AxisX] += X;
			Position[AxisY] += Y;

			const int32* VertexIndex = CellToVertexIndex.Find(FCell(Position.X, Position.Y, Position.Z));
			if (!VertexIndex)
			{
				// NaN or edge
				return -1;
			}

			return *VertexIndex;
		};

		const int32 Index00 = FindVertex(0, 0);
		const int32 Index11 = FindVertex(1, 1);

		if (Index00 == -1 ||
			Index11 == -1)
		{
			continue;
		}

		const FVector3f Vertex00 = Vertices[Index00];
		const FVector3f Vertex11 = Vertices[Index11];

		const int32 Index01 = FindVertex(1, 0);
		const int32 Index10 = FindVertex(0, 1);

		if (Index01 != -1)
		{
			const FVector3f Vertex01 = Vertices[Index01];

			if (FVoxelUtilities::IsTriangleValid(Vertex00, Vertex11, Vertex01))
			{
				if (Edge.Sign)
				{
					Indices.Add_EnsureNoGrow(Index00);
					Indices.Add_EnsureNoGrow(Index11);
					Indices.Add_EnsureNoGrow(Index01);
				}
				else
				{
					Indices.Add_EnsureNoGrow(Index01);
					Indices.Add_EnsureNoGrow(Index11);
					Indices.Add_EnsureNoGrow(Index00);
				}
			}
		}

		if (Index10 != -1)
		{
			const FVector3f Vertex10 = Vertices[Index10];

			if (FVoxelUtilities::IsTriangleValid(Vertex00, Vertex10, Vertex11))
			{
				if (Edge.Sign)
				{
					Indices.Add_EnsureNoGrow(Index00);
					Indices.Add_EnsureNoGrow(Index10);
					Indices.Add_EnsureNoGrow(Index11);
				}
				else
				{
					Indices.Add_EnsureNoGrow(Index11);
					Indices.Add_EnsureNoGrow(Index10);
					Indices.Add_EnsureNoGrow(Index00);
				}
			}
		}
	}
}

void FVoxelNavigationMesher::RemoveUnusedVertices()
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelBitArray ValidVertices;
	ValidVertices.SetNum(Vertices.Num(), false);

	for (const int32 Index : Indices)
	{
		ValidVertices[Index] = true;
	}

	const FVoxelSurfaceTypeTable& SurfaceTypeTable = Subsystem.GetSurfaceTypeTable();
	DependencyCollector.AddDependency(*SurfaceTypeTable.InvisibleSurfaceTypesDependency);

	if (SurfaceTypeTable.InvisibleSurfaceTypes.Num() > 0)
	{
		VOXEL_SCOPE_COUNTER("InvisibleSurfaceTypes");

		FVoxelDoubleVectorBuffer Positions;
		Positions.Allocate(Vertices.Num());

		{
			int32 WriteIndex = 0;
			for (const int32 Index : ValidVertices.IterateSetBits())
			{
				const FVector Vertex = ChunkStart + FVector(Vertices[Index]) * VoxelSize;

				Positions.Set(WriteIndex++, Vertex);
			}
			Positions.ShrinkTo(WriteIndex);
		}

		FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
		SurfaceTypes.AllocateZeroed(Positions.Num());

		const FVoxelQuery Query(
			0,
			Subsystem.GetLayers(),
			Subsystem.GetSurfaceTypeTable(),
			DependencyCollector);

		(void)Query.SampleVolumeLayer(
			Subsystem.GetConfig().LayerToRender,
			Positions,
			SurfaceTypes.View(),
			{});

		int32 SurfaceIndex = 0;
		for (const int32 Index : ValidVertices.IterateSetBits())
		{
			if (SurfaceTypeTable.InvisibleSurfaceTypes.Contains(SurfaceTypes[SurfaceIndex].GetTopLayer().Type))
			{
				ValidVertices[Index] = false;
			}

			SurfaceIndex++;
		}
	}

	TVoxelArray<int32> OldToNewVertices;
	FVoxelUtilities::SetNumFast(OldToNewVertices, Vertices.Num());

	int32 WriteIndex = 0;
	for (int32 Index = 0; Index < Vertices.Num(); Index++)
	{
		if (!ValidVertices[Index])
		{
			OldToNewVertices[Index] = -1;
			continue;
		}

		OldToNewVertices[Index] = WriteIndex;

		if (WriteIndex != Index)
		{
			Vertices[WriteIndex] = Vertices[Index];
		}

		WriteIndex++;
	}
	Vertices.SetNum(WriteIndex);

	TVoxelArray<int32> NewIndices;
	NewIndices.Reserve(Indices.Num());

	checkVoxelSlow(Indices.Num() % 3 == 0);
	for (int32 Index = 0; Index < Indices.Num() / 3; Index++)
	{
		const int32 IndexA = Indices[3 * Index + 0];
		const int32 IndexB = Indices[3 * Index + 1];
		const int32 IndexC = Indices[3 * Index + 2];

		const int32 NewIndexA = OldToNewVertices[IndexA];
		const int32 NewIndexB = OldToNewVertices[IndexB];
		const int32 NewIndexC = OldToNewVertices[IndexC];

		if (NewIndexA == -1 ||
			NewIndexB == -1 ||
			NewIndexC == -1)
		{
			continue;
		}

		NewIndices.Add_EnsureNoGrow(NewIndexA);
		NewIndices.Add_EnsureNoGrow(NewIndexB);
		NewIndices.Add_EnsureNoGrow(NewIndexC);
	}

	Indices = MoveTemp(NewIndices);
}