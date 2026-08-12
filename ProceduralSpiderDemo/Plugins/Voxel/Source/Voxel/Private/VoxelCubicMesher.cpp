// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelCubicMesher.h"
#include "VoxelLayers.h"
#include "VoxelQuery.h"
#include "VoxelSparseSampler.h"
#include "VoxelVolumeLayer.h"
#include "VoxelHeightLayer.h"
#include "Buffer/VoxelBaseBuffers.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "MegaMaterial/VoxelMegaMaterialProxy.h"

FVoxelCubicMesher::FVoxelCubicMesher(
	FVoxelLayers& Layers,
	FVoxelSurfaceTypeTable& SurfaceTypeTable,
	FVoxelDependencyCollector& DependencyCollector,
	const FVoxelWeakStackLayer& WeakLayer,
	const FInt64Vector& ChunkOffset,
	const int32 VoxelSize,
	const int32 ChunkSize,
	const FTransform& LocalToWorld,
	const FVoxelMegaMaterialProxy& MegaMaterialProxy,
	const TSharedPtr<const FVoxelCellGeneratorHeights>& CachedHeights)
	: Layers(Layers)
	, SurfaceTypeTable(SurfaceTypeTable)
	, DependencyCollector(DependencyCollector)
	, WeakLayer(WeakLayer)
	, ChunkOffset(ChunkOffset)
	, VoxelSize(VoxelSize)
	, ChunkSize(ChunkSize)
	, LocalToWorld(LocalToWorld)
	, MegaMaterialProxy(MegaMaterialProxy)
	, DataSize(ChunkSize + 2)
	, CachedHeights(CachedHeights)
{
}

