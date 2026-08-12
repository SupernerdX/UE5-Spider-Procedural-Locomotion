// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMesher.h"
#include "VoxelMesh.h"
#include "VoxelQuery.h"
#include "VoxelCellGenerator.h"
#include "VoxelDistanceFieldWrapper.h"
#include "VoxelGraphPositionParameter.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "VoxelMesherDistanceFieldBuilder.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"

FVoxelMesher::FVoxelMesher(
	FVoxelLayers& Layers,
	FVoxelSurfaceTypeTable& SurfaceTypeTable,
	FVoxelDependencyCollector& DependencyCollector,
	const FVoxelWeakStackLayer& WeakLayer,
	const int32 ChunkLOD,
	const FInt64Vector& ChunkOffset,
	const int32 VoxelSize,
	const int32 ChunkSize,
	const FTransform& LocalToWorld,
	const FVoxelMegaMaterialProxy& MegaMaterialProxy,
	const FVoxelFloatMetadataRef BlockinessMetadata,
	const bool bGenerateDistanceFields,
	const float MeshDistanceFieldBias,
	const int32 MeshDistanceFieldBricksPerChunk)
	: Layers(Layers)
	, SurfaceTypeTable(SurfaceTypeTable)
	, DependencyCollector(DependencyCollector)
	, WeakLayer(WeakLayer)
	, ChunkLOD(ChunkLOD)
	, ChunkOffset(ChunkOffset)
	, VoxelSize(VoxelSize)
	, ChunkSize(ChunkSize)
	, LocalToWorld(LocalToWorld)
	, MegaMaterialProxy(MegaMaterialProxy)
	, BlockinessMetadata(BlockinessMetadata)
	, bGenerateDistanceFields(bGenerateDistanceFields)
	, MeshDistanceFieldBias(MeshDistanceFieldBias)
	, MeshDistanceFieldBricksPerChunk(MeshDistanceFieldBricksPerChunk)
	// We need edges to have accurate mesh normals & for Lumen
	, DataSize(ChunkSize + 4)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelMesh> FVoxelMesher::CreateMesh(const TSharedPtr<const FVoxelCellGeneratorHeights>& CachedHeights)
{
	VOXEL_SCOPE_COUNTER_FORMAT("FVoxelMesher::CreateMesh ChunkSize=%d", ChunkSize);

	const FVector Start = FVector(ChunkOffset - (1 << ChunkLOD)) * VoxelSize;
	const float Step = float(1 << ChunkLOD) * VoxelSize;
	const FVoxelBox Bounds = FVoxelBox(Start, Start + FVector(DataSize) * Step);

	const FVoxelQuery Query(
		ChunkLOD,
		Layers,
		SurfaceTypeTable,
		DependencyCollector);

	if (!Query.HasStamps(
		WeakLayer,
		Bounds,
		EVoxelStampBehavior::AffectShape))
	{
		return {};
	}

	// Reserve AFTER checking HasStamps
	{
		VOXEL_SCOPE_COUNTER("Reserve");

		const int32 EstimatedNumCells = 4 * FMath::Square(ChunkSize + 2);

		CellToVertexIndex.Reserve(EstimatedNumCells);
		Edges.Reserve(3 * EstimatedNumCells);

		VertexCells.Reserve(EstimatedNumCells);
		Vertices.Reserve(EstimatedNumCells);
	}

	ensure(!CellGenerator);
	CellGenerator = MakeShared<FVoxelCellGenerator>(
		ChunkLOD,
		Layers,
		SurfaceTypeTable,
		FVector(ChunkOffset - (1 << ChunkLOD)) * VoxelSize,
		FIntVector(DataSize),
		float(1 << ChunkLOD) * VoxelSize,
		WeakLayer,
		CachedHeights);

	GenerateVertices();
	GenerateTriangles();

	return Finalize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMesher::GenerateVertices()
{
	VOXEL_FUNCTION_COUNTER();

	CellGenerator->ForeachCell(
		DependencyCollector,
		[&](
		FIntVector Position,
		const TVoxelStaticArray<float, 8>& CellDistances)
		{
			Position -= FIntVector(1);

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
					checkVoxelSlow(FVoxelUtilities::IsValidINT16(Position.X));
					checkVoxelSlow(FVoxelUtilities::IsValidINT16(Position.Y));
					checkVoxelSlow(FVoxelUtilities::IsValidINT16(Position.Z));

					FEdge Edge;
					Edge.X = Position.X;
					Edge.Y = Position.Y;
					Edge.Z = Position.Z;
					Edge.Direction = Direction;
					Edge.Sign = DistanceA < 0;
					Edge.MortonCode =
						(FMath::MortonCode3(Position.X) << 0) |
						(FMath::MortonCode3(Position.Y) << 1) |
						(FMath::MortonCode3(Position.Z) << 2);

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

			const FVector3f Vertex = FVector3f(Position) + Alpha;

			const int32 VertexIndex = Vertices.Add(Vertex);
			ensureVoxelSlow(VertexIndex == VertexCells.Add(FCell(Position)));

			CellToVertexIndex.Add_CheckNew(FCell(Position), VertexIndex);
		},
		bGenerateDistanceFields
		? &CellGeneratorOutput
		: nullptr);
}

void FVoxelMesher::GenerateTriangles()
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER_FORMAT("Num Edges = %d", Edges.Num());

	Indices.Reserve(6 * Edges.Num());

	{
		VOXEL_SCOPE_COUNTER("Sort");

		// Sort by morton order to improve Nanite clusters

		Edges.Sort([](const FEdge& A, const FEdge& B)
		{
			return A.MortonCode < B.MortonCode;
		});
	}

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

			const int32* VertexIndex = CellToVertexIndex.Find(FCell(Position));
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

		// Always add triangles if BlockinessMetadata is set, as they might end up being extended by the blockiness
		// TODO Better logic here? Still clean up the mesh at the end?

		if (Index01 != -1)
		{
			const FVector3f Vertex01 = Vertices[Index01];

			if (BlockinessMetadata.IsValid() ||
				FVoxelUtilities::IsTriangleValid(Vertex00, Vertex11, Vertex01))
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

			if (BlockinessMetadata.IsValid() ||
				FVoxelUtilities::IsTriangleValid(Vertex00, Vertex10, Vertex11))
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMesher::FPermutation::Update()
{
	VOXEL_FUNCTION_COUNTER();

	NewNumVertices = ValidVertices.CountSetBits();

	FVoxelUtilities::SetNumFast(OldToNewVertices, ValidVertices.Num());
	FVoxelUtilities::SetAll(OldToNewVertices, -1);

	int32 WriteIndex = 0;
	for (const int32 ReadIndex : ValidVertices.IterateSetBits())
	{
		OldToNewVertices[ReadIndex] = WriteIndex++;
	}
	checkVoxelSlow(WriteIndex == NewNumVertices);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMesher::FPermutation FVoxelMesher::GetPermutation() const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelBitArray ValidVertices;
	ValidVertices.SetNum(Vertices.Num(), true);

	// We need edges to have accurate mesh normals & for Lumen
	for (int32 Index = 0; Index < Vertices.Num(); Index++)
	{
		const FCell Cell = VertexCells[Index];

		if (Cell.X < 0 ||
			Cell.Y < 0 ||
			Cell.Z < 0 ||
			Cell.X > ChunkSize ||
			Cell.Y > ChunkSize ||
			Cell.Z > ChunkSize)
		{
			ValidVertices[Index] = false;
		}
	}

	{
		FVoxelBitArray UsedVertices;
		UsedVertices.SetNum(Vertices.Num(), false);

		checkVoxelSlow(Indices.Num() % 3 == 0);
		for (int32 Index = 0; Index < Indices.Num() / 3; Index++)
		{
			const int32 IndexA = Indices[3 * Index + 0];
			const int32 IndexB = Indices[3 * Index + 1];
			const int32 IndexC = Indices[3 * Index + 2];

			if (!ValidVertices[IndexA] ||
				!ValidVertices[IndexB] ||
				!ValidVertices[IndexC])
			{
				continue;
			}

			UsedVertices[IndexA] = true;
			UsedVertices[IndexB] = true;
			UsedVertices[IndexC] = true;
		}

		ValidVertices.BitwiseAnd(UsedVertices);
	}

	FPermutation Result;
	Result.ValidVertices = MoveTemp(ValidVertices);
	Result.Update();
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelArray<int32> FVoxelMesher::ComputeIndices(const FPermutation& Permutation) const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<int32> NewIndices;
	NewIndices.Reserve(Indices.Num());

	checkVoxelSlow(Indices.Num() % 3 == 0);
	for (int32 Index = 0; Index < Indices.Num() / 3; Index++)
	{
		const int32 IndexA = Indices[3 * Index + 0];
		const int32 IndexB = Indices[3 * Index + 1];
		const int32 IndexC = Indices[3 * Index + 2];

		const int32 NewIndexA = Permutation.OldToNewVertices[IndexA];
		const int32 NewIndexB = Permutation.OldToNewVertices[IndexB];
		const int32 NewIndexC = Permutation.OldToNewVertices[IndexC];

		if (NewIndexA == -1 ||
			NewIndexB == -1 ||
			NewIndexC == -1)
		{
			continue;
		}

		const FCell VertexA = VertexCells[IndexA];
		const FCell VertexB = VertexCells[IndexB];
		const FCell VertexC = VertexCells[IndexC];

		const bool bVertexAOutside =
			VertexA.X >= ChunkSize ||
			VertexA.Y >= ChunkSize ||
			VertexA.Z >= ChunkSize;

		const bool bVertexBOutside =
			VertexB.X >= ChunkSize ||
			VertexB.Y >= ChunkSize ||
			VertexB.Z >= ChunkSize;

		const bool bVertexCOutside =
			VertexC.X >= ChunkSize ||
			VertexC.Y >= ChunkSize ||
			VertexC.Z >= ChunkSize;

		if (bVertexAOutside &&
			bVertexBOutside &&
			bVertexCOutside)
		{
			continue;
		}

		NewIndices.Add_EnsureNoGrow(NewIndexA);
		NewIndices.Add_EnsureNoGrow(NewIndexB);
		NewIndices.Add_EnsureNoGrow(NewIndexC);
	}

	return NewIndices;
}

TVoxelArray<FVector3f> FVoxelMesher::ComputeVertices(const FPermutation& Permutation) const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<FVector3f> NewVertices;
	NewVertices.Reserve(Permutation.NewNumVertices);

	for (const int32 Index : Permutation.ValidVertices.IterateSetBits())
	{
		NewVertices.Add_EnsureNoGrow(Vertices[Index]);
	}

	return NewVertices;
}

FVoxelVectorBuffer FVoxelMesher::ComputeNormals(const FPermutation& Permutation) const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<FVector3f> Normals;
	FVoxelUtilities::SetNumZeroed(Normals, Vertices.Num());

	checkVoxelSlow(Indices.Num() % 3 == 0);
	for (int32 Index = 0; Index < Indices.Num() / 3; Index++)
	{
		const int32 IndexA = Indices[3 * Index + 0];
		const int32 IndexB = Indices[3 * Index + 1];
		const int32 IndexC = Indices[3 * Index + 2];

		const FVector3f Normal = FVoxelUtilities::GetTriangleNormal(
			Vertices[IndexA],
			Vertices[IndexB],
			Vertices[IndexC]);

		Normals[IndexA] += Normal;
		Normals[IndexB] += Normal;
		Normals[IndexC] += Normal;
	}

	FVoxelVectorBuffer Result;
	Result.Allocate(Permutation.NewNumVertices);

	int32 WriteIndex = 0;
	for (const int32 Index : Permutation.ValidVertices.IterateSetBits())
	{
		Result.Set(WriteIndex++, Normals[Index].GetSafeNormal());
	}
	checkVoxelSlow(WriteIndex == Result.Num());

	return Result;
}

