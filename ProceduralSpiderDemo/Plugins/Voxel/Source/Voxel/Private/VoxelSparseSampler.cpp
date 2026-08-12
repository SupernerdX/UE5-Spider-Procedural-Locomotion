// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSparseSampler.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "VoxelVolumeLayer.h"

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelSparseSamplerDebug, false,
	"voxel.SparseSampler.Debug",
	"If true will draw debug points for sparse sampling");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, float, GVoxelSparseSamplerTolerance, 0.1f,
	"voxel.SparseSampler.Tolerance",
	"Used to make sparse sampling be less aggressive in cell-skipping");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelSparseSamplerPropagate, true,
	"voxel.SparseSampler.Propagate",
	"Add a 1-cell wide border to sparse sampling checks");

VOXEL_CONSOLE_VARIABLE(
	VOXEL_API, bool, GVoxelSparseSamplerCheckSigns, true,
	"voxel.SparseSampler.CheckSigns",
	"Force mesh if sparse cell changes sign");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelArray<FVoxelIntBox> FVoxelSparseSampler::ComputeBoundsToQuery(
	const FVoxelQuery& Query,
	const FVoxelVolumeLayer& VolumeLayer,
	const FVoxelIntBox& VolumeBounds,
	const FVector& Start,
	const float CellSize,
	const FVoxelLayers* Layers,
	const int32 LOD)
{
	VOXEL_SCOPE_COUNTER("FVoxelSparseSampler::ComputeBoundsToQuery");

	const FVoxelIntBox SparseVolumeBounds = VolumeBounds.DivideBigger(SparseMultiplier).Extend(1);

	const FVector SparseStart =
		Start +
		FVector(SparseVolumeBounds.Min) * CellSize * SparseMultiplier +
		CellSize * (SparseMultiplier - 1) / 2.f;

	TVoxelArray<float> SparseDistances;
	{
		VOXEL_SCOPE_COUNTER_NUM("Query sparse distances", SparseVolumeBounds.Count_int32());

		FVoxelUtilities::SetNumFast(SparseDistances, SparseVolumeBounds.Count_int32());
		FVoxelUtilities::SetAll(SparseDistances, FVoxelUtilities::NaNf());

		VolumeLayer.Sample(FVoxelVolumeBulkQuery::Create(
			Query,
			SparseDistances,
			SparseStart,
			SparseVolumeBounds.Size(),
			CellSize * SparseMultiplier));
	}

	const FIntVector SparseSize = SparseVolumeBounds.Size();
	const int32 SparseCount = SparseSize.X * SparseSize.Y * SparseSize.Z;

	FVoxelBitArray ShouldQuery;
	{
		VOXEL_SCOPE_COUNTER("ShouldQuery");

		{
			VOXEL_SCOPE_COUNTER("First pass");

			ShouldQuery.SetNum(SparseCount, false);

			for (int32 Z = 0; Z < SparseSize.Z; Z++)
			{
				for (int32 Y = 0; Y < SparseSize.Y; Y++)
				{
					for (int32 X = 0; X < SparseSize.X; X++)
					{
						const int32 Index = FVoxelUtilities::Get3DIndex<int32>(SparseSize, X, Y, Z);
						const double Distance = SparseDistances[Index];

						if (FVoxelUtilities::IsNaN(Distance))
						{
							continue;
						}

						if (FMath::Abs(Distance) > SparseMultiplier * CellSize * UE_HALF_SQRT_3 * (1.f + GVoxelSparseSamplerTolerance))
						{
							continue;
						}

						ShouldQuery[Index] = true;
					}
				}
			}
		}

		if (GVoxelSparseSamplerCheckSigns)
		{
			VOXEL_SCOPE_COUNTER("Check signs");

			// Check transitions from negative to positive and force them to be meshed
			// This avoids holes when the distance field is incorrect

			for (int32 Z = 0; Z < SparseSize.Z; Z++)
			{
				for (int32 Y = 0; Y < SparseSize.Y; Y++)
				{
					for (int32 X = 0; X < SparseSize.X; X++)
					{
						const int32 Index = FVoxelUtilities::Get3DIndex<int32>(SparseSize, X, Y, Z);
						if (ShouldQuery[Index])
						{
							continue;
						}

						const bool bIsNegative = SparseDistances[Index] < 0;

						const bool bAnyNeighborDifferent = INLINE_LAMBDA
						{
							const FIntVector Min = FVoxelUtilities::ComponentMax(FIntVector(0), FIntVector(X - 1, Y - 1, Z - 1));
							const FIntVector Max = FVoxelUtilities::ComponentMin(SparseSize - 1, FIntVector(X + 1, Y + 1, Z + 1));

							for (int32 NeighborZ = Min.Z; NeighborZ <= Max.Z; NeighborZ++)
							{
								for (int32 NeighborY = Min.Y; NeighborY <= Max.Y; NeighborY++)
								{
									for (int32 NeighborX = Min.X; NeighborX <= Max.X; NeighborX++)
									{
										const bool bNeighborIsNegative = SparseDistances[FVoxelUtilities::Get3DIndex<int32>(SparseSize, NeighborX, NeighborY, NeighborZ)] < 0;
										if (bNeighborIsNegative != bIsNegative)
										{
											return true;
										}
									}
								}
							}

							return false;
						};

						if (bAnyNeighborDifferent)
						{
							ShouldQuery[Index] = true;
						}
					}
				}
			}
		}

		if (GVoxelSparseSamplerPropagate)
		{
			VOXEL_SCOPE_COUNTER("Propagate");

			// Don't recursively propagate
			const FVoxelBitArray ShouldQueryCopy = ShouldQuery;

			for (int32 Z = 0; Z < SparseSize.Z; Z++)
			{
				for (int32 Y = 0; Y < SparseSize.Y; Y++)
				{
					for (int32 X = 0; X < SparseSize.X; X++)
					{
						if (!ShouldQueryCopy[FVoxelUtilities::Get3DIndex<int32>(SparseSize, X, Y, Z)])
						{
							continue;
						}

						const FIntVector Min = FVoxelUtilities::ComponentMax(FIntVector(0), FIntVector(X - 1, Y - 1, Z - 1));
						const FIntVector Max = FVoxelUtilities::ComponentMin(SparseSize - 1, FIntVector(X + 1, Y + 1, Z + 1));

						for (int32 NeighborZ = Min.Z; NeighborZ <= Max.Z; NeighborZ++)
						{
							for (int32 NeighborY = Min.Y; NeighborY <= Max.Y; NeighborY++)
							{
								for (int32 NeighborX = Min.X; NeighborX <= Max.X; NeighborX++)
								{
									ShouldQuery[FVoxelUtilities::Get3DIndex<int32>(SparseSize, NeighborX, NeighborY, NeighborZ)] = true;
								}
							}
						}
					}
				}
			}
		}
	}

	if (ShouldQuery.AllEqual(false))
	{
		return {};
	}

	if (GVoxelSparseSamplerDebug &&
		Layers)
	{
		VOXEL_SCOPE_COUNTER("Debug");

		FVoxelDebugDrawer Drawer(Layers->World);

		for (int32 Z = 0; Z < SparseSize.Z; Z++)
		{
			for (int32 Y = 0; Y < SparseSize.Y; Y++)
			{
				for (int32 X = 0; X < SparseSize.X; X++)
				{
					Drawer.Color(INLINE_LAMBDA
					{
						if (ShouldQuery[FVoxelUtilities::Get3DIndex<int32>(SparseSize, X, Y, Z)])
						{
							switch (LOD)
							{
							case 0: return FLinearColor(1.0f, 0.0f, 0.0f);
							case 1: return FLinearColor(1.0f, 0.3f, 0.0f);
							case 2: return FLinearColor(1.0f, 0.5f, 0.0f);
							case 3: return FLinearColor(1.0f, 0.7f, 0.0f);
							case 4: return FLinearColor(1.0f, 1.0f, 0.0f);
							case 5: return FLinearColor(0.9f, 0.9f, 0.3f);
							case 6: return FLinearColor(0.8f, 0.8f, 0.5f);
							case 7: return FLinearColor(0.7f, 0.6f, 0.4f);
							case 8: return FLinearColor(0.6f, 0.4f, 0.3f);
							case 9: return FLinearColor(0.5f, 0.3f, 0.2f);
							default: return FLinearColor(0.4f, 0.2f, 0.1f);
							}
						}
						else
						{
							switch (LOD)
							{
							case 0: return FLinearColor(0.0f, 1.0f, 0.0f);
							case 1: return FLinearColor(0.0f, 0.9f, 0.3f);
							case 2: return FLinearColor(0.0f, 0.8f, 0.5f);
							case 3: return FLinearColor(0.0f, 0.7f, 0.7f);
							case 4: return FLinearColor(0.0f, 0.6f, 0.9f);
							case 5: return FLinearColor(0.2f, 0.5f, 1.0f);
							case 6: return FLinearColor(0.3f, 0.4f, 0.9f);
							case 7: return FLinearColor(0.4f, 0.3f, 0.8f);
							case 8: return FLinearColor(0.5f, 0.2f, 0.7f);
							case 9: return FLinearColor(0.4f, 0.1f, 0.6f);
							default: return FLinearColor(0.3f, 0.0f, 0.5f);
							}
						}
					});

					const FVector Center = SparseStart + FVector(X, Y, Z) * CellSize * SparseMultiplier;

					Drawer.DrawBox(FVoxelBox(Center).Extend(CellSize * SparseMultiplier / 2), FTransform::Identity);
					Drawer.DrawPoint(Center);
				}
			}
		}
	}

	const TVoxelArray<FVoxelIntBox> AllSparseBounds = ShouldQuery.View().GreedyMeshing3D(SparseSize);

	TVoxelArray<FVoxelIntBox> Result;
	Result.Reserve(AllSparseBounds.Num());

	for (const FVoxelIntBox& SparseBounds : AllSparseBounds)
	{
		const FVoxelIntBox BoundsToCompute = SparseBounds.ShiftBy(SparseVolumeBounds.Min).Scale(SparseMultiplier).IntersectWith(VolumeBounds);
		if (!BoundsToCompute.IsValid())
		{
			// Padding only
			continue;
		}

		for (const FVoxelIntBox& Other : Result)
		{
			ensureVoxelSlow(!Other.Intersects(BoundsToCompute));
		}

		Result.Add_EnsureNoGrow(BoundsToCompute);
	}

	return Result;
}