TSharedPtr<FVoxelMesh> FVoxelCubicMesher::CreateMesh()
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicMesher::CreateMesh");

	// Add dependency on cubic occlusion settings
	DependencyCollector.AddDependency(*SurfaceTypeTable.CubicOcclusionDependency);

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
		return nullptr;
	}

	// Reserve estimated memory (surface cells estimate)
	const int32 EstimatedFaces = 6 * ChunkSize * ChunkSize;
	Vertices.Reserve(4 * EstimatedFaces);
	Normals.Reserve(4 * EstimatedFaces);
	Indices.Reserve(6 * EstimatedFaces);
	SurfaceTypes.Reserve(4 * EstimatedFaces);
	Cells.Reserve(4 * EstimatedFaces);

	QueryDistancesAndSurfaceTypes();
	ResolveSmartSurfaceTypes();
	GenerateFaces();

	if (Indices.Num() == 0)
	{
		return nullptr;
	}

	// Filter out vertices with invisible surface types
	FilterInvisibleSurfaceTypes();

	// Check again after filtering
	if (Vertices.Num() == 0)
	{
		return nullptr;
	}

	if (Indices.Num() == 0)
	{
		return nullptr;
	}

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
	INLINE_LAMBDA
	{
		// Get metadata refs from the cubic template material
		const TConstVoxelArrayView<FVoxelMetadataRef> MetadataRefs = MegaMaterialProxy.GetMetadataIndexToMetadata();
		if (MetadataRefs.Num() == 0)
		{
			return;
		}

		VOXEL_SCOPE_COUNTER("Metadata");

		// Helper to compute flat index from cell coordinates
		// Cells are in range [0, ChunkSize), so use ChunkSize as stride
		const auto GetCellFlatIndex = [this](const FVoxelMesh::FCell& Cell) -> int32
		{
			return Cell.X + Cell.Y * ChunkSize + Cell.Z * ChunkSize * ChunkSize;
		};

		// 1. Collect unique cell positions and build cell-to-index map
		TVoxelMap<int32, int32> CellFlatIndexToArrayIndex;
		TVoxelArray<FVoxelMesh::FCell> UniqueCells;
		UniqueCells.Reserve(Cells.Num() / 4); // Estimate: ~4 vertices per cell

		for (const FVoxelMesh::FCell& Cell : Cells)
		{
			const int32 FlatIndex = GetCellFlatIndex(Cell);
			if (!CellFlatIndexToArrayIndex.Contains(FlatIndex))
			{
				CellFlatIndexToArrayIndex.Add_CheckNew(FlatIndex, UniqueCells.Num());
				UniqueCells.Add(Cell);
			}
		}

		const int32 NumCells = UniqueCells.Num();

		// 2. Build query positions at cell centers
		FVoxelDoubleVectorBuffer QueryPositions;
		QueryPositions.Allocate(NumCells);

		const FVector ChunkStart = FVector(ChunkOffset) * VoxelSize;
		for (int32 CellIndex = 0; CellIndex < NumCells; CellIndex++)
		{
			const FVoxelMesh::FCell& Cell = UniqueCells[CellIndex];
			const FVector WorldPosition = ChunkStart + FVector(Cell.X + 0.5, Cell.Y + 0.5, Cell.Z + 0.5) * VoxelSize;
			QueryPositions.Set(CellIndex, WorldPosition);
		}

		// 3. Create per-cell metadata buffers and query
		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> CellMetadataToBuffer;
		for (const FVoxelMetadataRef& MetadataRef : MetadataRefs)
		{
			CellMetadataToBuffer.Add_EnsureNew(
				MetadataRef,
				MetadataRef.MakeDefaultBuffer(NumCells));
		}

		const FVoxelQuery MetadataQuery(
			0,
			Layers,
			SurfaceTypeTable,
			DependencyCollector);

		(void)MetadataQuery.SampleVolumeLayer(
			WeakLayer,
			QueryPositions,
			{},
			CellMetadataToBuffer);

		// 4. Build vertex-to-cell index mapping
		TVoxelArray<int32> VertexToCellIndex;
		FVoxelUtilities::SetNumFast(VertexToCellIndex, Vertices.Num());

		for (int32 VertexIndex = 0; VertexIndex < Vertices.Num(); VertexIndex++)
		{
			const int32 FlatIndex = GetCellFlatIndex(Cells[VertexIndex]);
			VertexToCellIndex[VertexIndex] = CellFlatIndexToArrayIndex.FindChecked(FlatIndex);
		}

		// 5. Expand cell metadata to per-vertex buffers using Gather
		for (const FVoxelMetadataRef& MetadataRef : MetadataRefs)
		{
			const TSharedRef<FVoxelBuffer>& CellBuffer = CellMetadataToBuffer.FindChecked(MetadataRef);
			MetadataToBuffer.Add_EnsureNew(MetadataRef, CellBuffer->Gather(VertexToCellIndex));
		}
	};

	// Collect used surface types
	TVoxelSet<FVoxelSurfaceType> UsedSurfaceTypesSet;
	for (const FVoxelSurfaceTypeBlend& Blend : SurfaceTypes)
	{
		if (!Blend.IsNull())
		{
			UsedSurfaceTypesSet.Add(Blend.GetTopLayer().Type);
		}
	}
	TVoxelArray<FVoxelSurfaceType> UsedSurfaceTypes = UsedSurfaceTypesSet.Array();
	UsedSurfaceTypes.Sort();

	// Convert normals to octahedron format
	TVoxelArray<FVoxelOctahedron> OctahedronNormals;
	FVoxelUtilities::SetNumFast(OctahedronNormals, Normals.Num());
	for (int32 Index = 0; Index < Normals.Num(); Index++)
	{
		OctahedronNormals[Index] = FVoxelOctahedron(Normals[Index]);
	}

	// No LODs for cubic mesher
	TVoxelArray<FVoxelMesh::FLOD> EmptyLODs;

	return MakeShared<FVoxelMesh>(
		0, // ChunkLOD = 0
		ChunkOffset,
		ChunkSize,
		MoveTemp(UsedSurfaceTypes),
		MoveTemp(MetadataToBuffer),
		MoveTemp(Indices),
		MoveTemp(Vertices),
		MoveTemp(OctahedronNormals),
		MoveTemp(SurfaceTypes),
		MoveTemp(Cells),
		MoveTemp(EmptyLODs));
}

