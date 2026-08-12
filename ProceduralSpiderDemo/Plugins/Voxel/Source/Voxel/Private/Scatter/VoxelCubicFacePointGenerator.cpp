// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelCubicFacePointGenerator.h"
#include "VoxelLayers.h"
#include "VoxelQuery.h"
#include "VoxelPointId.h"
#include "VoxelCellGenerator.h"
#include "VoxelSparseSampler.h"
#include "VoxelVolumeLayer.h"
#include "VoxelHeightLayer.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "Utilities/VoxelBufferMathUtilities.h"

FVoxelCubicFacePointGenerator::FVoxelCubicFacePointGenerator(
	const FVoxelLayers& Layers,
	const FVoxelSurfaceTypeTable& SurfaceTypeTable,
	FVoxelDependencyCollector& DependencyCollector,
	const FVoxelWeakStackLayer& WeakLayer,
	const FInt64Vector& ChunkOffset,
	const int32 VoxelSize,
	const int32 ChunkSize,
	const uint64 Seed,
	const bool bQuerySurfaceTypes,
	const bool bResolveSurfaceTypes,
	const bool bSkipNaNFaces,
	const TConstVoxelArrayView<FVoxelMetadataRef> MetadatasToQuery)
	: Layers(Layers)
	, SurfaceTypeTable(SurfaceTypeTable)
	, DependencyCollector(DependencyCollector)
	, WeakLayer(WeakLayer)
	, ChunkOffset(ChunkOffset)
	, VoxelSize(VoxelSize)
	, ChunkSize(ChunkSize)
	, Seed(Seed)
	, bQuerySurfaceTypes(bQuerySurfaceTypes)
	, bResolveSurfaceTypes(bResolveSurfaceTypes)
	, bSkipNaNFaces(bSkipNaNFaces)
	, MetadatasToQuery(MetadatasToQuery)
	, DataSize(ChunkSize + 2)
{
}

