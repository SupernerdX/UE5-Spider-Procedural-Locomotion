// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterUtilities.h"
#include "Scatter/VoxelCubicFacePointGenerator.h"
#include "Math/Sobol.h"
#include "VoxelPointId.h"
#include "VoxelPointSet.h"
#include "TransvoxelData.h"
#include "VoxelCellGenerator.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Utilities/VoxelBufferMathUtilities.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "Utilities/VoxelBufferGradientUtilities.h"

FVoxelPointSet FVoxelScatterUtilities::ScatterPoints3D(
	const FVoxelQuery& Query,
	const FVoxelBox& Bounds,
	const TOptional<FVoxelBox>& OptionalInitialBounds,
	const float DistanceBetweenPoints,
	const uint64 Seed,
	const float Looseness,
	const float MinCellArea,
	const FVoxelWeakStackLayer& WeakLayer,
	const bool bQuerySurfaceTypes,
	const bool bResolveSurfaceTypes,
	const TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER_FORMAT("Size = %s", *Bounds.Size().ToString());

	const int32 ChunkSize = FMath::CeilToInt(FMath::Max(DistanceBetweenPoints * 16.f, 16.f));
	const FVoxelIntBox BoundsWithPadding = FVoxelIntBox::FromFloatBox_WithPadding(Bounds);

	const FVoxelIntBox ChunkedBounds = FVoxelIntBox(
		FVoxelUtilities::DivideFloor(BoundsWithPadding.Min, ChunkSize) * ChunkSize,
		FVoxelUtilities::DivideCeil(BoundsWithPadding.Max, ChunkSize) * ChunkSize);

	TVoxelChunkedArray<FVoxelPointId> ChunkedIds;
	TVoxelChunkedArray<double> ChunkedPositionsX;
	TVoxelChunkedArray<double> ChunkedPositionsY;
	TVoxelChunkedArray<double> ChunkedPositionsZ;
	TVoxelChunkedArray<float> ChunkedNormalsX;
	TVoxelChunkedArray<float> ChunkedNormalsY;
	TVoxelChunkedArray<float> ChunkedNormalsZ;
	{
		VOXEL_SCOPE_COUNTER("CellGenerator");

		const FVector Start = FVector(ChunkedBounds.Min);
		const FIntVector Size = FVoxelUtilities::CeilToInt(FVector(ChunkedBounds.Size()) / DistanceBetweenPoints);

		FVoxelCellGenerator CellGenerator(
			Query.LOD,
			Query.Layers,
			Query.SurfaceTypeTable,
			Start,
			Size,
			DistanceBetweenPoints,
			WeakLayer,
			nullptr);

		CellGenerator.ForeachCell(
			Query.DependencyCollector,
			[&](
			const FIntVector& CellPositionInBounds,
			const TVoxelStaticArray<float, 8>& CellDistances)
			{
				int32 CellCode =
					((CellDistances[0] >= 0) << 0) |
					((CellDistances[1] >= 0) << 1) |
					((CellDistances[2] >= 0) << 2) |
					((CellDistances[3] >= 0) << 3) |
					((CellDistances[4] >= 0) << 4) |
					((CellDistances[5] >= 0) << 5) |
					((CellDistances[6] >= 0) << 6) |
					((CellDistances[7] >= 0) << 7);

				CellCode = ~CellCode & 0xFF;
				checkVoxelSlow(CellCode != 0 && CellCode != 255);

				const FVector CellPosition = Start + FVector(CellPositionInBounds) * DistanceBetweenPoints;
				const uint64 PointSeed = FVoxelUtilities::MurmurHashMulti(
					Seed,
					CellPosition.X,
					CellPosition.Y,
					CellPosition.Z);

				const FRandomStream RandomStream(PointSeed);

				const int32 CellClass = Voxel::Transvoxel::GetCellClass(CellCode);
				const Voxel::Transvoxel::FCellIndices CellIndices = Voxel::Transvoxel::CellClassToCellIndices[CellClass];
				const Voxel::Transvoxel::FCellVertices CellVertices = Voxel::Transvoxel::CellCodeToCellVertices[CellCode];

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

				const FVector3f SamplePosition
				{
					RandomStream.FRand() * Looseness,
					RandomStream.FRand() * Looseness,
					RandomStream.FRand() * Looseness
				};

				double TriangleAreas = 0.;
				for (int32 Index = 0; Index < CellIndices.NumTriangles(); Index++)
				{
					const int32 IndexA = CellIndices.GetIndex(3 * Index + 0);
					const int32 IndexB = CellIndices.GetIndex(3 * Index + 1);
					const int32 IndexC = CellIndices.GetIndex(3 * Index + 2);

					const FVector3f VertexA = Vertices[IndexA];
					const FVector3f VertexB = Vertices[IndexB];
					const FVector3f VertexC = Vertices[IndexC];

					const float TriangleAreaSquared = FVoxelUtilities::GetTriangleAreaSquared(
						VertexA,
						VertexB,
						VertexC);
					if (TriangleAreaSquared < FMath::Square(0.001f))
					{
						continue;
					}

					TriangleAreas += TriangleAreaSquared;

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

				if (TriangleAreas < FMath::Square(MinCellArea))
				{
					return;
				}

				const FVector Position = CellPosition + FVector(BestPosition) * DistanceBetweenPoints;

				if (!Bounds.Contains(Position))
				{
					return;
				}

				const FVector Alpha = (Position - CellPosition) / DistanceBetweenPoints;
				ensureVoxelSlow(-KINDA_SMALL_NUMBER < Alpha.X && Alpha.X < 1.f + KINDA_SMALL_NUMBER);
				ensureVoxelSlow(-KINDA_SMALL_NUMBER < Alpha.Y && Alpha.Y < 1.f + KINDA_SMALL_NUMBER);
				ensureVoxelSlow(-KINDA_SMALL_NUMBER < Alpha.Z && Alpha.Z < 1.f + KINDA_SMALL_NUMBER);

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

				ChunkedIds.Add(PointSeed);

				ChunkedPositionsX.Add(Position.X);
				ChunkedPositionsY.Add(Position.Y);
				ChunkedPositionsZ.Add(Position.Z);

				ChunkedNormalsX.Add(Normal.X);
				ChunkedNormalsY.Add(Normal.Y);
				ChunkedNormalsZ.Add(Normal.Z);
			});
	}

	const int32 Num = ChunkedPositionsX.Num();
	checkVoxelSlow(Num == ChunkedPositionsY.Num());
	checkVoxelSlow(Num == ChunkedPositionsZ.Num());
	checkVoxelSlow(Num == ChunkedNormalsX.Num());
	checkVoxelSlow(Num == ChunkedNormalsY.Num());
	checkVoxelSlow(Num == ChunkedNormalsZ.Num());
	checkVoxelSlow(Num == ChunkedIds.Num());

	if (Num == 0)
	{
		return {};
	}

	FVoxelPointIdBuffer Ids;
	FVoxelDoubleVectorBuffer Positions;
	FVoxelVectorBuffer Normals;
	{
		VOXEL_SCOPE_COUNTER("Copy");

		Ids.Allocate(Num);
		Positions.Allocate(Num);
		Normals.Allocate(Num);

		ChunkedIds.CopyTo(Ids.View());

		ChunkedPositionsX.CopyTo(Positions.X.View());
		ChunkedPositionsY.CopyTo(Positions.Y.View());
		ChunkedPositionsZ.CopyTo(Positions.Z.View());

		ChunkedNormalsX.CopyTo(Normals.X.View());
		ChunkedNormalsY.CopyTo(Normals.Y.View());
		ChunkedNormalsZ.CopyTo(Normals.Z.View());
	}

	FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;

	if (bQuerySurfaceTypes ||
		MetadatasToQuery.Num() > 0)
	{
		VOXEL_SCOPE_COUNTER("SurfaceTypes & Metadata");

		if (bQuerySurfaceTypes)
		{
			SurfaceTypes.AllocateZeroed(Num);
		}

		{
			MetadataToBuffer.Reserve(MetadatasToQuery.Num());

			for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
			{
				if (!MetadataToQuery.IsValid())
				{
					continue;
				}

				MetadataToBuffer.Add_EnsureNew(
					MetadataToQuery,
					MetadataToQuery.MakeDefaultBuffer(Num));
			}
		}

		(void)Query.SampleVolumeLayer(
			WeakLayer,
			Positions,
			SurfaceTypes.View(),
			MetadataToBuffer);

		if (bQuerySurfaceTypes &&
			bResolveSurfaceTypes)
		{
			VOXEL_SCOPE_COUNTER("Smart surface types");

			FVoxelSmartSurfaceTypeResolver Resolver(
				0,
				WeakLayer,
				Query.Layers,
				Query.SurfaceTypeTable,
				Query.DependencyCollector,
				Positions,
				Normals,
				SurfaceTypes.View());

			Resolver.Resolve();
		}
	}

	FVoxelBoolBuffer IsNeighbor;
	if (OptionalInitialBounds.IsSet())
	{
		IsNeighbor.Allocate(Num);
		const FVoxelBox InitialBounds = OptionalInitialBounds.GetValue();
		for (int32 Index = 0; Index < Num; Index++)
		{
			IsNeighbor.Set(Index, !InitialBounds.Contains(Positions[Index]));
		}
	}

	FVoxelPointSet Points;
	Points.SetNum(Num);
	Points.Add(FVoxelPointAttributes::Id, MakeSharedCopy(MoveTemp(Ids)));
	Points.Add(FVoxelPointAttributes::Position, MakeSharedCopy(MoveTemp(Positions)));
	Points.Add(FVoxelPointAttributes::Rotation, MakeSharedCopy(FVoxelBufferMathUtilities::MakeQuaternionFromZ(Normals)));

	if (OptionalInitialBounds.IsSet())
	{
		Points.Add(FVoxelPointAttributes::IsNeighbor, MakeSharedCopy(MoveTemp(IsNeighbor)));
	}

	if (bQuerySurfaceTypes)
	{
		Points.Add(FVoxelPointAttributes::SurfaceTypes, MakeSharedCopy(MoveTemp(SurfaceTypes)));
	}

	for (const auto& It : MetadataToBuffer)
	{
		Points.Add(It.Key.GetFName(), It.Value);
	}

	return Points;
}

FVoxelPointSet FVoxelScatterUtilities::ScatterPoints2D(
	const FVoxelQuery& Query,
	const FVoxelBox& Bounds,
	const TOptional<FVoxelBox>& OptionalInitialBounds,
	float DistanceBetweenPoints,
	uint64 Seed,
	float Looseness,
	const EVoxelHeightScatterType Type,
	const FVoxelWeakStackLayer& WeakLayer,
	bool bQuerySurfaceTypes,
	bool bResolveSurfaceTypes,
	TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER_FORMAT("Size = %s", *Bounds.Size().ToString());

	FVoxelDoubleVector2DBuffer HeightQueryPositions;

	switch (Type)
	{
	case EVoxelHeightScatterType::Grid:
	{
		if (!GenerateGridPositions(
			Bounds,
			DistanceBetweenPoints,
			Looseness,
			Seed,
			HeightQueryPositions))
		{
			return {};
		}
		break;
	}
	case EVoxelHeightScatterType::Sobol:
	{
		if (!GenerateSobolPositions(
			Bounds,
			DistanceBetweenPoints,
			Seed,
			HeightQueryPositions))
		{
			return {};
		}
		break;
	}
	case EVoxelHeightScatterType::Halton:
	{
		if (!GenerateHaltonPositions(
			Bounds,
			DistanceBetweenPoints,
			Seed,
			HeightQueryPositions))
		{
			return {};
		}
		break;
	}
	}

	FVoxelSurfaceTypeBlendBuffer QueriedSurfaceTypes;
	QueriedSurfaceTypes.AllocateZeroed(HeightQueryPositions.Num());

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> QueriedMetadataToBuffer;
	QueriedMetadataToBuffer.Reserve(MetadatasToQuery.Num());

	for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
	{
		if (!MetadataToQuery.IsValid())
		{
			continue;
		}

		QueriedMetadataToBuffer.Add_EnsureNew(
			MetadataToQuery,
			MetadataToQuery.MakeDefaultBuffer(HeightQueryPositions.Num()));
	}

	const FVoxelFloatBuffer Heights = Query.SampleHeightLayer(
		WeakLayer,
		HeightQueryPositions,
		QueriedSurfaceTypes.View(),
		QueriedMetadataToBuffer);

	if (Voxel::ShouldCancel())
	{
		return {};
	}

	FVoxelPointIdBuffer PointIdBuffer;
	PointIdBuffer.Allocate(HeightQueryPositions.Num());
	FVoxelDoubleVectorBuffer PointPositions;
	PointPositions.Allocate(HeightQueryPositions.Num());
	FVoxelSurfaceTypeBlendBuffer PointSurfaceTypes;
	PointSurfaceTypes.AllocateZeroed(HeightQueryPositions.Num());

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> PointMetadataToBuffer;
	PointMetadataToBuffer.Reserve(MetadatasToQuery.Num());

	for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
	{
		PointMetadataToBuffer.Add_EnsureNew(
			MetadataToQuery,
			MetadataToQuery.MakeDefaultBuffer(HeightQueryPositions.Num()));
	}

	int32 NumPoints = 0;
	for (int32 Index = 0; Index < HeightQueryPositions.Num(); Index++)
	{
		const float Height = Heights[Index];

		if (FVoxelUtilities::IsNaN(Height) ||
			Height < Bounds.Min.Z ||
			Height > Bounds.Max.Z)
		{
			continue;
		}

		const int32 WriteIndex = NumPoints++;

		const FVector Position(HeightQueryPositions.X[Index], HeightQueryPositions.Y[Index], Heights[Index]);

		const uint64 PointSeed = FVoxelUtilities::MurmurHashMulti(
			Seed,
			Position.X,
			Position.Y,
			Position.Z);

		PointIdBuffer.Set(WriteIndex, PointSeed);
		PointPositions.Set(WriteIndex, Position);

		if (bQuerySurfaceTypes)
		{
			PointSurfaceTypes.Set(WriteIndex, QueriedSurfaceTypes[Index]);
		}

		for (const auto& It : QueriedMetadataToBuffer)
		{
			PointMetadataToBuffer[It.Key]->SetGeneric(WriteIndex, It.Value->GetGeneric(Index));
		}
	}

	PointIdBuffer.ShrinkTo(NumPoints);
	PointPositions.ShrinkTo(NumPoints);
	PointSurfaceTypes.ShrinkTo(NumPoints);
	for (const auto& It : PointMetadataToBuffer)
	{
		It.Value->ShrinkTo(NumPoints);
	}

	FVoxelVectorBuffer PointNormals;
	{
		VOXEL_SCOPE_COUNTER("Compute Gradient");

		// TODO?
		const float GradientStep = 100.f;

		FVoxelDoubleVector2DBuffer GradientPositions = FVoxelBufferGradientUtilities::SplitPositions2D(PointPositions, GradientStep);

		const FVoxelFloatBuffer GradientHeights = Query.SampleHeightLayer(WeakLayer, GradientPositions);

		PointNormals = FVoxelBufferGradientUtilities::CollapseGradient2DToNormal(GradientHeights, NumPoints, GradientStep);
	}

	if (bQuerySurfaceTypes &&
		bResolveSurfaceTypes)
	{
		VOXEL_SCOPE_COUNTER("Smart surface types");

		FVoxelSmartSurfaceTypeResolver Resolver(
			Query.LOD,
			WeakLayer,
			Query.Layers,
			Query.SurfaceTypeTable,
			Query.DependencyCollector,
			PointPositions,
			PointNormals,
			PointSurfaceTypes.View());

		Resolver.Resolve();
	}

	FVoxelBoolBuffer IsNeighbor;
	if (OptionalInitialBounds.IsSet())
	{
		IsNeighbor.Allocate(NumPoints);
		const FVoxelBox InitialBounds = OptionalInitialBounds.GetValue();
		for (int32 Index = 0; Index < NumPoints; Index++)
		{
			IsNeighbor.Set(Index, !InitialBounds.Contains(PointPositions[Index]));
		}
	}

	FVoxelPointSet Points;
	Points.SetNum(NumPoints);
	Points.Add(FVoxelPointAttributes::Id, MakeSharedCopy(MoveTemp(PointIdBuffer)));
	Points.Add(FVoxelPointAttributes::Position, MakeSharedCopy(MoveTemp(PointPositions)));
	Points.Add(FVoxelPointAttributes::Rotation, MakeSharedCopy(FVoxelBufferMathUtilities::MakeQuaternionFromZ(PointNormals)));

	if (OptionalInitialBounds.IsSet())
	{
		Points.Add(FVoxelPointAttributes::IsNeighbor, MakeSharedCopy(MoveTemp(IsNeighbor)));
	}

	if (bQuerySurfaceTypes)
	{
		Points.Add(FVoxelPointAttributes::SurfaceTypes, MakeSharedCopy(MoveTemp(PointSurfaceTypes)));
	}

	for (const auto& It : QueriedMetadataToBuffer)
	{
		Points.Add(It.Key.GetFName(), It.Value);
	}

	return Points;
}

FVoxelPointSet FVoxelScatterUtilities::ScatterPointsCubicFaces(
	const FVoxelQuery& Query,
	const FVoxelBox& Bounds,
	const int32 VoxelSize,
	const int32 ChunkSize,
	const uint64 Seed,
	const FVoxelWeakStackLayer& WeakLayer,
	const bool bQuerySurfaceTypes,
	const bool bResolveSurfaceTypes,
	const bool bSkipNaNFaces,
	const TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_COUNTER_FORMAT("Size = %s", *Bounds.Size().ToString());

	const FVoxelIntBox BoundsWithPadding = FVoxelIntBox::FromFloatBox_WithPadding(Bounds);

	// Divide bounds into chunks
	const int32 ChunkSizeWorld = ChunkSize * VoxelSize;
	const FVoxelIntBox ChunkedBounds = FVoxelIntBox(
		FVoxelUtilities::DivideFloor(BoundsWithPadding.Min, ChunkSizeWorld) * ChunkSizeWorld,
		FVoxelUtilities::DivideCeil(BoundsWithPadding.Max, ChunkSizeWorld) * ChunkSizeWorld);

	TVoxelArray<TSharedRef<const FVoxelPointSet>> ChunkPointSets;

	// Iterate over chunks
	for (int64 ChunkZ = ChunkedBounds.Min.Z; ChunkZ < ChunkedBounds.Max.Z; ChunkZ += ChunkSizeWorld)
	{
		for (int64 ChunkY = ChunkedBounds.Min.Y; ChunkY < ChunkedBounds.Max.Y; ChunkY += ChunkSizeWorld)
		{
			for (int64 ChunkX = ChunkedBounds.Min.X; ChunkX < ChunkedBounds.Max.X; ChunkX += ChunkSizeWorld)
			{
				const FInt64Vector ChunkOffset(
					ChunkX / VoxelSize,
					ChunkY / VoxelSize,
					ChunkZ / VoxelSize);

				FVoxelCubicFacePointGenerator Generator(
					Query.Layers,
					Query.SurfaceTypeTable,
					Query.DependencyCollector,
					WeakLayer,
					ChunkOffset,
					VoxelSize,
					ChunkSize,
					Seed,
					bQuerySurfaceTypes,
					bResolveSurfaceTypes,
					bSkipNaNFaces,
					MetadatasToQuery);

				FVoxelPointSet ChunkPoints = Generator.GeneratePoints();
				if (ChunkPoints.Num() > 0)
				{
					ChunkPointSets.Add(MakeSharedCopy(MoveTemp(ChunkPoints)));
				}
			}
		}
	}

	if (ChunkPointSets.Num() == 0)
	{
		return {};
	}

	if (ChunkPointSets.Num() == 1)
	{
		return FVoxelPointSet(*ChunkPointSets[0]);
	}

	return *FVoxelPointSet::Merge(MoveTemp(ChunkPointSets));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelScatterUtilities::PrepareChunksForPositionsGenerator(
	const FVoxelBox2D& Bounds,
	const double DistanceBetweenPositions,
	FVoxelDoubleVector2DBuffer& OutPositions,
	const TFunction<void(const FVoxelIntBox2D&, const int64, int64&)>& Lambda)
{
	const int32 ChunkSize = FMath::CeilToInt(FMath::Max(DistanceBetweenPositions * 16.f, 16.f));
	const FVoxelIntBox2D BoundsWithPadding = FVoxelIntBox2D::FromFloatBox_WithPadding(Bounds);

	TVoxelArray<FVoxelIntBox2D> Chunks;
	if (!ensure(BoundsWithPadding.Subdivide(ChunkSize, Chunks, false, 1024 * 1024)))
	{
		return false;
	}

	const double PositionArea = FMath::Square<double>(DistanceBetweenPositions);
	const int64 NumMaxPositions = FMath::CeilToInt64(int64(ChunkSize) * ChunkSize / PositionArea);

	if (NumMaxPositions * Chunks.Num() > 1024 * 1024)
	{
		return false;
	}

	OutPositions.Allocate(NumMaxPositions * Chunks.Num());

	int64 NumPositions = 0;
	for (const FVoxelIntBox2D& Chunk : Chunks)
	{
		Lambda(Chunk, NumMaxPositions, NumPositions);
	}
	OutPositions.ShrinkTo(NumPositions);

	return NumPositions > 0;
}

bool FVoxelScatterUtilities::GenerateHaltonPositions(
	const FVoxelBox& Bounds,
	const double DistanceBetweenPositions,
	const uint32 Seed,
	FVoxelDoubleVector2DBuffer& OutPositions)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelBox2D Bounds2D(Bounds);
	return PrepareChunksForPositionsGenerator(
		Bounds2D,
		DistanceBetweenPositions,
		OutPositions,
		[&](const FVoxelIntBox2D& Chunk, const int64 NumMaxPositions, int64& OutNumPosition)
		{
			uint32 SeedValue = FVoxelUtilities::MurmurHash(Seed);
			SeedValue ^= FVoxelUtilities::MurmurHash(Chunk);

			for (int32 Index = 0; Index < NumMaxPositions; Index++)
			{
				FVector2D Position = FVector2D::ZeroVector;
				Position.X = Chunk.Min.X + FVoxelUtilities::Halton<2>(SeedValue) * (Chunk.Max.X - Chunk.Min.X);
				Position.Y = Chunk.Min.Y + FVoxelUtilities::Halton<3>(SeedValue) * (Chunk.Max.Y - Chunk.Min.Y);
				SeedValue++;

				if (!Bounds2D.Contains(Position))
				{
					continue;
				}

				const int64 WriteIndex = OutNumPosition++;
				OutPositions.Set(WriteIndex, FVector2D(Position));
			}
		});
}

bool FVoxelScatterUtilities::GenerateGridPositions(
	const FVoxelBox& Bounds,
	const double DistanceBetweenPositions,
	const double JitterDistance,
	const uint32 Seed,
	FVoxelDoubleVector2DBuffer& OutPositions)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelBox2D Bounds2D(Bounds);
	return PrepareChunksForPositionsGenerator(
		Bounds2D,
		DistanceBetweenPositions,
		OutPositions,
		[&](const FVoxelIntBox2D& Chunk, const int64 NumMaxPositions, int64& OutNumPosition)
		{
			uint32 SeedValue = FVoxelUtilities::MurmurHash(Seed);
			SeedValue ^= FVoxelUtilities::MurmurHash(Chunk);

			const FRandomStream Stream(SeedValue);

			const int64 NumGridPositions = (Chunk.Max.X - Chunk.Min.X) / DistanceBetweenPositions;
			const int64 NumIndices = FMath::Min(NumGridPositions * NumGridPositions, NumMaxPositions);
			for (int64 Index = 0; Index < NumIndices; Index++)
			{
				FVector2D Position = FVector2D::ZeroVector;
				Position.X = Chunk.Min.X + DistanceBetweenPositions * (Index % NumGridPositions) + Stream.FRand() * JitterDistance * DistanceBetweenPositions;
				Position.Y = Chunk.Min.Y + DistanceBetweenPositions * (Index / NumGridPositions) + Stream.FRand() * JitterDistance * DistanceBetweenPositions;

				if (!Bounds2D.Contains(Position))
				{
					continue;
				}

				const int64 WriteIndex = OutNumPosition++;
				OutPositions.Set(WriteIndex, Position);
			}
		});
}