TVoxelArray<TSharedPtr<FVoxelMesh>> FVoxelCubicMesher::CreateMeshes()
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicMesher::CreateMeshes");

	// Add dependency on cubic occlusion settings
	DependencyCollector.AddDependency(*SurfaceTypeTable.CubicOcclusionDependency);

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
	Vertices.Reserve(4 * EstimatedFaces);
	Normals.Reserve(4 * EstimatedFaces);
	Indices.Reserve(6 * EstimatedFaces);
	SurfaceTypes.Reserve(4 * EstimatedFaces);
	Cells.Reserve(4 * EstimatedFaces);

	QueryDistancesAndSurfaceTypes();
	ResolveSmartSurfaceTypes();
	GenerateFaces();

	if (Indices.Num() == 0)
	{
		return {};
	}

	// Filter out vertices with invisible surface types
	FilterInvisibleSurfaceTypes();

	// Check again after filtering
	if (Vertices.Num() == 0 ||
		Indices.Num() == 0)
	{
		return {};
	}

	// Split quads by blend mode: masked (opaque+masked) vs translucent
	// Each quad is 4 vertices and 6 indices
	checkVoxelSlow(Indices.Num() % 6 == 0);
	const int32 NumQuads = Indices.Num() / 6;

	struct FMeshData
	{
		TVoxelArray<int32> Indices;
		TVoxelArray<FVector3f> Vertices;
		TVoxelArray<FVector3f> Normals;
		TVoxelArray<FVoxelMesh::FCell> Cells;
		TVoxelArray<FVoxelSurfaceTypeBlend> SurfaceTypes;
		TVoxelSet<FVoxelSurfaceType> UsedSurfaceTypesSet;
	};

	FMeshData MaskedData;
	FMeshData TranslucentData;

	// Reserve space based on common case (most faces are masked/opaque)
	MaskedData.Vertices.Reserve(Vertices.Num());
	MaskedData.Normals.Reserve(Normals.Num());
	MaskedData.Indices.Reserve(Indices.Num());
	MaskedData.SurfaceTypes.Reserve(SurfaceTypes.Num());
	MaskedData.Cells.Reserve(Cells.Num());

	for (int32 QuadIndex = 0; QuadIndex < NumQuads; QuadIndex++)
	{
		const int32 BaseIndex = QuadIndex * 6;
		const int32 FirstVertexIndex = Indices[BaseIndex];

		// Get surface type from first vertex of quad
		const FVoxelSurfaceTypeBlend& Blend = SurfaceTypes[FirstVertexIndex];
		const bool bIsTranslucent = !Blend.IsNull() &&
			SurfaceTypeTable.CubicTranslucentTypes.Contains(Blend.GetTopLayer().Type);

		FMeshData& TargetData = bIsTranslucent ? TranslucentData : MaskedData;

		// Add 4 vertices for this quad
		const int32 NewBaseVertex = TargetData.Vertices.Num();
		for (int32 V = 0; V < 4; V++)
		{
			const int32 OldVertexIndex = FirstVertexIndex + V;
			TargetData.Vertices.Add(Vertices[OldVertexIndex]);
			TargetData.Normals.Add(Normals[OldVertexIndex]);
			TargetData.SurfaceTypes.Add(SurfaceTypes[OldVertexIndex]);
			TargetData.Cells.Add(Cells[OldVertexIndex]);

			if (!SurfaceTypes[OldVertexIndex].IsNull())
			{
				TargetData.UsedSurfaceTypesSet.Add(SurfaceTypes[OldVertexIndex].GetTopLayer().Type);
			}
		}

		// Add 6 indices for this quad (remapped to new vertex base)
		TargetData.Indices.Add(NewBaseVertex + 0);
		TargetData.Indices.Add(NewBaseVertex + 2);
		TargetData.Indices.Add(NewBaseVertex + 1);
		TargetData.Indices.Add(NewBaseVertex + 0);
		TargetData.Indices.Add(NewBaseVertex + 3);
		TargetData.Indices.Add(NewBaseVertex + 2);
	}

	// Helper to create FVoxelMesh from mesh data
	const auto CreateMeshFromData = [this](FMeshData& Data, bool bIsTranslucentMesh) -> TSharedPtr<FVoxelMesh>
	{
		if (Data.Indices.Num() == 0)
		{
			return nullptr;
		}

		// Convert normals to octahedron format
		TVoxelArray<FVoxelOctahedron> OctahedronNormals;
		FVoxelUtilities::SetNumFast(OctahedronNormals, Data.Normals.Num());
		for (int32 Index = 0; Index < Data.Normals.Num(); Index++)
		{
			OctahedronNormals[Index] = FVoxelOctahedron(Data.Normals[Index]);
		}

		TVoxelArray<FVoxelSurfaceType> UsedSurfaceTypes = Data.UsedSurfaceTypesSet.Array();
		UsedSurfaceTypes.Sort();

		// No LODs for cubic mesher
		TVoxelArray<FVoxelMesh::FLOD> EmptyLODs;

		// Empty metadata for now (could be extended to query per-mesh)
		TVoxelMap<FVoxelMetadataRef, TSharedRef<const FVoxelBuffer>> MetadataToBuffer;

		return MakeShared<FVoxelMesh>(
			0, // ChunkLOD = 0
			ChunkOffset,
			ChunkSize,
			MoveTemp(UsedSurfaceTypes),
			MoveTemp(MetadataToBuffer),
			MoveTemp(Data.Indices),
			MoveTemp(Data.Vertices),
			MoveTemp(OctahedronNormals),
			MoveTemp(Data.SurfaceTypes),
			MoveTemp(Data.Cells),
			MoveTemp(EmptyLODs),
			bIsTranslucentMesh);
	};

	TVoxelArray<TSharedPtr<FVoxelMesh>> Result;

	// Always add masked mesh first (if non-empty)
	if (TSharedPtr<FVoxelMesh> MaskedMesh = CreateMeshFromData(MaskedData, false))
	{
		Result.Add(MaskedMesh);
	}

	// Add translucent mesh if non-empty
	if (TSharedPtr<FVoxelMesh> TranslucentMesh = CreateMeshFromData(TranslucentData, true))
	{
		Result.Add(TranslucentMesh);
	}

	return Result;
}

