// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGVoxelElevationIsolines.h"

#include "VoxelQuery.h"
#include "VoxelLayer.h"
#include "VoxelLayers.h"
#include "VoxelPCGUtilities.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Utilities/VoxelBufferGradientUtilities.h"

#include "PCGComponent.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSplineData.h"
#include "Data/PCGSpatialData.h"
#include "Components/SplineComponent.h"
#include "SpatialAlgo/PCGMarchingSquares.h"

FString UPCGVoxelElevationIsolines::GetAdditionalTitleInformation() const
{
	FString Title = "2D";
	if (Layer.Layer)
	{
		Title += " (" + Layer.Layer->GetName() + ")";
	}

	return Title; 
}

TArray<FPCGPinProperties> UPCGVoxelElevationIsolines::InputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	// Only one connection allowed, user can union multiple shapes
	Properties.Emplace(
		"Bounding Shape",
		EPCGDataType::Spatial,
		false,
		false);

	Properties.Emplace(
		"Dependency",
		EPCGDataType::Any,
		true,
		true);

	return Properties;
}

TArray<FPCGPinProperties> UPCGVoxelElevationIsolines::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;
	Properties.Emplace(PCGPinConstants::DefaultOutputLabel, bOutputAsSpline ? EPCGDataType::Spline : EPCGDataType::Point);
	return Properties;
}

TSharedPtr<FVoxelPCGOutput> UPCGVoxelElevationIsolines::CreateOutput(FPCGContext& Context) const
{
	VOXEL_FUNCTION_COUNTER();

	const UPCGComponent* Component = GetPCGComponent(Context);
	if (!Component)
	{
		return {};
	}

	if (!Layer.IsValid())
	{
		VOXEL_MESSAGE(Error, "Invalid layer");
		return {};
	}

	if (FMath::IsNearlyZero(ElevationIncrement) ||
		ElevationEnd < ElevationStart)
	{
		return {};
	}

	const FVoxelBox Bounds = INLINE_LAMBDA -> FVoxelBox
	{
		FBox Result;
		{
			bool bOutUnionWasCreated = false;
			const UPCGSpatialData* BoundingShape = Context.InputData.GetSpatialUnionOfInputsByPin(&Context, "Bounding Shape", bOutUnionWasCreated);

			// Fallback to getting bounds from actor
			if (!BoundingShape)
			{
				ensure(!bOutUnionWasCreated);
				BoundingShape = Cast<UPCGSpatialData>(Component->GetActorPCGData());
			}

			if (!ensure(BoundingShape))
			{
				return {};
			}

			Result = BoundingShape->GetBounds();
		}

		if (!ensureVoxelSlow(Result.IsValid))
		{
			return {};
		}

		// Intersect with actor bounds
		{
			const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Component->GetActorPCGData());
			if (!ensure(SpatialData))
			{
				return {};
			}

			const FBox ActorBounds = SpatialData->GetBounds();
			if (!ensure(ActorBounds.IsValid))
			{
				return {};
			}

			if (!Result.Intersect(ActorBounds))
			{
				// Nothing to process
				return {};
			}

			Result = Result.Overlap(ActorBounds);
		}

		if (!ensure(Result.IsValid))
		{
			return {};
		}

		return FVoxelBox(Result);
	};

	if (!Bounds.IsValidAndNotEmpty())
	{
		return {};
	}

	return MakeShared<FVoxelElevationIsolinesPCGOutput>(
		FVoxelLayers::Get(Component->GetWorld()),
		FVoxelSurfaceTypeTable::Get(),
		Bounds,
		FVoxelWeakStackHeightLayer(Layer),
		LOD,
		ElevationStart,
		ElevationEnd,
		ElevationIncrement,
		Resolution,
		bAddTagOnOutputForSameElevation,
		bProjectSurfaceNormal,
		bOutputAsSpline,
		bLinearSpline);
}