FVoxelPointSet FVoxelCubicFacePointGenerator::GeneratePoints()
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicFacePointGenerator::GeneratePoints");

	const FVoxelQuery Query(
		0, // LOD 0 always for cubic
		Layers,
		SurfaceTypeTable,
		DependencyCollector);

	// Check if there are any stamps affecting this chunk
	const FVector Start = FVector(ChunkOffset - 1) * VoxelSize;
	const FVoxelBox Bounds = FVoxelBox(Start, Start + FVector(DataSize) * VoxelSize);

	if (!Query.HasStamps(
		WeakLayer,
		Bounds,
		EVoxelStampBehavior::AffectShape))
	{
		return {};
	}

	// Reserve estimated memory (surface cells estimate)
	const int32 EstimatedFaces = 6 * ChunkSize * ChunkSize;
	PointIds.Reserve(EstimatedFaces);
	PositionsX.Reserve(EstimatedFaces);
	PositionsY.Reserve(EstimatedFaces);
	PositionsZ.Reserve(EstimatedFaces);
	NormalsX.Reserve(EstimatedFaces);
	NormalsY.Reserve(EstimatedFaces);
	NormalsZ.Reserve(EstimatedFaces);
	SurfaceTypes.Reserve(EstimatedFaces);

	QueryDistancesAndSurfaceTypes();
	ResolveSmartSurfaceTypes();
	GenerateFacePoints();

	const int32 Num = PointIds.Num();
	if (Num == 0)
	{
		return {};
	}

	// Build output buffers
	FVoxelPointIdBuffer PointIdBuffer;
	PointIdBuffer.Allocate(Num);
	FMemory::Memcpy(PointIdBuffer.View().GetData(), PointIds.GetData(), Num * sizeof(FVoxelPointId));

	FVoxelDoubleVectorBuffer Positions;
	Positions.Allocate(Num);
	FMemory::Memcpy(Positions.X.View().GetData(), PositionsX.GetData(), Num * sizeof(double));
	FMemory::Memcpy(Positions.Y.View().GetData(), PositionsY.GetData(), Num * sizeof(double));
	FMemory::Memcpy(Positions.Z.View().GetData(), PositionsZ.GetData(), Num * sizeof(double));

	FVoxelVectorBuffer Normals;
	Normals.Allocate(Num);
	FMemory::Memcpy(Normals.X.View().GetData(), NormalsX.GetData(), Num * sizeof(float));
	FMemory::Memcpy(Normals.Y.View().GetData(), NormalsY.GetData(), Num * sizeof(float));
	FMemory::Memcpy(Normals.Z.View().GetData(), NormalsZ.GetData(), Num * sizeof(float));

	FVoxelSurfaceTypeBlendBuffer SurfaceTypeBuffer;
	if (bQuerySurfaceTypes)
	{
		SurfaceTypeBuffer.Allocate(Num);
		FMemory::Memcpy(SurfaceTypeBuffer.View().GetData(), SurfaceTypes.GetData(), Num * sizeof(FVoxelSurfaceTypeBlend));

		if (bResolveSurfaceTypes)
		{
			VOXEL_SCOPE_COUNTER("Smart surface types");

			FVoxelSmartSurfaceTypeResolver Resolver(
				0,
				WeakLayer,
				Layers,
				SurfaceTypeTable,
				DependencyCollector,
				Positions,
				Normals,
				SurfaceTypeBuffer.View());

			Resolver.Resolve();
		}
	}

	// Build point set
	FVoxelPointSet Points;
	Points.SetNum(Num);
	Points.Add(FVoxelPointAttributes::Id, MakeSharedCopy(MoveTemp(PointIdBuffer)));
	Points.Add(FVoxelPointAttributes::Position, MakeSharedCopy(MoveTemp(Positions)));
	Points.Add(FVoxelPointAttributes::Rotation, MakeSharedCopy(FVoxelBufferMathUtilities::MakeQuaternionFromZ(Normals)));

	if (bQuerySurfaceTypes)
	{
		Points.Add(FVoxelPointAttributes::SurfaceTypes, MakeSharedCopy(MoveTemp(SurfaceTypeBuffer)));
	}

	return Points;
}