void FVoxelCubicMesher::QueryDistancesAndSurfaceTypes()
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicMesher::QueryDistancesAndSurfaceTypes");

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
			// Use sparse sampling to determine which regions need full-resolution querying
			const TVoxelArray<FVoxelIntBox> BoundsToQuery = FVoxelSparseSampler::ComputeBoundsToQuery(
				Query,
				*VolumeLayer,
				VolumeBounds,
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

void FVoxelCubicMesher::GenerateFaces(const bool bAirFacesOnly)
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicMesher::GenerateFaces");

	const bool bHideFacesAtNaNBoundaries = MegaMaterialProxy.ShouldHideFacesAtNaNBoundaries();

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

					bool bShouldGenerateFace = false;

					if (!IsSolid(NeighborX, NeighborY, NeighborZ))
					{
						// Neighbor is air - always generate (unless NaN boundary hiding)
						if (bHideFacesAtNaNBoundaries &&
							IsNaN(NeighborX, NeighborY, NeighborZ))
						{
							continue;
						}
						bShouldGenerateFace = true;
					}
					else if (!bAirFacesOnly)
					{
						// Neighbor is solid - check occlusion rules
						const FVoxelSurfaceType NeighborType = BlockTypeGrid[GetGridIndex(NeighborX, NeighborY, NeighborZ)];

						if (CellBlockType == NeighborType)
						{
							// Same type: generate face if this type doesn't self-occlude
							bShouldGenerateFace = SurfaceTypeTable.CubicNonSelfOccluding.Contains(CellBlockType);
						}
						else
						{
							// Different type: generate face if neighbor doesn't occlude others
							bShouldGenerateFace = SurfaceTypeTable.CubicNonOtherOccluding.Contains(NeighborType);
						}
					}

					if (bShouldGenerateFace)
					{
						// Convert from grid coords to chunk-local coords
						const FIntVector LocalCell(X - 1, Y - 1, Z - 1);
						AddFace(LocalCell, Face, CellBlockType);
					}
				}
			}
		}
	}
}

void FVoxelCubicMesher::AddFace(
	const FIntVector& CellPosition,
	const int32 FaceDirection,
	const FVoxelSurfaceType BlockType)
{
	static constexpr int32 NumFaces = 6;
	checkVoxelSlow(FaceDirection < NumFaces);

	// Face vertex offsets (CCW winding when viewed from outside)
	// Each face is a unit quad at the cell boundary
	static const FVector3f FaceVertices[NumFaces][4] = {
		// +X face (at X+1)
		{{1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 1}},
		// -X face (at X=0)
		{{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}},
		// +Y face (at Y+1)
		{{0, 1, 0}, {0, 1, 1}, {1, 1, 1}, {1, 1, 0}},
		// -Y face (at Y=0)
		{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},
		// +Z face (at Z+1)
		{{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}},
		// -Z face (at Z=0)
		{{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}}
	};

	static const FVector3f FaceNormalVectors[NumFaces] = {
		{ 1,  0,  0},  // +X
		{-1,  0,  0},  // -X
		{ 0,  1,  0},  // +Y
		{ 0, -1,  0},  // -Y
		{ 0,  0,  1},  // +Z
		{ 0,  0, -1}   // -Z
	};

	const FVector3f CellBase = FVector3f(CellPosition);
	const FVector3f Normal = FaceNormalVectors[FaceDirection];
	const int32 BaseVertex = Vertices.Num();

	// Create surface type blend with single layer at full weight
	FVoxelSurfaceTypeBlend Blend;
	Blend.InitializeFromType(BlockType);

	// Add 4 vertices for the quad
	for (int32 V = 0; V < 4; V++)
	{
		Vertices.Add(CellBase + FaceVertices[FaceDirection][V]);
		Normals.Add(Normal);
		SurfaceTypes.Add(Blend);

		// Store cell position for potential LOD displacement
		Cells.Add({
			int16(CellPosition.X),
			int16(CellPosition.Y),
			int16(CellPosition.Z)
		});
	}

	// Add 2 triangles (6 indices) for the quad
	// CW winding when viewed from outside: 0-2-1, 0-3-2
	Indices.Add(BaseVertex + 0);
	Indices.Add(BaseVertex + 2);
	Indices.Add(BaseVertex + 1);

	Indices.Add(BaseVertex + 0);
	Indices.Add(BaseVertex + 3);
	Indices.Add(BaseVertex + 2);
}

