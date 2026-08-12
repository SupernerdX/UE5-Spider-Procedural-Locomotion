// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPointGenerator.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "TransvoxelData.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"

FVoxelPointGenerator::FVoxelPointGenerator(
	const TSharedRef<FVoxelDependencyCollector>& DependencyCollector,
	const TSharedRef<FVoxelLayers>& Layers,
	const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
	const FVector& Start,
	const FIntVector& Size,
	const float CellSize,
	const FVoxelWeakStackLayer& WeakLayer,
	const int32 LOD,
	const bool bResolveSmartSurfaceTypes)
	: DependencyCollector(DependencyCollector)
	, Layers(Layers)
	, SurfaceTypeTable(SurfaceTypeTable)
	, Start(Start)
	, Size(Size)
	, CellSize(CellSize)
	, WeakLayer(WeakLayer)
	, LOD(LOD)
	, bResolveSmartSurfaceTypes(bResolveSmartSurfaceTypes)
{
}

FVoxelFuture FVoxelPointGenerator::Initialize(const float Tolerance)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelShouldCancel ShouldCancel;
	if (ShouldCancel)
	{
		return {};
	}

	const FVoxelQuery Query(
		LOD,
		*Layers,
		*SurfaceTypeTable,
		*DependencyCollector);

	if (!Query.HasStamps(
		WeakLayer,
		FVoxelBox(Start, Start + FVector(Size) * CellSize),
		EVoxelStampBehavior::AffectShape))
	{
		return {};
	}

	SizeInChunks = FVoxelUtilities::DivideCeil(Size, ChunkSize);
	FVoxelUtilities::SetNum(Chunks, SizeInChunks.X * SizeInChunks.Y * SizeInChunks.Z);

	const FVoxelFloatBuffer Distances = Query.SampleVolumeLayer(
		WeakLayer,
		Start + CellSize * ChunkSize / 4,
		SizeInChunks * 2,
		CellSize * ChunkSize / 2);

	TVoxelArray<FVoxelFuture> Futures;

	for (int32 Z = 0; Z < SizeInChunks.Z; Z++)
	{
		for (int32 Y = 0; Y < SizeInChunks.Y; Y++)
		{
			for (int32 X = 0; X < SizeInChunks.X; X++)
			{
				if (ShouldCancel)
				{
					return {};
				}

				bool bValid = false;
				for (int32 ChildIndex = 0; ChildIndex < 8; ChildIndex++)
				{
					const int32 Index = FVoxelUtilities::Get3DIndex<int32>(
						SizeInChunks * 2,
						2 * X + bool(ChildIndex & 0x1),
						2 * Y + bool(ChildIndex & 0x2),
						2 * Z + bool(ChildIndex & 0x4));

					if (FVoxelUtilities::IsNaN(Distances[Index]) ||
						FMath::Abs(Distances[Index]) >= CellSize * ChunkSize / 4 * UE_SQRT_2 * FMath::Max(1.f + Tolerance, 1.f))
					{
						continue;
					}

					bValid = true;
					break;
				}

				if (!bValid)
				{
					continue;
				}

				Futures.Add(Voxel::AsyncTask(MakeStrongPtrLambda(this, [=, this]
				{
					VOXEL_FUNCTION_COUNTER();

					FChunk& Chunk = Chunks[FVoxelUtilities::Get3DIndex<int32>(SizeInChunks, X, Y, Z)];

					Chunk.Size = FVoxelUtilities::ComponentMin(FIntVector(ChunkSize), Size - FIntVector(X, Y, Z) * ChunkSize);

					Chunk.Distances = Query.SampleVolumeLayer(
						WeakLayer,
						Start + FVector(X, Y, Z) * ChunkSize * CellSize,
						Chunk.Size,
						CellSize);
				})));
			}
		}
	}

	return FVoxelFuture(Futures);
}