bool FVoxelScatterUtilities::GenerateSobolPositions(
	const FVoxelBox& Bounds,
	const double DistanceBetweenPositions,
	const uint32 Seed,
	FVoxelDoubleVector2DBuffer& OutPositions)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelBox2D Bounds2D(Bounds);
	return PrepareChunksForPositionsGenerator(
		Bounds2D,
		DistanceBetweenPositions,
		OutPositions,
		[&](const FVoxelIntBox2D& Chunk, const int64 NumMaxPositions, int64& OutNumPosition)
		{
			uint32 SeedValue = FVoxelUtilities::MurmurHash(Seed);
			SeedValue ^= FVoxelUtilities::MurmurHash(Chunk);

			FVector2D SobolPosition = FSobol::Evaluate(0, 1, FIntPoint::ZeroValue, FIntPoint(SeedValue, FVoxelUtilities::MurmurHash32(SeedValue)));
			for (int32 Index = 0; Index < NumMaxPositions; Index++)
			{
				FVector2D Position = FVector2D::ZeroVector;
				Position.X = Chunk.Min.X + SobolPosition.X * (Chunk.Max.X - Chunk.Min.X);
				Position.Y = Chunk.Min.Y + SobolPosition.Y * (Chunk.Max.Y - Chunk.Min.Y);
				SeedValue++;

				SobolPosition = FSobol::Next(Index + 1, 1, SobolPosition);

				if (!Bounds2D.Contains(Position))
				{
					continue;
				}

				const int64 WriteIndex = OutNumPosition++;
				OutPositions.Set(WriteIndex, FVector2D(Position));
			}
		});
}