void FVoxelCubicMesher::QueryHeightLayerCells(const FVoxelQuery& Query, const FVoxelIntBox& VolumeBounds)
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicMesher::QueryHeightLayerCells");

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

	// Use cached heights if available, otherwise compute them
	Heights = CachedHeights;

	if (!Heights)
	{
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

		const TSharedRef<FVoxelCellGeneratorHeights> NewHeights = MakeShared<FVoxelCellGeneratorHeights>();
		NewHeights->Indices = HeightQueryIndices;
		FVoxelUtilities::SetNumFast(NewHeights->Heights, HeightQueryIndices.Count_int32());
		FVoxelUtilities::SetAll(NewHeights->Heights, FVoxelUtilities::NaNf());

		const FVoxelQuery HeightQuery(
			0, // LOD 0 for cubic
			Layers,
			SurfaceTypeTable,
			NewHeights->DependencyCollector,
			nullptr);

		VOXEL_SCOPE_COUNTER_NUM("Query heights", NewHeights->Heights.Num());

		HeightLayer->Sample(FVoxelHeightBulkQuery::Create(
			HeightQuery,
			NewHeights->Heights,
			FVector2D(Start) + (FVector2D(HeightQueryIndices.Min) + 0.5) * CellSize,
			HeightQueryIndices.Size(),
			CellSize));

		Heights = NewHeights;
	}

	DependencyCollector.AddDependencies(Heights->DependencyCollector);

	// If volume covers full size, heights are only cached for reuse
	if (VolumeBounds.IsValid() &&
		VolumeBounds == FVoxelIntBox(0, DataSize))
	{
		return;
	}

	const FVoxelIntBox2D HeightIndices = Heights->Indices;
	const FIntPoint Size2D = HeightIndices.Size();
	const TConstVoxelArrayView<float> HeightView = Heights->Heights.View();

	// Query surface types at 2D positions (same grid as heights)
	// Height stamps define surface types per 2D position, applied to all cells in each column
	FVoxelDoubleVector2DBuffer QueryPositions;
	QueryPositions.Allocate(HeightView.Num());

	for (int32 LocalY = 0; LocalY < Size2D.Y; LocalY++)
	{
		for (int32 LocalX = 0; LocalX < Size2D.X; LocalX++)
		{
			const int32 Index = FVoxelUtilities::Get2DIndex<int32>(Size2D, LocalX, LocalY);
			const int32 GridX = HeightIndices.Min.X + LocalX;
			const int32 GridY = HeightIndices.Min.Y + LocalY;

			// Query at cell center (2D)
			const FVector2D Position = FVector2D(Start) + FVector2D(GridX + 0.5, GridY + 0.5) * CellSize;
			QueryPositions.Set(Index, Position);
		}
	}

	FVoxelSurfaceTypeBlendBuffer SurfaceTypeBuffer;
	SurfaceTypeBuffer.AllocateZeroed(HeightView.Num());

	VOXEL_SCOPE_COUNTER_NUM("Query height surface types", HeightView.Num());

	(void)Query.SampleHeightLayer(
		WeakHeightLayer.GetValue(),
		QueryPositions,
		SurfaceTypeBuffer.View(),
		{});

	// For cubic mesher, we sample at cell centers
	// A cell at Z is solid if its center (Z + 0.5) is below the height
	for (int32 LocalY = 0; LocalY < Size2D.Y; LocalY++)
	{
		for (int32 LocalX = 0; LocalX < Size2D.X; LocalX++)
		{
			const int32 Index2D = FVoxelUtilities::Get2DIndex<int32>(Size2D, LocalX, LocalY);
			const float Height = HeightView[Index2D];

			if (FVoxelUtilities::IsNaN(Height))
			{
				continue;
			}

			const FVoxelSurfaceType SurfaceType = SurfaceTypeBuffer[Index2D].GetTopLayer().Type;

			const int32 GridX = HeightIndices.Min.X + LocalX;
			const int32 GridY = HeightIndices.Min.Y + LocalY;

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

void FVoxelCubicMesher::ResolveSmartSurfaceTypes()
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicMesher::ResolveSmartSurfaceTypes");

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
			// Fallback to up vector if gradient is zero (all NaN neighbors)
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

void FVoxelCubicMesher::FilterInvisibleSurfaceTypes()
{
	VOXEL_SCOPE_COUNTER("FVoxelCubicMesher::FilterInvisibleSurfaceTypes");

	DependencyCollector.AddDependency(*SurfaceTypeTable.InvisibleSurfaceTypesDependency);

	if (SurfaceTypeTable.InvisibleSurfaceTypes.Num() == 0)
	{
		return;
	}

	// Check if any used surface types are invisible
	bool bHasInvisible = false;
	for (const FVoxelSurfaceTypeBlend& Blend : SurfaceTypes)
	{
		if (!Blend.IsNull() &&
			SurfaceTypeTable.InvisibleSurfaceTypes.Contains(Blend.GetTopLayer().Type))
		{
			bHasInvisible = true;
			break;
		}
	}

	if (!bHasInvisible)
	{
		return;
	}

	// Build old-to-new vertex index mapping
	TVoxelArray<int32> OldToNewVertex;
	FVoxelUtilities::SetNumFast(OldToNewVertex, Vertices.Num());
	FVoxelUtilities::SetAll(OldToNewVertex, -1);

	int32 NewVertexCount = 0;
	for (int32 Index = 0; Index < Vertices.Num(); Index++)
	{
		const FVoxelSurfaceTypeBlend& Blend = SurfaceTypes[Index];
		if (!Blend.IsNull() &&
			SurfaceTypeTable.InvisibleSurfaceTypes.Contains(Blend.GetTopLayer().Type))
		{
			continue;
		}

		OldToNewVertex[Index] = NewVertexCount++;
	}

	if (NewVertexCount == Vertices.Num())
	{
		return;
	}

	// Compact vertex arrays
	TVoxelArray<FVector3f> NewVertices;
	TVoxelArray<FVector3f> NewNormals;
	TVoxelArray<FVoxelSurfaceTypeBlend> NewSurfaceTypes;
	TVoxelArray<FVoxelMesh::FCell> NewCells;
	NewVertices.Reserve(NewVertexCount);
	NewNormals.Reserve(NewVertexCount);
	NewSurfaceTypes.Reserve(NewVertexCount);
	NewCells.Reserve(NewVertexCount);

	for (int32 Index = 0; Index < Vertices.Num(); Index++)
	{
		if (OldToNewVertex[Index] == -1)
		{
			continue;
		}

		NewVertices.Add(Vertices[Index]);
		NewNormals.Add(Normals[Index]);
		NewSurfaceTypes.Add(SurfaceTypes[Index]);
		NewCells.Add(Cells[Index]);
	}

	// Remap indices, filtering out triangles with invisible vertices
	TVoxelArray<int32> NewIndices;
	NewIndices.Reserve(Indices.Num());

	checkVoxelSlow(Indices.Num() % 3 == 0);
	for (int32 TriIndex = 0; TriIndex < Indices.Num() / 3; TriIndex++)
	{
		const int32 OldA = Indices[3 * TriIndex + 0];
		const int32 OldB = Indices[3 * TriIndex + 1];
		const int32 OldC = Indices[3 * TriIndex + 2];

		const int32 NewA = OldToNewVertex[OldA];
		const int32 NewB = OldToNewVertex[OldB];
		const int32 NewC = OldToNewVertex[OldC];

		if (NewA == -1 ||
			NewB == -1 ||
			NewC == -1)
		{
			continue;
		}

		NewIndices.Add(NewA);
		NewIndices.Add(NewB);
		NewIndices.Add(NewC);
	}

	Vertices = MoveTemp(NewVertices);
	Normals = MoveTemp(NewNormals);
	SurfaceTypes = MoveTemp(NewSurfaceTypes);
	Cells = MoveTemp(NewCells);
	Indices = MoveTemp(NewIndices);
}