void FVoxelCubicFacePointGenerator::QueryDistancesAndSurfaceTypes()
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicFacePointGenerator::QueryDistancesAndSurfaceTypes");

	const int32 TotalCells = DataSize * DataSize * DataSize;
	const float CellSize = float(VoxelSize);
	const FVector Start = FVector(ChunkOffset - 1) * VoxelSize;
	const FVoxelBox Bounds = FVoxelBox(Start, Start + FVector(DataSize) * CellSize);

	const FVoxelQuery Query(
		0, // LOD 0 always for cubic
		Layers,
		SurfaceTypeTable,
		DependencyCollector);

	// Get volume layer for sparse sampling
	const TSharedPtr<const FVoxelVolumeLayer> VolumeLayer = Layers.FindVolumeLayer(WeakLayer, DependencyCollector);

	// Initialize grids: all distances NaN until sampled
	DistanceGrid.SetNum(TotalCells);
	BlockTypeGrid.SetNum(TotalCells);
	FVoxelUtilities::SetAll(DistanceGrid, FVoxelUtilities::NaNf());

	// Track volume bounds for height layer processing
	FVoxelIntBox VolumeBounds;
	bool bHasVolumeBounds = false;

	if (VolumeLayer)
	{
		// Get bounds of stamps that affect shape
		const FVoxelOptionalBox OptionalVolumeBounds = VolumeLayer->GetVolumeStampBounds(
			Query,
			Bounds,
			EVoxelStampBehavior::AffectShape);

		if (OptionalVolumeBounds.IsValid())
		{
			// Convert to grid-relative integer bounds
			VolumeBounds = FVoxelIntBox::FromFloatBox_WithPadding(
				OptionalVolumeBounds.GetBox().ShiftBy(-Start) / CellSize)
				.IntersectWith(FVoxelIntBox(0, DataSize));

			bHasVolumeBounds = VolumeBounds.IsValid();
		}

		if (bHasVolumeBounds)
		{
			// Expand bounds by 1 cell for sparse sampling to include adjacent air cells
			// This ensures cells just outside the stamp get queried (important for -Z faces)
			const FVoxelIntBox ExpandedBounds = VolumeBounds.Extend(1).IntersectWith(FVoxelIntBox(0, DataSize));

			// Use sparse sampling to determine which regions need full-resolution querying
			const TVoxelArray<FVoxelIntBox> BoundsToQuery = FVoxelSparseSampler::ComputeBoundsToQuery(
				Query,
				*VolumeLayer,
				ExpandedBounds,
				Start,
				CellSize,
				&Layers,
				0); // LOD 0 for cubic

			// Query only the identified regions
			for (const FVoxelIntBox& RegionBounds : BoundsToQuery)
			{
				VOXEL_SCOPE_COUNTER("Query region");

				const int32 RegionCount = RegionBounds.Count_int32();

				// Build query positions for this region (at cell centers)
				FVoxelDoubleVectorBuffer QueryPositions;
				QueryPositions.Allocate(RegionCount);

				int32 WriteIndex = 0;
				for (int32 Z = RegionBounds.Min.Z; Z < RegionBounds.Max.Z; Z++)
				{
					for (int32 Y = RegionBounds.Min.Y; Y < RegionBounds.Max.Y; Y++)
					{
						for (int32 X = RegionBounds.Min.X; X < RegionBounds.Max.X; X++)
						{
							// Sample at cell CENTER for blocky discretization
							const FVector Position = Start + FVector(X + 0.5, Y + 0.5, Z + 0.5) * CellSize;
							QueryPositions.Set(WriteIndex++, Position);
						}
					}
				}

				// Query surface types for this region
				FVoxelSurfaceTypeBlendBuffer SurfaceTypeBuffer;
				SurfaceTypeBuffer.AllocateZeroed(RegionCount);

				const FVoxelFloatBuffer Distances = Query.SampleVolumeLayer(
					WeakLayer,
					QueryPositions,
					SurfaceTypeBuffer.View(),
					{});

				// Update solidity and block type grids for this region
				int32 ReadIndex = 0;
				for (int32 Z = RegionBounds.Min.Z; Z < RegionBounds.Max.Z; Z++)
				{
					for (int32 Y = RegionBounds.Min.Y; Y < RegionBounds.Max.Y; Y++)
					{
						for (int32 X = RegionBounds.Min.X; X < RegionBounds.Max.X; X++)
						{
							const float Distance = Distances[ReadIndex];
							const int32 GridIndex = GetGridIndex(X, Y, Z);

							// Store the distance (NaN stays NaN, negative = solid, positive = air)
							DistanceGrid[GridIndex] = Distance;

							// Store block type for solid cells
							if (!FVoxelUtilities::IsNaN(Distance) &&
								Distance < 0)
							{
								BlockTypeGrid[GridIndex] = SurfaceTypeBuffer[ReadIndex].GetTopLayer().Type;
							}

							ReadIndex++;
						}
					}
				}
			}
		}
	}

	// Process height layer for cells not covered by volume
	QueryHeightLayerCells(Query, bHasVolumeBounds ? VolumeBounds : FVoxelIntBox());
}