TVoxelArray<FVoxelPointGenerator::FPoint> FVoxelPointGenerator::Generate(
	const FVoxelBox& Bounds,
	const int32 Seed,
	const float Looseness,
	const float Ratio,
	const bool bApplyDensityToPoints,
	const TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery,
	FVoxelSurfaceTypeBlendBuffer& OutSurfaceTypes,
	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>>& OutMetadataToBuffer)
{
	VOXEL_FUNCTION_COUNTER();

	const auto FillMetadatas = [&](const int32 NumItems)
	{
		for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
		{
			OutMetadataToBuffer.Add_EnsureNew(
				MetadataToQuery,
				MetadataToQuery.MakeDefaultBuffer(NumItems));
		}

	};

	const FVoxelShouldCancel ShouldCancel;
	if (ShouldCancel)
	{
		FillMetadatas(0);
		return {};
	}

	if (Chunks.Num() == 0)
	{
		FillMetadatas(0);
		return {};
	}

	TVoxelArray<FVoxelIntBox> AllChunkBounds;
	AllChunkBounds.Reserve(Chunks.Num());

	int32 MaxNumPoints = 0;
	for (int32 Z = 0; Z < SizeInChunks.Z; Z++)
	{
		for (int32 Y = 0; Y < SizeInChunks.Y; Y++)
		{
			for (int32 X = 0; X < SizeInChunks.X; X++)
			{
				const FChunk& Chunk = Chunks[FVoxelUtilities::Get3DIndex<int32>(SizeInChunks, X, Y, Z)];
				if (Chunk.Distances.Num() == 0)
				{
					checkVoxelSlow(Chunk.Size == FIntVector(ForceInit));
					continue;
				}

				MaxNumPoints += Chunk.Size.X * Chunk.Size.Y * Chunk.Size.Z;

				AllChunkBounds.Add(
					FVoxelIntBox(0, Chunk.Size)
					.ShiftBy(FIntVector(X, Y, Z) * ChunkSize)
					.IntersectWith(FVoxelIntBox(0, Size - 1)));
			}
		}
	}

	TVoxelArray<FPoint> Points;
	Points.Reserve(MaxNumPoints);

	FVoxelVectorBuffer PointNormals;
	PointNormals.Allocate(MaxNumPoints);

	FVoxelDoubleVectorBuffer PointPositions;
	PointPositions.Allocate(MaxNumPoints);

	for (const FVoxelIntBox& ChunkBounds : AllChunkBounds)
	{
		if (ShouldCancel)
		{
			return {};
		}

		for (int32 Z = ChunkBounds.Min.Z; Z < ChunkBounds.Max.Z; Z++)
		{
			for (int32 Y = ChunkBounds.Min.Y; Y < ChunkBounds.Max.Y; Y++)
			{
				for (int32 X = ChunkBounds.Min.X; X < ChunkBounds.Max.X; X++)
				{
					const float Distance0 = GetDistance(X + 0, Y + 0, Z + 0);
					const float Distance1 = GetDistance(X + 1, Y + 0, Z + 0);
					const float Distance2 = GetDistance(X + 0, Y + 1, Z + 0);
					const float Distance3 = GetDistance(X + 1, Y + 1, Z + 0);
					const float Distance4 = GetDistance(X + 0, Y + 0, Z + 1);
					const float Distance5 = GetDistance(X + 1, Y + 0, Z + 1);
					const float Distance6 = GetDistance(X + 0, Y + 1, Z + 1);
					const float Distance7 = GetDistance(X + 1, Y + 1, Z + 1);

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

					int32 CellCode =
						((Distance0 >= 0) << 0) |
						((Distance1 >= 0) << 1) |
						((Distance2 >= 0) << 2) |
						((Distance3 >= 0) << 3) |
						((Distance4 >= 0) << 4) |
						((Distance5 >= 0) << 5) |
						((Distance6 >= 0) << 6) |
						((Distance7 >= 0) << 7);

					CellCode = ~CellCode & 0xFF;

					if (CellCode == 0 ||
						CellCode == 255)
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
						Distance7
					};

					const FVector CellPosition = Start + FVector(X, Y, Z) * CellSize;

					const int32 CellClass = Voxel::Transvoxel::GetCellClass(CellCode);
					const Voxel::Transvoxel::FCellIndices CellIndices = Voxel::Transvoxel::CellClassToCellIndices[CellClass];
					const Voxel::Transvoxel::FCellVertices CellVertices = Voxel::Transvoxel::CellCodeToCellVertices[CellCode];

					const int64 PointSeed = FVoxelUtilities::MurmurHashMulti(
						Seed,
						CellPosition.X,
						CellPosition.Y,
						CellPosition.Z);

					FRandomStream RandomSource(PointSeed);

					const float Chance = RandomSource.FRand();

					if (Chance >= Ratio)
					{
						continue;
					}

					TVoxelFixedArray<FVector3f, 12> Vertices;
					for (int32 CellVertexIndex = 0; CellVertexIndex < CellVertices.NumVertices(); CellVertexIndex++)
					{
						const Voxel::Transvoxel::FVertexData VertexData = CellVertices.GetVertexData(CellVertexIndex);

						// A: low point / B: high point
						const int32 IndexA = VertexData.IndexA;
						const int32 IndexB = VertexData.IndexB;
						checkVoxelSlow(0 <= IndexA && IndexA < 8);
						checkVoxelSlow(0 <= IndexB && IndexB < 8);

						const float DistanceA = CellDistances[IndexA];
						const float DistanceB = CellDistances[IndexB];
						ensureVoxelSlow((DistanceA >= 0) != (DistanceB >= 0));

						const int32 EdgeIndex = VertexData.EdgeIndex;
						checkVoxelSlow(0 <= EdgeIndex && EdgeIndex < 3);

						FVector3f Position = FVector3f(
							bool(IndexA & 1),
							bool(IndexA & 2),
							bool(IndexA & 4));
						Position[EdgeIndex] += DistanceA / (DistanceA - DistanceB);

						Vertices.Add(Position);
					}

					FVector3f BestPosition = FVector3f(ForceInit);
					float BestDistance = MAX_flt;

					FVector3f SamplePosition;
					SamplePosition.X = RandomSource.FRand() * Looseness;
					SamplePosition.Y = RandomSource.FRand() * Looseness;
					SamplePosition.Z = RandomSource.FRand() * Looseness;

					for (int32 Index = 0; Index < CellIndices.NumTriangles(); Index++)
					{
						const int32 IndexA = CellIndices.GetIndex(3 * Index + 0);
						const int32 IndexB = CellIndices.GetIndex(3 * Index + 1);
						const int32 IndexC = CellIndices.GetIndex(3 * Index + 2);

						const FVector3f VertexA = Vertices[IndexA];
						const FVector3f VertexB = Vertices[IndexB];
						const FVector3f VertexC = Vertices[IndexC];

						if (!FVoxelUtilities::IsTriangleValid(
							VertexA,
							VertexB,
							VertexC,
							0.001f))
						{
							continue;
						}

						FVector3f Barycentrics;
						const float Distance = FVoxelUtilities::PointTriangleDistanceSquared(
							SamplePosition,
							VertexA,
							VertexB,
							VertexC,
							Barycentrics);

						if (Distance > BestDistance)
						{
							continue;
						}

						BestPosition =
							Barycentrics.X * VertexA +
							Barycentrics.Y * VertexB +
							Barycentrics.Z * VertexC;

						BestDistance = Distance;
					}

					const FVector Position = CellPosition + FVector(BestPosition) * CellSize;

					if (!Bounds.Contains(Position))
					{
						// TODO? Currently creates a border
						//continue;
					}

					const FVector Alpha = (Position - CellPosition) / CellSize;
					ensureVoxelSlow(-KINDA_SMALL_NUMBER<Alpha.X && Alpha.X < 1.f + KINDA_SMALL_NUMBER);
					ensureVoxelSlow(-KINDA_SMALL_NUMBER<Alpha.Y && Alpha.Y < 1.f + KINDA_SMALL_NUMBER);
					ensureVoxelSlow(-KINDA_SMALL_NUMBER<Alpha.Z && Alpha.Z < 1.f + KINDA_SMALL_NUMBER);

					const float MinX = FVoxelUtilities::BilinearInterpolation(
						CellDistances[0b000],
						CellDistances[0b010],
						CellDistances[0b100],
						CellDistances[0b110],
						Alpha.Y,
						Alpha.Z);

					const float MaxX = FVoxelUtilities::BilinearInterpolation(
						CellDistances[0b001],
						CellDistances[0b011],
						CellDistances[0b101],
						CellDistances[0b111],
						Alpha.Y,
						Alpha.Z);

					const float MinY = FVoxelUtilities::BilinearInterpolation(
						CellDistances[0b000],
						CellDistances[0b001],
						CellDistances[0b100],
						CellDistances[0b101],
						Alpha.X,
						Alpha.Z);

					const float MaxY = FVoxelUtilities::BilinearInterpolation(
						CellDistances[0b010],
						CellDistances[0b011],
						CellDistances[0b110],
						CellDistances[0b111],
						Alpha.X,
						Alpha.Z);

					const float MinZ = FVoxelUtilities::BilinearInterpolation(
						CellDistances[0b000],
						CellDistances[0b001],
						CellDistances[0b010],
						CellDistances[0b011],
						Alpha.X,
						Alpha.Y);

					const float MaxZ = FVoxelUtilities::BilinearInterpolation(
						CellDistances[0b100],
						CellDistances[0b101],
						CellDistances[0b110],
						CellDistances[0b111],
						Alpha.X,
						Alpha.Y);

					const FVector3f Normal = FVector3f(
						MaxX - MinX,
						MaxY - MinY,
						MaxZ - MinZ).GetSafeNormal();

					const int32 Index = Points.Num();

					FPoint& Point = Points.Emplace_GetRef_EnsureNoGrow();
					Point.Position = Position;
					Point.Normal = Normal;
					Point.Seed = PointSeed;
					Point.Density = bApplyDensityToPoints ? (Ratio - Chance) / Ratio : 1.0f;

					PointNormals.Set(Index, Normal);
					PointPositions.Set(Index, Position);
				}
			}
		}
	}

	PointNormals.ShrinkTo(Points.Num());
	PointPositions.ShrinkTo(Points.Num());

	OutSurfaceTypes.AllocateZeroed(PointPositions.Num());

	FillMetadatas(PointPositions.Num());

	{
		VOXEL_SCOPE_COUNTER("Metadata");

		const FVoxelQuery Query(
			LOD,
			*Layers,
			*SurfaceTypeTable,
			*DependencyCollector);

		(void)Query.SampleVolumeLayer(
			WeakLayer,
			PointPositions,
			OutSurfaceTypes.View(),
			OutMetadataToBuffer);
	}

	if (bResolveSmartSurfaceTypes)
	{
		FVoxelSmartSurfaceTypeResolver Resolver(
			0,
			WeakLayer,
			*Layers,
			*SurfaceTypeTable,
			*DependencyCollector,
			PointPositions,
			PointNormals,
			OutSurfaceTypes.View());

		Resolver.Resolve();
	}

	return Points;
}