FString UPCGVoxelElevationIsolines::GetNodeDebugInfo() const
{
	return Super::GetNodeDebugInfo() + " [Layer: " + FString(Layer.Layer ? Layer.Layer->GetName() : "None") + "]";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFuture FVoxelElevationIsolinesPCGOutput::Run() const
{
	return Voxel::AsyncTask([this]
	{
		if (WeakLayer.Type == EVoxelLayerType::Height)
		{
			return Generate2D();
		}
		else
		{
			return Generate3D();
		}
	});
}

FVoxelFuture FVoxelElevationIsolinesPCGOutput::Generate2D() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Layers->HasLayer(WeakLayer, GetDependencyCollector()))
	{
		return {};
	}

	const FIntPoint CellMin = FVoxelUtilities::CeilToInt(FVector2D(Bounds.Min) / Resolution);
	const FIntPoint CellMax = FVoxelUtilities::FloorToInt(FVector2D(Bounds.Max) / Resolution);

	if (CellMin.X > CellMax.X ||
		CellMin.Y > CellMax.Y)
	{
		VOXEL_MESSAGE(Error, "Invalid resolution: {0}", Resolution);
		return {};
	}

	if (FInt64Point(CellMax - CellMin + 1).SizeSquared() >= MAX_int32)
	{
		VOXEL_MESSAGE(Error, "Invalid cell size: overflow: {0}", Resolution);
		return {};
	}

	const int32 CellCount = (CellMax - CellMin + 1).SizeSquared();

	FVoxelDoubleVector2DBuffer Positions;
	Positions.Allocate(CellCount);

	int32 NumSamples = 0;
	for (int32 IndexX = CellMin.X; IndexX <= CellMax.X; IndexX++)
	{
		for (int32 IndexY = CellMin.Y; IndexY <= CellMax.Y; IndexY++)
		{
			const double CurrentX = IndexX * Resolution;
			const double CurrentY = IndexY * Resolution;

			Positions.Set(NumSamples++, FVector2D(CurrentX, CurrentY));
		}
	}

	Positions.ShrinkTo(NumSamples);

	const FVoxelQuery Query(
		LOD,
		*Layers,
		*SurfaceTypeTable,
		GetDependencyCollector());

	FVoxelFloatBuffer Heights = Query.SampleHeightLayer(WeakLayer, Positions);

	if (Voxel::ShouldCancel())
	{
		return {};
	}

	FVoxelVectorBuffer PointNormals;
	if (bProjectSurfaceNormal)
	{
		VOXEL_SCOPE_COUNTER("Compute Gradient");

		FVoxelDoubleVector2DBuffer GradientPositions = FVoxelBufferGradientUtilities::SplitPositions2D(Positions, Resolution);

		const FVoxelFloatBuffer GradientHeights = Query.SampleHeightLayer(WeakLayer, GradientPositions);

		PointNormals = FVoxelBufferGradientUtilities::CollapseGradient2DToNormal(GradientHeights, NumSamples, Resolution);
	}

	const int32 NumXCells = CellMax.X - CellMin.X + 1;
	const int32 NumYCells = CellMax.Y - CellMin.Y + 1;

	struct FIsoline
	{
		TVoxelArray<FPCGPoint> Points;
		TVoxelArray<FSplinePoint> SplinePoints;
		float Elevation = 0.f;
		bool bClosed = false;
		int32 ElevationIndex = 0;
	};
	TVoxelArray<FIsoline> Isolines;
	Isolines.Reserve(NumSamples);

	int32 ElevationIndex = 0;
	for (double Elevation = ElevationStart; Elevation < ElevationEnd; Elevation += ElevationIncrement)
	{
		const TArray<PCGSpatialAlgo::FPCGMarchingSquareResult> MarchingSquareResults =
			PCGSpatialAlgo::MarchingSquares(
				NumXCells,
				NumYCells,
				Elevation,
				[&](const int32 X, const int32 Y) -> double
				{
					const int32 Index = X * NumYCells + Y;
					return Heights[Index];
				},
				true);

		for (const PCGSpatialAlgo::FPCGMarchingSquareResult& Result : MarchingSquareResults)
		{
			// We can't do anything with less than 2 points.
			if (Result.LinkedGridCoordinates.Num() < 2)
			{
				continue;
			}

			const double Threshold = Resolution * Resolution * 0.25;
			auto MergePointsIfTooClose = [Threshold](FTransform& PreviousTransform, const FTransform& CurrentTransform) -> bool
			{
				if (FVector::DistSquared(PreviousTransform.GetLocation(), CurrentTransform.GetLocation()) >= Threshold)
				{
					return false;
				}

				const FQuat LerpQuat = FQuat::Slerp(PreviousTransform.GetRotation(), CurrentTransform.GetRotation(), 0.5);
				PreviousTransform.SetRotation(LerpQuat);
				PreviousTransform.LerpTranslationScale3D(PreviousTransform, CurrentTransform, ScalarRegister(0.5));
				return true;
			};

			TVoxelArray<FTransform> FinalTransforms;
			FinalTransforms.Reserve(Result.LinkedGridCoordinates.Num());

			for (int32 Index = 0; Index < Result.LinkedGridCoordinates.Num(); ++Index)
			{
				const FVector2D GridCoordinates = Result.LinkedGridCoordinates[Index];
				FTransform Transform(FVector(
					Bounds.Min.X + Resolution * GridCoordinates.X,
					Bounds.Min.Y + Resolution * GridCoordinates.Y,
					Elevation));

				Transform.SetRotation(INLINE_LAMBDA
				{
					if (!bProjectSurfaceNormal)
					{
						return FQuat::Identity;
					}

					const int32 Index1 = FMath::FloorToInt(GridCoordinates.X) * NumYCells + FMath::FloorToInt(GridCoordinates.Y);
					const int32 Index2 = FMath::CeilToInt(GridCoordinates.X) * NumYCells + FMath::CeilToInt(GridCoordinates.Y);

					const FQuat Rotation = FRotationMatrix::MakeFromZ(FVector(PointNormals[Index1])).ToQuat();
					if (Index1 == Index2)
					{
						return Rotation;
					}

					const double RatioX = FMath::Frac(GridCoordinates.X);
					const double RatioY = FMath::Frac(GridCoordinates.Y);
					const double Ratio = FMath::IsNearlyZero(RatioX) ? RatioY : RatioX;

					return FQuat::Slerp(
						Rotation,
						FRotationMatrix::MakeFromZ(FVector(PointNormals[Index2])).ToQuat(),
						Ratio);
				});

				if (!FinalTransforms.IsEmpty())
				{
					FTransform& PreviousTransform = FinalTransforms.Last();
					const bool bHasMergedWithPrevious = MergePointsIfTooClose(PreviousTransform, Transform);
					bool bSkipPoint = bHasMergedWithPrevious;

					if (Result.bClosed &&
						Index == Result.LinkedGridCoordinates.Num() - 1)
					{
						const FTransform& CurrentTransform = bHasMergedWithPrevious ? PreviousTransform : Transform;
						if (MergePointsIfTooClose(FinalTransforms[0], CurrentTransform))
						{
							bSkipPoint = true;
							if (bHasMergedWithPrevious)
							{
								FinalTransforms.Pop();
							}
						}
					}

					if (bSkipPoint)
					{
						continue;
					}
				}

				FinalTransforms.Add(MoveTemp(Transform));
			}

			if (FinalTransforms.Num() < 2)
			{
				continue;
			}

			FIsoline Isoline;

			if (bOutputAsSpline)
			{
				FVoxelUtilities::SetNumFast(Isoline.SplinePoints, FinalTransforms.Num());
				for (int32 Index = 0; Index < FinalTransforms.Num(); Index++)
				{
					const FTransform& Transform = FinalTransforms[Index];

					// Control points will be locally at 0 in Z.
					// It is to prevent any approximation errors while interpolating on the spline.
					// The spline itself will be a Z = Elevation.
					FVector Location = Transform.GetLocation();
					Location.Z = 0;

					Isoline.SplinePoints[Index] = FSplinePoint(
						float(Index),
						Location,
						FVector::ZeroVector,
						FVector::ZeroVector,
						Transform.GetRotation().Rotator(),
						Transform.GetScale3D(),
						bLinearSpline ? ESplinePointType::Linear : ESplinePointType::Curve);
				}
			}
			else
			{
				const FBox CellBounds(
					FVector(-Resolution / 2.f, -Resolution / 2.f, -0.5f),
					FVector(Resolution / 2.f, Resolution / 2.f, 0.5f));

				FVoxelUtilities::SetNum(Isoline.Points, FinalTransforms.Num());
				for (int32 Index = 0; Index < FinalTransforms.Num(); Index++)
				{
					Isoline.Points[Index].Transform = FinalTransforms[Index];
					Isoline.Points[Index].SetLocalBounds(CellBounds);
				}
			}

			Isoline.bClosed = Result.bClosed;
			Isoline.Elevation = Elevation;
			Isoline.ElevationIndex = ElevationIndex;
			Isolines.Add(MoveTemp(Isoline));
		}

		ElevationIndex++;
	}

	return Voxel::GameTask([
		this,
		Isolines = MakeSharedCopy(MoveTemp(Isolines))]
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		if (!GetOwner())
		{
			return;
		}

		for (FIsoline& Isoline : *Isolines)
		{
			FPCGTaggedData& OutputData = GetOwner()->OutputData.TaggedData.Emplace_GetRef();
			OutputData.Pin = PCGPinConstants::DefaultOutputLabel;

			if (bAddTagOnOutputForSameElevation)
			{
				OutputData.Tags.Add(FString::Printf(TEXT("ElevationIdx_%d"), Isoline.ElevationIndex));
			}

			if (bOutputAsSpline)
			{
				UPCGSplineData* OutSplineData = FPCGContext::NewObject_AnyThread<UPCGSplineData>(GetOwner());
				OutSplineData->Initialize(Isoline.SplinePoints, Isoline.bClosed, FTransform(FVector::UpVector * Isoline.Elevation));
				OutputData.Data = OutSplineData;
			}
			else
			{
				UPCGPointData* OutPointData = FPCGContext::NewObject_AnyThread<UPCGPointData>(GetOwner());
				OutputData.Data = OutPointData;

				FVoxelPCGUtilities::AddPointsToMetadata(*OutPointData->Metadata, Isoline.Points);

				OutPointData->GetMutablePoints() = MoveTemp(Isoline.Points);
			}
		}
	});
}

FVoxelFuture FVoxelElevationIsolinesPCGOutput::Generate3D() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Layers->HasLayer(WeakLayer, GetDependencyCollector()))
	{
		return {};
	}

	return {};
}