void FVoxelCubicFacePointGenerator::QueryHeightLayerCells(const FVoxelQuery& Query, const FVoxelIntBox& VolumeBounds)
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicFacePointGenerator::QueryHeightLayerCells");

	const float CellSize = float(VoxelSize);
	const FVector Start = FVector(ChunkOffset - 1) * VoxelSize;
	const FVoxelBox Bounds = FVoxelBox(Start, Start + FVector(DataSize) * CellSize);

	// Check if volume layer has intersect stamps - if so, can't generate height cells
	const TSharedPtr<const FVoxelVolumeLayer> VolumeLayer = Layers.FindVolumeLayer(WeakLayer, DependencyCollector);
	if (VolumeLayer &&
		VolumeLayer->HasIntersectStamps())
	{
		return;
	}

	// Get height layer
	const TVoxelOptional<FVoxelWeakStackLayer> WeakHeightLayer = Query.GetFirstHeightLayer(WeakLayer);
	if (!WeakHeightLayer)
	{
		return;
	}

	const TSharedPtr<const FVoxelHeightLayer> HeightLayer = Layers.FindHeightLayer(WeakHeightLayer.GetValue(), DependencyCollector);
	if (!HeightLayer ||
		!HeightLayer->HasStamps(
			Query,
			Bounds,
			EVoxelStampBehavior::AffectShape,
			true))
	{
		return;
	}

	VOXEL_SCOPE_COUNTER("Compute heights");

	const FVoxelOptionalBox2D HeightBounds = HeightLayer->GetStampBounds(
		Query,
		FVoxelBox2D(Bounds),
		EVoxelStampBehavior::AffectShape);

	if (!HeightBounds.IsValid())
	{
		return;
	}

	const FVoxelIntBox2D HeightQueryIndices =
		FVoxelIntBox2D::FromFloatBox_WithPadding(HeightBounds.GetBox().ShiftBy(-FVector2D(Start)) / CellSize)
		.IntersectWith(FVoxelIntBox2D(0, FIntPoint(DataSize, DataSize)));

	const FIntPoint Size2D = HeightQueryIndices.Size();
	const int32 NumHeightQueries = HeightQueryIndices.Count_int32();

	// Use FVoxelCellGeneratorHeights to manage heights and dependency collection
	const TSharedRef<FVoxelCellGeneratorHeights> HeightData = MakeShared<FVoxelCellGeneratorHeights>();
	HeightData->Indices = HeightQueryIndices;
	FVoxelUtilities::SetNumFast(HeightData->Heights, NumHeightQueries);
	FVoxelUtilities::SetAll(HeightData->Heights, FVoxelUtilities::NaNf());

	const FVoxelQuery HeightQuery(
		0, // LOD 0 for cubic
		Layers,
		SurfaceTypeTable,
		HeightData->DependencyCollector,
		nullptr);

	VOXEL_SCOPE_COUNTER_NUM("Query heights", HeightData->Heights.Num());

	HeightLayer->Sample(FVoxelHeightBulkQuery::Create(
		HeightQuery,
		HeightData->Heights,
		FVector2D(Start) + (FVector2D(HeightQueryIndices.Min) + 0.5) * CellSize,
		HeightQueryIndices.Size(),
		CellSize));

	DependencyCollector.AddDependencies(HeightData->DependencyCollector);

	// If volume covers full size, heights are only cached for reuse
	if (VolumeBounds.IsValid() &&
		VolumeBounds == FVoxelIntBox(0, DataSize))
	{
		return;
	}

	// Query surface types at 2D positions
	FVoxelDoubleVector2DBuffer QueryPositions;
	QueryPositions.Allocate(NumHeightQueries);

	for (int32 LocalY = 0; LocalY < Size2D.Y; LocalY++)
	{
		for (int32 LocalX = 0; LocalX < Size2D.X; LocalX++)
		{
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(Size2D, LocalX, LocalY);
			const int32 GridX = HeightQueryIndices.Min.X + LocalX;
			const int32 GridY = HeightQueryIndices.Min.Y + LocalY;

			// Query at cell center (2D)
			const FVector2D Position = FVector2D(Start) + FVector2D(GridX + 0.5, GridY + 0.5) * CellSize;
			QueryPositions.Set(Index, Position);
		}
	}

	FVoxelSurfaceTypeBlendBuffer SurfaceTypeBuffer;
	SurfaceTypeBuffer.AllocateZeroed(NumHeightQueries);

	VOXEL_SCOPE_COUNTER_NUM("Query height surface types", NumHeightQueries);

	(void)Query.SampleHeightLayer(
		WeakHeightLayer.GetValue(),
		QueryPositions,
		SurfaceTypeBuffer.View(),
		{});

	// For cubic, we sample at cell centers
	// A cell at Z is solid if its center (Z + 0.5) is below the height
	for (int32 LocalY = 0; LocalY < Size2D.Y; LocalY++)
	{
		for (int32 LocalX = 0; LocalX < Size2D.X; LocalX++)
		{
			const int32 Index2D = FVoxelUtilities::Get2DIndex<int32>(Size2D, LocalX, LocalY);
			const float Height = HeightData->Heights[Index2D];

			if (FVoxelUtilities::IsNaN(Height))
			{
				continue;
			}

			const FVoxelSurfaceType SurfaceType = SurfaceTypeBuffer[Index2D].GetTopLayer().Type;

			const int32 GridX = HeightQueryIndices.Min.X + LocalX;
			const int32 GridY = HeightQueryIndices.Min.Y + LocalY;

			for (int32 GridZ = 0; GridZ < DataSize; GridZ++)
			{
				// Skip cells covered by volume bounds (already processed)
				if (VolumeBounds.IsValid() &&
					VolumeBounds.Contains(FIntVector(GridX, GridY, GridZ)))
				{
					continue;
				}

				const int32 GridIndex = GetGridIndex(GridX, GridY, GridZ);

				// Compute synthetic distance: cell center Z - height
				// Negative = below surface = solid
				const float CellCenterZ = Start.Z + (GridZ + 0.5f) * CellSize;
				const float Distance = CellCenterZ - Height;

				// Only update if not already set by volume layer (NaN check)
				if (FVoxelUtilities::IsNaN(DistanceGrid[GridIndex]))
				{
					DistanceGrid[GridIndex] = Distance;

					if (Distance < 0)
					{
						BlockTypeGrid[GridIndex] = SurfaceType;
					}
				}
			}
		}
	}
}