TVoxelArray<FVoxelMesh::FCell> FVoxelMesher::ComputeCells(const FPermutation& Permutation) const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<FVoxelMesh::FCell> Result;
	Result.Reserve(Permutation.NewNumVertices);

	for (const int32 Index : Permutation.ValidVertices.IterateSetBits())
	{
		const FCell Cell = VertexCells[Index];
		Result.Add_EnsureNoGrow({ Cell.X, Cell.Y, Cell.Z });
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMesh::FLOD FVoxelMesher::ComputeLOD(
	const FPermutation& Permutation,
	const int32 RelativeLOD) const
{
	VOXEL_SCOPE_COUNTER_FORMAT("FVoxelMesher::ComputeLOD RelativeLOD=%d", RelativeLOD);
	checkVoxelSlow(RelativeLOD >= 1);

	TVoxelLinkedArray<int32> VertexIndicesArray;

	struct FParentCell
	{
		FIntVector Position;
		TVoxelStaticArray<int32, 8> DistanceIndices{ ForceInit };
		TVoxelLinkedArrayHandle<int32> VertexIndices;
	};
	TVoxelArray<FParentCell> ParentCells;
	ParentCells.Reserve(ChunkSize * 32);
	{
		VOXEL_SCOPE_COUNTER_NUM("ParentCells", Permutation.NewNumVertices);

		TVoxelMap<FIntVector, int32> ParentCellToIndex;
		ParentCellToIndex.Reserve(ChunkSize * 32);

		const int32 Step = 1 << (RelativeLOD - 1);
		const int32 ChunkSizeMinusStep = ChunkSize - Step;

		for (const int32 OldVertexIndex : Permutation.ValidVertices.IterateSetBits())
		{
			const FCell Cell = VertexCells[OldVertexIndex];

			const bool bIsOnEdge =
				Cell.X < Step ||
				Cell.Y < Step ||
				Cell.Z < Step ||
				Cell.X > ChunkSizeMinusStep ||
				Cell.Y > ChunkSizeMinusStep ||
				Cell.Z > ChunkSizeMinusStep;

			if (!bIsOnEdge)
			{
				continue;
			}

			const int32 VertexIndex = Permutation.OldToNewVertices[OldVertexIndex];
			checkVoxelSlow(VertexIndex != -1);

			const FIntVector ParentCellPosition = FVoxelUtilities::DivideFloor_FastLog2(FIntVector(Cell.X, Cell.Y, Cell.Z), RelativeLOD);
			const uint32 Hash = ParentCellToIndex.HashValue(ParentCellPosition);

			if (const int32* ParentCellIndex = ParentCellToIndex.FindHashed(Hash, ParentCellPosition))
			{
				checkVoxelSlow(ParentCells[*ParentCellIndex].Position == ParentCellPosition);

				VertexIndicesArray.AddTo(
					ParentCells[*ParentCellIndex].VertexIndices,
					VertexIndex);

				continue;
			}

			ParentCellToIndex.AddHashed_CheckNew(Hash, ParentCellPosition, ParentCells.Num());

			FParentCell& ParentCell = ParentCells.Emplace_GetRef();
			ParentCell.Position = ParentCellPosition;
			ParentCell.VertexIndices = VertexIndicesArray.Allocate();
			VertexIndicesArray.AddTo(ParentCell.VertexIndices, VertexIndex);
		}
	}

	if (ParentCells.Num() == 0)
	{
		return {};
	}

	FVoxelDoubleVectorBuffer Positions;
	{
		VOXEL_SCOPE_COUNTER_NUM("Build Positions", ParentCells.Num());

		Positions.Allocate(8 * ParentCells.Num());

		TVoxelMap<FIntVector, int32> PositionToIndex;
		PositionToIndex.Reserve(8 * ParentCells.Num());

		const int32 Step = 1 << (ChunkLOD + RelativeLOD);

		int32 WriteIndex = 0;
		for (FParentCell& ParentCell : ParentCells)
		{
			for (int32 Corner = 0; Corner < 8; Corner++)
			{
				const FIntVector Position
				{
					ParentCell.Position.X + bool(Corner & 0x1),
					ParentCell.Position.Y + bool(Corner & 0x2),
					ParentCell.Position.Z + bool(Corner & 0x4)
				};

				const uint32 Hash = PositionToIndex.HashValue(Position);

				if (const int32* Index = PositionToIndex.FindHashed(Hash, Position))
				{
					ParentCell.DistanceIndices[Corner] = *Index;
					continue;
				}

				const FVector FinalPosition = FVector(ChunkOffset + FInt64Vector3(Position) * Step) * VoxelSize;

				Positions.Set(WriteIndex, FinalPosition);

				ParentCell.DistanceIndices[Corner] = WriteIndex;
				PositionToIndex.AddHashed_CheckNew_EnsureNoGrow(Hash, Position, WriteIndex);

				WriteIndex++;
			}
		}

		Positions.ShrinkTo(WriteIndex);
	}

	const FVoxelQuery Query(
		ChunkLOD + RelativeLOD,
		Layers,
		SurfaceTypeTable,
		DependencyCollector);

	FVoxelFloatBuffer Blockiness;
	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelFloatBuffer>> MetadataToBuffer;

	if (BlockinessMetadata)
	{
		Blockiness.AllocateZeroed(Positions.Num());
		MetadataToBuffer.Add_EnsureNew(BlockinessMetadata, MakeSharedCopy(Blockiness));
	}

	const FVoxelFloatBuffer ParentDistances = Query.SampleVolumeLayer(
		WeakLayer,
		Positions,
		{},
		MetadataToBuffer);

	VOXEL_SCOPE_COUNTER("Build LOD");

	FVoxelMesh::FLOD LOD;
	FVoxelUtilities::SetNumFast(LOD.VertexIndexToDisplacedVertexIndex, Permutation.NewNumVertices);
	FVoxelUtilities::SetAll(LOD.VertexIndexToDisplacedVertexIndex, -1);
	LOD.DisplacedVertices.Reserve(ParentCells.Num());

	for (const FParentCell& ParentCell : ParentCells)
	{
		TVoxelStaticArray<float, 8> CellDistances{ NoInit };
		for (int32 Corner = 0; Corner < 8; Corner++)
		{
			CellDistances[Corner] = ParentDistances[ParentCell.DistanceIndices[Corner]];
		}

		if (FVoxelUtilities::IsNaN(CellDistances[0]) ||
			FVoxelUtilities::IsNaN(CellDistances[1]) ||
			FVoxelUtilities::IsNaN(CellDistances[2]) ||
			FVoxelUtilities::IsNaN(CellDistances[3]) ||
			FVoxelUtilities::IsNaN(CellDistances[4]) ||
			FVoxelUtilities::IsNaN(CellDistances[5]) ||
			FVoxelUtilities::IsNaN(CellDistances[6]) ||
			FVoxelUtilities::IsNaN(CellDistances[7]))
		{
			//ensureVoxelSlow(false);
			continue;
		}

		if ((CellDistances[0] >= 0) == (CellDistances[1] >= 0) &&
			(CellDistances[0] >= 0) == (CellDistances[2] >= 0) &&
			(CellDistances[0] >= 0) == (CellDistances[3] >= 0) &&
			(CellDistances[0] >= 0) == (CellDistances[4] >= 0) &&
			(CellDistances[0] >= 0) == (CellDistances[5] >= 0) &&
			(CellDistances[0] >= 0) == (CellDistances[6] >= 0) &&
			(CellDistances[0] >= 0) == (CellDistances[7] >= 0))
		{
			//ensureVoxelSlow(false);
			continue;
		}

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

		FVector3f Vertex = FVector3f(ParentCell.Position) + Alpha;

		if (BlockinessMetadata)
		{
			float FinalBlockiness = 0.f;
			for (int32 Corner = 0; Corner < 8; Corner++)
			{
				FinalBlockiness += Blockiness[ParentCell.DistanceIndices[Corner]];
			}
			FinalBlockiness /= 8.f;

			Vertex = FMath::Lerp(
				Vertex,
				FVector3f(ParentCell.Position) + FVector3f(0.5f),
				FMath::Clamp(FinalBlockiness, 0.f, 1.f));
		}

		Vertex *= (1 << RelativeLOD);

 		const int32 DisplacedVertexIndex = LOD.DisplacedVertices.Add_EnsureNoGrow(Vertex);

		for (const int32 VertexIndex : VertexIndicesArray.Iterate(ParentCell.VertexIndices))
		{
			LOD.VertexIndexToDisplacedVertexIndex[VertexIndex] = DisplacedVertexIndex;
		}
	}

	return LOD;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelMesh> FVoxelMesher::Finalize()
{
	VOXEL_FUNCTION_COUNTER();

	if (Indices.Num() == 0)
	{
		return {};
	}

	FPermutation Permutation = GetPermutation();

	if (Permutation.NewNumVertices == 0)
	{
		return {};
	}

	FVoxelDoubleVectorBuffer QueryPositions;
	{
		VOXEL_SCOPE_COUNTER_NUM("QueryPositions", Permutation.NewNumVertices);

		QueryPositions.Allocate(Permutation.NewNumVertices);

		const float Step = 1 << ChunkLOD;

		int32 WriteIndex = 0;

		for (const int32 Index : Permutation.ValidVertices.IterateSetBits())
		{
			FVector Vertex = FVector(ChunkOffset) + FVector(Vertices[Index]) * Step;
			Vertex *= VoxelSize;

			QueryPositions.Set(WriteIndex++, Vertex);
		}

		checkVoxelSlow(WriteIndex == Permutation.NewNumVertices);
	}

	FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
	SurfaceTypes.AllocateZeroed(QueryPositions.Num());
	{
		VOXEL_SCOPE_COUNTER_NUM("SurfaceTypes", Permutation.NewNumVertices);

		FVoxelFloatBuffer Blockiness;
		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelFloatBuffer>> MetadataToBuffer;

		if (BlockinessMetadata)
		{
			Blockiness.AllocateZeroed(QueryPositions.Num());
			MetadataToBuffer.Add_EnsureNew(BlockinessMetadata, MakeSharedCopy(Blockiness));
		}

		const FVoxelQuery Query(
			ChunkLOD,
			Layers,
			SurfaceTypeTable,
			DependencyCollector);

		(void)Query.SampleVolumeLayer(
			WeakLayer,
			QueryPositions,
			SurfaceTypes.View(),
			MetadataToBuffer);

		if (BlockinessMetadata)
		{
			VOXEL_SCOPE_COUNTER("Blockiness");

			for (const auto& It : CellToVertexIndex)
			{
				const int32 OldIndex = It.Value;
				const int32 NewIndex = Permutation.OldToNewVertices[OldIndex];
				if (NewIndex == -1)
				{
					continue;
				}

				const FVector3f NewPosition = FVector3f(It.Key.X, It.Key.Y, It.Key.Z) + 0.5f;

				Vertices[OldIndex] = FMath::Lerp(Vertices[OldIndex], NewPosition, FMath::Clamp(Blockiness[NewIndex], 0.f, 1.f));
			}
		}
	}

	FVoxelVectorBuffer Normals = ComputeNormals(Permutation);

	FVoxelSmartSurfaceTypeResolver Resolver(
		ChunkLOD,
		WeakLayer,
		Layers,
		SurfaceTypeTable,
		DependencyCollector,
		QueryPositions,
		Normals,
		SurfaceTypes.View());

	Resolver.Resolve();

	TVoxelInlineSet<FVoxelSurfaceType, 32> UsedSurfaceTypesSet;
	TVoxelArray<FVoxelSurfaceType> UsedSurfaceTypes;
	{
		VOXEL_SCOPE_COUNTER("UsedSurfaces");

		for (const FVoxelSurfaceTypeBlend& SurfaceType : SurfaceTypes)
		{
			for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceType.GetLayers())
			{
				UsedSurfaceTypesSet.Add(Layer.Type);
			};
		}

		UsedSurfaceTypes = UsedSurfaceTypesSet.Array();
		UsedSurfaceTypes.Sort();
	}

	check(SurfaceTypes.Num() == Normals.Num());

	DependencyCollector.AddDependency(*SurfaceTypeTable.InvisibleSurfaceTypesDependency);

	if (SurfaceTypeTable.InvisibleSurfaceTypes.Num() > 0 &&
		UsedSurfaceTypesSet.Contains(SurfaceTypeTable.InvisibleSurfaceTypes))
	{
		VOXEL_SCOPE_COUNTER("InvisibleSurfaceTypes");

		int32 ReadIndex = 0;
		int32 WriteIndex = 0;
		for (const int32 Index : Permutation.ValidVertices.IterateSetBits())
		{
			const FVoxelSurfaceTypeBlend& SurfaceType = SurfaceTypes[ReadIndex];

			if (SurfaceTypeTable.InvisibleSurfaceTypes.Contains(SurfaceType.GetTopLayer().Type))
			{
				Permutation.ValidVertices[Index] = false;
				ReadIndex++;
				continue;
			}

			if (WriteIndex != ReadIndex)
			{
				QueryPositions.Set(WriteIndex, QueryPositions[ReadIndex]);
				SurfaceTypes.Set(WriteIndex, SurfaceType);
				Normals.Set(WriteIndex, Normals[ReadIndex]);
			}

			WriteIndex++;
			ReadIndex++;
		}

		if (WriteIndex != SurfaceTypes.Num())
		{
			QueryPositions.ShrinkTo(WriteIndex);
			SurfaceTypes.ShrinkTo(WriteIndex);
			Normals.ShrinkTo(WriteIndex);
			Permutation.Update();
		}
	}

	if (Permutation.NewNumVertices == 0)
	{
		return {};
	}

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
	INLINE_LAMBDA
	{
		if (!bQueryMetadata)
		{
			return;
		}

		VOXEL_SCOPE_COUNTER("Metadata");

		TVoxelSet<FVoxelMetadataRef> MetadataRefs;
		MetadataRefs.Reserve(32);

		for (const FVoxelSurfaceType SurfaceType : UsedSurfaceTypes)
		{
			const FVoxelMaterialRenderIndex RenderIndex = MegaMaterialProxy.GetRenderIndex(SurfaceType);
			MetadataRefs.Append(MegaMaterialProxy.GetUsedMetadatas(RenderIndex));
		}

		if (MetadataRefs.Num() == 0)
		{
			return;
		}

		for (const FVoxelMetadataRef& MetadataRef : MetadataRefs)
		{
			MetadataToBuffer.Add_EnsureNew(
				MetadataRef,
				MetadataRef.MakeDefaultBuffer(QueryPositions.Num()));
		}

		const FVoxelQuery Query(
			ChunkLOD,
			Layers,
			SurfaceTypeTable,
			DependencyCollector);

		(void)Query.SampleVolumeLayer(
			WeakLayer,
			QueryPositions,
			{},
			MetadataToBuffer);
	};

	TVoxelArray<FVoxelMesh::FLOD> LODs;
	{
		VOXEL_SCOPE_COUNTER("Compute LODs");

		for (int32 RelativeLOD = 1; RelativeLOD <= MaxRelativeLOD; RelativeLOD++)
		{
			LODs.Add(ComputeLOD(Permutation, RelativeLOD));
		}
	}

	TVoxelArray<int32> FinalIndices = ComputeIndices(Permutation);
	if (FinalIndices.Num() == 0)
	{
		return {};
	}

	const TSharedRef<FVoxelMesh> Mesh = MakeShared<FVoxelMesh>(
		ChunkLOD,
		ChunkOffset,
		ChunkSize,
		MoveTemp(UsedSurfaceTypes),
		MoveTemp(MetadataToBuffer),
		MoveTemp(FinalIndices),
		ComputeVertices(Permutation),
		FVoxelUtilities::MakeOctahedrons(
			Normals.X.View(),
			Normals.Y.View(),
			Normals.Z.View()),
		SurfaceTypes.View().Array(),
		ComputeCells(Permutation),
		MoveTemp(LODs));

	if (bGenerateDistanceFields)
	{
		Mesh->DistanceFieldsData = FVoxelMesherDistanceFieldBuilder::Build(
			ChunkLOD,
			ChunkOffset,
			VoxelSize,
			ChunkSize,
			MeshDistanceFieldBias,
			MeshDistanceFieldBricksPerChunk,
			DataSize,
			CellGeneratorOutput);
	}

	Mesh->UpdateStats();
	return Mesh;
}