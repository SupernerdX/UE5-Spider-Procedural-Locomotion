// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelCellGenerator.h"
#include "VoxelSparseSampler.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "VoxelHeightLayer.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelCellGeneratorDebug, false,
	"voxel.CellGenerator.Debug",
	"If true will draw debug points for bounds processing");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelCellGenerator::FVoxelCellGenerator(
	const int32 LOD,
	const FVoxelLayers& Layers,
	const FVoxelSurfaceTypeTable& SurfaceTypeTable,
	const FVector& Start,
	const FIntVector& Size,
	const float CellSize,
	const FVoxelWeakStackLayer& WeakLayer,
	const TSharedPtr<const FVoxelCellGeneratorHeights>& CachedHeights)
	: LOD(LOD)
	, Layers(Layers)
	, SurfaceTypeTable(SurfaceTypeTable)
	, Start(Start)
	, Size(Size)
	, CellSize(CellSize)
	, WeakLayer(WeakLayer)
	, Bounds(FVoxelIntBox(0, Size).ToVoxelBox().Scale(CellSize).ShiftBy(Start))
	, Query(
		LOD,
		Layers,
		SurfaceTypeTable,
		DependencyCollector,
		nullptr)

	, HeightLayer(INLINE_LAMBDA -> TSharedPtr<const FVoxelHeightLayer>
	{
		const TVoxelOptional<FVoxelWeakStackLayer> WeakHeightLayer = Query.GetFirstHeightLayer(WeakLayer);

		if (!WeakHeightLayer)
		{
			return {};
		}

		const TSharedPtr<const FVoxelHeightLayer> Result = Layers.FindHeightLayer(WeakHeightLayer.GetValue(), DependencyCollector);
		ensure(Result);
		return Result;
	})
	, VolumeLayer(Layers.FindVolumeLayer(WeakLayer, DependencyCollector))
	, Heights(CachedHeights)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelCellGenerator::ForeachCell(
	FVoxelDependencyCollector& OutDependencyCollector,
	const TVoxelFunctionRef<void(const FIntVector&, const TVoxelStaticArray<float, 8>&)> Lambda,
	FVoxelCellGeneratorOutput* OutData)
{
	VOXEL_FUNCTION_COUNTER();

	ON_SCOPE_EXIT
	{
		OutDependencyCollector.AddDependencies(DependencyCollector);
	};

	const FVoxelOptionalIntBox OptionalVolumeBounds = INLINE_LAMBDA -> FVoxelOptionalIntBox
	{
		if (!VolumeLayer)
		{
			return {};
		}

		const FVoxelOptionalBox VolumeBounds = VolumeLayer->GetVolumeStampBounds(
			Query,
			Bounds,
			EVoxelStampBehavior::AffectShape);

		if (!VolumeBounds.IsValid())
		{
			return {};
		}

		return FVoxelOptionalIntBox(
			FVoxelIntBox::FromFloatBox_WithPadding(VolumeBounds.GetBox().ShiftBy(-Start) / CellSize)
			.IntersectWith(FVoxelIntBox(0, Size)));
	};

	if (!OptionalVolumeBounds ||
		!OptionalVolumeBounds->IsValid())
	{
		ForeachHeightCell<false>(Lambda, OutData, {});
		return;
	}

	const FVoxelIntBox VolumeBounds = OptionalVolumeBounds.GetBox();
	const FIntVector VolumeSize = VolumeBounds.Size();

	ForeachHeightCell<true>(Lambda, OutData, VolumeBounds);

	const TVoxelArray<FVoxelIntBox> AllBoundsToCompute = FVoxelSparseSampler::ComputeBoundsToQuery(
		Query,
		*VolumeLayer,
		VolumeBounds,
		Start,
		CellSize,
		&Layers,
		LOD);

	TVoxelArray<float> Distances;
	{
		VOXEL_SCOPE_COUNTER_NUM("Allocate distances", VolumeBounds.Count_int32());

		FVoxelUtilities::SetNumFast(Distances, VolumeBounds.Count_int32());
		FVoxelUtilities::SetAll(Distances, FVoxelUtilities::NaNf());
	}

	for (const FVoxelIntBox& BoundsToCompute : AllBoundsToCompute)
	{
		VOXEL_SCOPE_COUNTER("Process bounds");
		checkVoxelSlow(VolumeBounds.Contains(BoundsToCompute));

		if (GVoxelCellGeneratorDebug)
		{
			VOXEL_SCOPE_COUNTER("Debug");

			FVoxelDebugDrawer Drawer(Layers.World);
			Drawer.Color(FLinearColor::MakeRandomColor());

			for (int32 Z = BoundsToCompute.Min.Z; Z < BoundsToCompute.Max.Z; Z++)
			{
				for (int32 Y = BoundsToCompute.Min.Y; Y < BoundsToCompute.Max.Y; Y++)
				{
					for (int32 X = BoundsToCompute.Min.X; X < BoundsToCompute.Max.X; X++)
					{
						Drawer.DrawPoint(Start + FVector(X, Y, Z) * CellSize);
					}
				}
			}
		}

		const auto QueryHeights = [&](const TVoxelArrayView<float> OutHeights, const FVoxelIntBox2D& QueryIndices2D)
		{
			VOXEL_SCOPE_COUNTER("Copy heights");
			checkVoxelSlow(OutHeights.Num() == QueryIndices2D.Count_int32());

			if (!Heights)
			{
				ensureVoxelSlowNoSideEffects(VolumeLayer->HasIntersectStamps());
				return false;
			}

			const FVoxelIntBox2D Indices2D = QueryIndices2D.ShiftBy(FIntPoint(VolumeBounds.Min.X, VolumeBounds.Min.Y));
			checkVoxelSlow(FVoxelIntBox2D(0, FIntPoint(Size.X, Size.Y)).Contains(Indices2D));

			if (!Heights->Indices.Contains(Indices2D))
			{
				// Height stamps don't encompass the entire chunk, NaN the padding
				FVoxelUtilities::SetAll(OutHeights, FVoxelUtilities::NaNf());
			}

			const FVoxelIntBox2D Indices2DToCopy = Indices2D.IntersectWith(Heights->Indices);
			const int32 CopySizeX = Indices2DToCopy.Size().X;
			const int32 QuerySizeX = QueryIndices2D.Size().X;
			const int32 HeightSizeX = Heights->Indices.Size().X;

			for (int32 Y = Indices2DToCopy.Min.Y; Y < Indices2DToCopy.Max.Y; Y++)
			{
				const int32 X = Indices2DToCopy.Min.X;
				checkVoxelSlow(Heights->Indices.GetX().Contains(X));
				checkVoxelSlow(Heights->Indices.GetY().Contains(Y));

				const int32 QueryX = X - VolumeBounds.Min.X;
				const int32 QueryY = Y - VolumeBounds.Min.Y;
				checkVoxelSlow(QueryIndices2D.GetX().Contains(QueryX));
				checkVoxelSlow(QueryIndices2D.GetY().Contains(QueryY));

				FVoxelUtilities::Memcpy(
					OutHeights.Slice(QuerySizeX * (QueryY - QueryIndices2D.Min.Y) + QueryX - QueryIndices2D.Min.X, CopySizeX),
					Heights->Heights.View().Slice(HeightSizeX * (Y - Heights->Indices.Min.Y) + X - Heights->Indices.Min.X, CopySizeX));
			}

			return true;
		};

		FVoxelVolumeBulkQuery BulkQuery = FVoxelVolumeBulkQuery::Create(
			Query,
			Distances,
			Start + FVector(VolumeBounds.Min) * CellSize,
			VolumeSize,
			BoundsToCompute.ShiftBy(-VolumeBounds.Min),
			CellSize);

		BulkQuery.QueryHeights = QueryHeights;

		VolumeLayer->Sample(BulkQuery);
	}

	VOXEL_SCOPE_COUNTER_NUM("Iterate distances", VolumeBounds.Count_int32());

	if (OutData)
	{
		OutData->VolumeData = 
		{
			Distances,
			VolumeBounds
		};
	}

	for (int32 LocalX = 0; LocalX < VolumeSize.X - 1; LocalX++)
	{
		for (int32 LocalY = 0; LocalY < VolumeSize.Y - 1; LocalY++)
		{
			for (int32 LocalZ = 0; LocalZ < VolumeSize.Z - 1; LocalZ++)
			{
				TVoxelStaticArray<float, 8> CellDistances{ NoInit };

				CellDistances[0] = Distances[FVoxelUtilities::Get3DIndex<int32>(VolumeSize, LocalX + 0, LocalY + 0, LocalZ + 0)];
				if (FVoxelUtilities::IsNaN(CellDistances[0])) { continue; }
				CellDistances[1] = Distances[FVoxelUtilities::Get3DIndex<int32>(VolumeSize, LocalX + 1, LocalY + 0, LocalZ + 0)];
				if (FVoxelUtilities::IsNaN(CellDistances[1])) { continue; }
				CellDistances[2] = Distances[FVoxelUtilities::Get3DIndex<int32>(VolumeSize, LocalX + 0, LocalY + 1, LocalZ + 0)];
				if (FVoxelUtilities::IsNaN(CellDistances[2])) { continue; }
				CellDistances[3] = Distances[FVoxelUtilities::Get3DIndex<int32>(VolumeSize, LocalX + 1, LocalY + 1, LocalZ + 0)];
				if (FVoxelUtilities::IsNaN(CellDistances[3])) { continue; }
				CellDistances[4] = Distances[FVoxelUtilities::Get3DIndex<int32>(VolumeSize, LocalX + 0, LocalY + 0, LocalZ + 1)];
				if (FVoxelUtilities::IsNaN(CellDistances[4])) { continue; }
				CellDistances[5] = Distances[FVoxelUtilities::Get3DIndex<int32>(VolumeSize, LocalX + 1, LocalY + 0, LocalZ + 1)];
				if (FVoxelUtilities::IsNaN(CellDistances[5])) { continue; }
				CellDistances[6] = Distances[FVoxelUtilities::Get3DIndex<int32>(VolumeSize, LocalX + 0, LocalY + 1, LocalZ + 1)];
				if (FVoxelUtilities::IsNaN(CellDistances[6])) { continue; }
				CellDistances[7] = Distances[FVoxelUtilities::Get3DIndex<int32>(VolumeSize, LocalX + 1, LocalY + 1, LocalZ + 1)];
				if (FVoxelUtilities::IsNaN(CellDistances[7])) { continue; }

				if (CellDistances[0] >= 0)
				{
					if (CellDistances[1] >= 0 &&
						CellDistances[2] >= 0 &&
						CellDistances[3] >= 0 &&
						CellDistances[4] >= 0 &&
						CellDistances[5] >= 0 &&
						CellDistances[6] >= 0 &&
						CellDistances[7] >= 0)
					{
						continue;
					}
				}
				else
				{
					if (CellDistances[1] < 0 &&
						CellDistances[2] < 0 &&
						CellDistances[3] < 0 &&
						CellDistances[4] < 0 &&
						CellDistances[5] < 0 &&
						CellDistances[6] < 0 &&
						CellDistances[7] < 0)
					{
						continue;
					}
				}

				const FIntVector Position = VolumeBounds.Min + FIntVector(LocalX, LocalY, LocalZ);

				ensureVoxelSlowNoSideEffects(IsValidCellDistances(CellDistances));
				ensureVoxelSlowNoSideEffects(VolumeBounds.Contains(Position));

				Lambda(
					Position,
					CellDistances);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<bool bCheckVolumeBounds>
void FVoxelCellGenerator::ForeachHeightCell(
	const TVoxelFunctionRef<void(const FIntVector&, const TVoxelStaticArray<float, 8>&)> Lambda,
	FVoxelCellGeneratorOutput* OutData,
	const FVoxelIntBox& VolumeBounds)
{
	VOXEL_FUNCTION_COUNTER();

	if (VolumeLayer &&
		VolumeLayer->HasIntersectStamps())
	{
		// Can't generate height cells
		return;
	}

	if (!HeightLayer ||
		!HeightLayer->HasStamps(
			Query,
			Bounds,
			EVoxelStampBehavior::AffectShape,
			true))
	{
		return;
	}

	if (!Heights)
	{
		VOXEL_SCOPE_COUNTER("Compute heights");

		const FVoxelOptionalBox2D HeightBounds = HeightLayer->GetStampBounds(
			Query,
			FVoxelBox2D(Bounds),
			EVoxelStampBehavior::AffectShape);

		if (!ensure(HeightBounds.IsValid()))
		{
			return;
		}

		const FVoxelIntBox2D Indices =
			FVoxelIntBox2D::FromFloatBox_WithPadding(HeightBounds.GetBox().ShiftBy(-FVector2D(Start)) / CellSize)
			.IntersectWith(FVoxelIntBox2D(0, FIntPoint(Size.X, Size.Y)));

		const TSharedRef<FVoxelCellGeneratorHeights> NewHeights = MakeShared<FVoxelCellGeneratorHeights>();
		NewHeights->Indices = Indices;
		FVoxelUtilities::SetNumFast(NewHeights->Heights, Indices.Count_int32());
		FVoxelUtilities::SetAll(NewHeights->Heights, FVoxelUtilities::NaNf());

		const FVoxelQuery HeightQuery(
			LOD,
			Layers,
			SurfaceTypeTable,
			NewHeights->DependencyCollector,
			nullptr);

		VOXEL_SCOPE_COUNTER_NUM("Query heights", NewHeights->Heights.Num());

		HeightLayer->Sample(FVoxelHeightBulkQuery::Create(
			HeightQuery,
			NewHeights->Heights,
			FVector2D(Start) + FVector2D(Indices.Min) * CellSize,
			Indices.Size(),
			CellSize));

		Heights = NewHeights;
	}

	DependencyCollector.AddDependencies(Heights->DependencyCollector);

	if (bCheckVolumeBounds &&
		VolumeBounds == FVoxelIntBox(0, Size))
	{
		// Still cache heights so we can reuse it, but no need to iterate
		return;
	}

	const FVoxelIntBox2D Indices = Heights->Indices;
	const FIntPoint Size2D = Indices.Size();
	const TConstVoxelArrayView<float> HeightView = Heights->Heights.View();

	if (OutData)
	{
		OutData->HeightData = 
		{
			HeightView.Array(),
			Indices
		};
	}

	for (int32 LocalY = 0; LocalY < Size2D.Y - 1; LocalY++)
	{
		for (int32 LocalX = 0; LocalX < Size2D.X - 1; LocalX++)
		{
			const float Height00 = HeightView[FVoxelUtilities::Get2DIndex<int32>(Size2D, LocalX + 0, LocalY + 0)];
			const float Height01 = HeightView[FVoxelUtilities::Get2DIndex<int32>(Size2D, LocalX + 1, LocalY + 0)];
			const float Height10 = HeightView[FVoxelUtilities::Get2DIndex<int32>(Size2D, LocalX + 0, LocalY + 1)];
			const float Height11 = HeightView[FVoxelUtilities::Get2DIndex<int32>(Size2D, LocalX + 1, LocalY + 1)];

			if (FVoxelUtilities::IsNaN(Height00) ||
				FVoxelUtilities::IsNaN(Height01) ||
				FVoxelUtilities::IsNaN(Height10) ||
				FVoxelUtilities::IsNaN(Height11))
			{
				continue;
			}

			const float MinHeight = FMath::Min3(Height00, Height01, FMath::Min(Height10, Height11));
			const float MaxHeight = FMath::Max3(Height00, Height01, FMath::Max(Height10, Height11));

			const int32 MinZ = FMath::Max(FMath::FloorToInt32((MinHeight - Start.Z) / CellSize) - 1, 0);
			const int32 MaxZ = FMath::Min(FMath::FloorToInt32((MaxHeight - Start.Z) / CellSize) + 1, Size.Z - 1);

			for (int32 Z = MinZ; Z < MaxZ; Z++)
			{
				const double WorldMinZ = Start.Z + (Z + 0) * CellSize;
				const double WorldMaxZ = Start.Z + (Z + 1) * CellSize;

				const TVoxelStaticArray<float, 8> CellDistances
				{
					WorldMinZ - Height00,
					WorldMinZ - Height01,
					WorldMinZ - Height10,
					WorldMinZ - Height11,
					WorldMaxZ - Height00,
					WorldMaxZ - Height01,
					WorldMaxZ - Height10,
					WorldMaxZ - Height11
				};

				if (CellDistances[0] >= 0)
				{
					if (CellDistances[1] >= 0 &&
						CellDistances[2] >= 0 &&
						CellDistances[3] >= 0 &&
						CellDistances[4] >= 0 &&
						CellDistances[5] >= 0 &&
						CellDistances[6] >= 0 &&
						CellDistances[7] >= 0)
					{
						continue;
					}
				}
				else
				{
					if (CellDistances[1] < 0 &&
						CellDistances[2] < 0 &&
						CellDistances[3] < 0 &&
						CellDistances[4] < 0 &&
						CellDistances[5] < 0 &&
						CellDistances[6] < 0 &&
						CellDistances[7] < 0)
					{
						continue;
					}
				}

				const FIntVector Position = FIntVector(
					Indices.Min.X + LocalX,
					Indices.Min.Y + LocalY,
					Z);

				ensureVoxelSlowNoSideEffects(IsValidCellDistances(CellDistances));
				ensureVoxelSlowNoSideEffects(FVoxelIntBox(0, Size - 1).Contains(Position));

				if (bCheckVolumeBounds &&
					FVoxelIntBox(VolumeBounds.Min, VolumeBounds.Max - 1).Contains(Position))
				{
					// Will be handled by the volume pass
					continue;
				}

				Lambda(
					Position,
					CellDistances);
			}
		}
	}
}

bool FVoxelCellGenerator::IsValidCellDistances(const TVoxelStaticArray<float, 8>& CellDistances)
{
	for (const float Distance : CellDistances)
	{
		if (!ensure(!FVoxelUtilities::IsNaN(Distance)))
		{
			return false;
		}
	}

	for (const float Distance : CellDistances)
	{
		if ((Distance >= 0) != CellDistances[0] >= 0)
		{
			return true;
		}
	}

	ensure(false);
	return false;
}