void FVoxelCubicFacePointGenerator::ResolveSmartSurfaceTypes()
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicFacePointGenerator::ResolveSmartSurfaceTypes");

	// Early out if no smart surface types are registered
	if (SurfaceTypeTable.SurfaceTypeToSmartSurfaceProxy.Num() == 0)
	{
		return;
	}

	// Collect solid cells that have smart surface types
	struct FCellToResolve
	{
		int32 GridIndex;
		FIntVector GridPosition;
	};
	TVoxelArray<FCellToResolve> CellsToResolve;

	const float CellSize = float(VoxelSize);
	const FVector Start = FVector(ChunkOffset - 1) * VoxelSize;

	// Iterate all cells in the chunk (excluding padding)
	for (int32 Z = 1; Z < DataSize - 1; Z++)
	{
		for (int32 Y = 1; Y < DataSize - 1; Y++)
		{
			for (int32 X = 1; X < DataSize - 1; X++)
			{
				const int32 GridIndex = GetGridIndex(X, Y, Z);
				const float Distance = DistanceGrid[GridIndex];

				// Skip non-solid cells
				if (FVoxelUtilities::IsNaN(Distance) ||
					Distance >= 0)
				{
					continue;
				}

				const FVoxelSurfaceType BlockType = BlockTypeGrid[GridIndex];

				// Check if this is a smart surface type
				if (BlockType.GetClass() != FVoxelSurfaceType::EClass::SmartSurfaceType)
				{
					continue;
				}

				// Skip interior cells (no visible faces)
				const bool bHasSurfaceFace =
					!IsSolid(X - 1, Y, Z) ||
					!IsSolid(X + 1, Y, Z) ||
					!IsSolid(X, Y - 1, Z) ||
					!IsSolid(X, Y + 1, Z) ||
					!IsSolid(X, Y, Z - 1) ||
					!IsSolid(X, Y, Z + 1);

				if (!bHasSurfaceFace)
				{
					continue;
				}

				CellsToResolve.Add({ GridIndex, FIntVector(X, Y, Z) });
			}
		}
	}

	if (CellsToResolve.Num() == 0)
	{
		return;
	}

	// Build position and normal buffers for the resolver
	FVoxelDoubleVectorBuffer PositionsBuffer;
	PositionsBuffer.Allocate(CellsToResolve.Num());

	FVoxelVectorBuffer NormalsBuffer;
	NormalsBuffer.Allocate(CellsToResolve.Num());

	FVoxelSurfaceTypeBlendBuffer SurfaceTypeBlends;
	SurfaceTypeBlends.Allocate(CellsToResolve.Num());

	for (int32 Index = 0; Index < CellsToResolve.Num(); Index++)
	{
		const FCellToResolve& Cell = CellsToResolve[Index];
		const int32 X = Cell.GridPosition.X;
		const int32 Y = Cell.GridPosition.Y;
		const int32 Z = Cell.GridPosition.Z;

		// Cell center in world space
		const FVector Position = Start + FVector(X + 0.5, Y + 0.5, Z + 0.5) * CellSize;
		PositionsBuffer.Set(Index, Position);

		// Compute gradient normal from neighbor distances
		FVector3f Gradient(ForceInit);

		const float Xm = GetDistance(X - 1, Y, Z);
		const float Xp = GetDistance(X + 1, Y, Z);
		if (!FVoxelUtilities::IsNaN(Xm) &&
			!FVoxelUtilities::IsNaN(Xp))
		{
			Gradient.X = Xp - Xm;
		}

		const float Ym = GetDistance(X, Y - 1, Z);
		const float Yp = GetDistance(X, Y + 1, Z);
		if (!FVoxelUtilities::IsNaN(Ym) &&
			!FVoxelUtilities::IsNaN(Yp))
		{
			Gradient.Y = Yp - Ym;
		}

		const float Zm = GetDistance(X, Y, Z - 1);
		const float Zp = GetDistance(X, Y, Z + 1);
		if (!FVoxelUtilities::IsNaN(Zm) &&
			!FVoxelUtilities::IsNaN(Zp))
		{
			Gradient.Z = Zp - Zm;
		}

		// Normal points inward (opposite of gradient direction)
		FVector3f Normal = -Gradient.GetSafeNormal();
		if (Normal.IsNearlyZero())
		{
			// Fallback to up vector if gradient is zero
			Normal = FVector3f(0, 0, 1);
		}
		NormalsBuffer.Set(Index, Normal);

		// Initialize surface type blend with the smart surface type
		FVoxelSurfaceTypeBlend Blend;
		Blend.InitializeFromType(BlockTypeGrid[Cell.GridIndex]);
		SurfaceTypeBlends.Set(Index, Blend);
	}

	// Resolve smart surface types
	FVoxelSmartSurfaceTypeResolver Resolver(
		0, // LOD 0 for cubic
		WeakLayer,
		Layers,
		SurfaceTypeTable,
		DependencyCollector,
		PositionsBuffer,
		NormalsBuffer,
		SurfaceTypeBlends.View());

	Resolver.Resolve();

	// Write resolved types back to BlockTypeGrid
	for (int32 Index = 0; Index < CellsToResolve.Num(); Index++)
	{
		const FCellToResolve& Cell = CellsToResolve[Index];
		const FVoxelSurfaceTypeBlend& ResolvedBlend = SurfaceTypeBlends[Index];

		if (!ResolvedBlend.IsNull())
		{
			BlockTypeGrid[Cell.GridIndex] = ResolvedBlend.GetTopLayer().Type;
		}
	}
}

void FVoxelCubicFacePointGenerator::GenerateFacePoints()
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicFacePointGenerator::GenerateFacePoints");

	// Face directions: 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z
	static const FIntVector FaceNormals[6] = {
		{ 1,  0,  0},  // +X
		{-1,  0,  0},  // -X
		{ 0,  1,  0},  // +Y
		{ 0, -1,  0},  // -Y
		{ 0,  0,  1},  // +Z
		{ 0,  0, -1}   // -Z
	};

	// Only iterate over the actual chunk cells (excluding padding)
	// Grid [1, DataSize-1) corresponds to chunk [0, ChunkSize)
	for (int32 Z = 1; Z < DataSize - 1; Z++)
	{
		for (int32 Y = 1; Y < DataSize - 1; Y++)
		{
			for (int32 X = 1; X < DataSize - 1; X++)
			{
				if (!IsSolid(X, Y, Z))
				{
					continue; // Air cell, no faces
				}

				const FVoxelSurfaceType CellBlockType = BlockTypeGrid[GetGridIndex(X, Y, Z)];

				// Check each face direction
				for (int32 Face = 0; Face < 6; Face++)
				{
					const FIntVector& Normal = FaceNormals[Face];
					const int32 NeighborX = X + Normal.X;
					const int32 NeighborY = Y + Normal.Y;
					const int32 NeighborZ = Z + Normal.Z;

					// Only generate points for faces adjacent to non-solid neighbors
					// NaN neighbors are treated as air unless bSkipNaNFaces is true
					if (!IsSolid(NeighborX, NeighborY, NeighborZ) &&
						(!bSkipNaNFaces || !IsNaN(NeighborX, NeighborY, NeighborZ)))
					{
						// Convert from grid coords to chunk-local coords
						const FIntVector LocalCell(X - 1, Y - 1, Z - 1);
						AddFacePoint(LocalCell, Face, CellBlockType);
					}
				}
			}
		}
	}
}

void FVoxelCubicFacePointGenerator::AddFacePoint(
	const FIntVector& CellPosition,
	const int32 FaceDirection,
	const FVoxelSurfaceType BlockType)
{
	static const FVector3f FaceNormalVectors[6] = {
		{ 1,  0,  0},  // +X
		{-1,  0,  0},  // -X
		{ 0,  1,  0},  // +Y
		{ 0, -1,  0},  // -Y
		{ 0,  0,  1},  // +Z
		{ 0,  0, -1}   // -Z
	};

	const FVector3f Normal = FaceNormalVectors[FaceDirection];

	// Face center position:
	// CellPosition is in chunk-local coords [0, ChunkSize)
	// Position = ChunkOffset * VoxelSize + (CellPos + 0.5 + Normal * 0.5) * VoxelSize
	const FVector Position = FVector(ChunkOffset) * VoxelSize +
		FVector(CellPosition.X + 0.5 + Normal.X * 0.5,
		        CellPosition.Y + 0.5 + Normal.Y * 0.5,
		        CellPosition.Z + 0.5 + Normal.Z * 0.5) * VoxelSize;

	// Deterministic point ID based on position and face direction
	const uint64 PointId = FVoxelUtilities::MurmurHashMulti(
		Seed,
		ChunkOffset.X + CellPosition.X,
		ChunkOffset.Y + CellPosition.Y,
		ChunkOffset.Z + CellPosition.Z,
		FaceDirection);

	PointIds.Add(PointId);
	PositionsX.Add(Position.X);
	PositionsY.Add(Position.Y);
	PositionsZ.Add(Position.Z);
	NormalsX.Add(Normal.X);
	NormalsY.Add(Normal.Y);
	NormalsZ.Add(Normal.Z);

	FVoxelSurfaceTypeBlend Blend;
	Blend.InitializeFromType(BlockType);
	SurfaceTypes.Add(Blend);
}
