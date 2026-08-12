// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGVoxelProjection.h"
#include "VoxelQuery.h"
#include "VoxelLayer.h"
#include "VoxelLayers.h"
#include "VoxelVersion.h"
#include "VoxelMetadata.h"
#include "VoxelPCGUtilities.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"

#include "PCGComponent.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGAsync.h"
#include "Helpers/PCGHelpers.h"

FString UPCGVoxelProjectionSettings::GetAdditionalTitleInformation() const
{
	FString Title = Layer.GetType() == EVoxelLayerType::Height ? "2D" : "3D";
	if (Layer.Layer)
	{
		Title += " (" + Layer.Layer->GetName() + ")";
	}

	return Title;
}

TArray<FPCGPinProperties> UPCGVoxelProjectionSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;

	// Only one connection allowed, user can union multiple shapes
	Properties.Emplace_GetRef(
		PCGPinConstants::DefaultInputLabel,
		EPCGDataType::Point,
		true,
		true).SetRequiredPin();

	return Properties;
}

TArray<FPCGPinProperties> UPCGVoxelProjectionSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;
	Properties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return Properties;
}

TSharedPtr<FVoxelPCGOutput> UPCGVoxelProjectionSettings::CreateOutput(FPCGContext& Context) const
{
	VOXEL_FUNCTION_COUNTER();

#if WITH_EDITOR
	UVoxelMetadata::Migrate(ConstCast(MetadatasToQuery), ConstCast(NewMetadatasToQuery));
#endif

	const UPCGComponent* Component = GetPCGComponent(Context);
	if (!ensure(Component))
	{
		return {};
	}

	if (!Layer.IsValid())
	{
		VOXEL_MESSAGE(Error, "Invalid layer");
		return {};
	}

	const TSharedRef<FVoxelProjectionPCGOutput> Output = MakeShared<FVoxelProjectionPCGOutput>(
		FVoxelLayers::Get(Component->GetWorld()),
		FVoxelSurfaceTypeTable::Get(),
		Layer,
		LOD,
		KillDistance,
		bUpdateRotation,
		bForceDirection,
		MaxSteps,
		Tolerance,
		Speed,
		GradientStep,
		bDebugSteps,
		bQuerySurfaceTypes,
		bResolveSmartSurfaceTypes,
		FVoxelMetadataRef::GetUniqueValidRefs(NewMetadatasToQuery));

	TArray<FPCGTaggedData> Sources = Context.InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context.OutputData.TaggedData;

	for (FPCGTaggedData& Source : Sources)
	{
		const UPCGPointData* ProjectionSource = Cast<UPCGPointData>(Source.Data);
		if (!ensureVoxelSlow(ProjectionSource))
		{
			continue;
		}

		UPCGPointData* ProjectionResult = CastChecked<UPCGPointData>(ProjectionSource->DuplicateData(&Context));

		FPCGTaggedData& TaggedData = Outputs.Emplace_GetRef(Source);
		TaggedData.Data = ProjectionResult;

		Output->SourceToResult.Add_EnsureNew(ProjectionSource, ProjectionResult);
	}

	return Output;
}

FString UPCGVoxelProjectionSettings::GetNodeDebugInfo() const
{
	return Super::GetNodeDebugInfo() + " [Layer: " + FString(Layer.Layer ? Layer.Layer->GetName() : "None") + "]";
}

void UPCGVoxelProjectionSettings::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.CustomVer(GVoxelCustomVersionGUID) < FVoxelVersion::AddResolveSmartSurfaceTypeToPCGQueryProjection)
	{
		bResolveSmartSurfaceTypes = false;
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FVoxelFuture FVoxelProjectionPCGOutput::Run() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	TVoxelArray<FVoxelFuture> Futures;

	for (const auto& It : SourceToResult)
	{
		const UPCGPointData* InputPoints = It.Key.Resolve();
		if (!ensure(InputPoints))
		{
			continue;
		}

		if (WeakLayer.Type == EVoxelLayerType::Height)
		{
			Futures.Add(Voxel::AsyncTask([
				this,
				Points = MakeSharedCopy(InputPoints->GetPoints()),
				WeakOutPointData = It.Value]
			{
				return Project2D(
					TVoxelArray<FPCGPoint>(MoveTemp(*Points)),
					WeakOutPointData);
			}));
		}
		else
		{
			check(WeakLayer.Type == EVoxelLayerType::Volume);

			Futures.Add(Voxel::AsyncTask([
				this,
				Points = MakeSharedCopy(InputPoints->GetPoints()),
				WeakOutPointData = It.Value]
			{
				return Project3D(
					TVoxelArray<FPCGPoint>(MoveTemp(*Points)),
					WeakOutPointData);
			}));
		}
	}

	return FVoxelFuture(Futures);
}

FVoxelFuture FVoxelProjectionPCGOutput::Project2D(
	TVoxelArray<FPCGPoint> Points,
	const TVoxelObjectPtr<UPCGPointData>& WeakOutPointData) const
{
	VOXEL_FUNCTION_COUNTER();

	bool bProcessRotations = bUpdateRotation;
	if (bQuerySurfaceTypes &&
		bResolveSmartSurfaceTypes)
	{
		bProcessRotations = true;
	}

	FVoxelVectorBuffer PointNormals;
	if (bQuerySurfaceTypes &&
		bResolveSmartSurfaceTypes)
	{
		PointNormals.Allocate(Points.Num());
	}

	if (bProcessRotations)
	{
		FVoxelDoubleVector2DBuffer Positions;
		Positions.Allocate(Points.Num() * 5);

		const float HalfGradientStep = GradientStep / 2.f;

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			const FPCGPoint& Point = Points[Index];
			const FVector Position = Point.Transform.GetLocation();

			Positions.Set(5 * Index + 0, FVector2d(Position.X - HalfGradientStep, Position.Y));
			Positions.Set(5 * Index + 1, FVector2d(Position.X + HalfGradientStep, Position.Y));
			Positions.Set(5 * Index + 2, FVector2d(Position.X, Position.Y - HalfGradientStep));
			Positions.Set(5 * Index + 3, FVector2d(Position.X, Position.Y + HalfGradientStep));
			Positions.Set(5 * Index + 4, FVector2d(Position.X, Position.Y));
		}

		const FVoxelQuery Query(
			LOD,
			*Layers,
			*SurfaceTypeTable,
			GetDependencyCollector());

		const FVoxelFloatBuffer Heights = Query.SampleHeightLayer(WeakLayer, Positions);

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			const float HeightMinX = Heights[5 * Index + 0];
			const float HeightMaxX = Heights[5 * Index + 1];
			const float HeightMinY = Heights[5 * Index + 2];
			const float HeightMaxY = Heights[5 * Index + 3];
			const float Height = Heights[5 * Index + 4];

			FPCGPoint& Point = Points[Index];

			if (FVoxelUtilities::IsNaN(Height))
			{
				Point.Density = 0.f;
				continue;
			}

			FVector Position = Point.Transform.GetLocation();
			Position.Z = Height;
			Point.Transform.SetLocation(Position);

			const FVector NewUpVector = FVector(
				(HeightMaxX - HeightMinX) / -GradientStep,
				(HeightMaxY - HeightMinY) / -GradientStep,
				1.f).GetSafeNormal();

			if (bUpdateRotation)
			{
				Point.Transform.SetRotation(FRotationMatrix::MakeFromZX(
					NewUpVector,
					Point.Transform.GetRotation().GetAxisX()).ToQuat());
			}

			if (bQuerySurfaceTypes &&
				bResolveSmartSurfaceTypes)
			{
				PointNormals.Set(Index, FVector3f(NewUpVector));
			}
		}
	}
	else
	{
		FVoxelDoubleVector2DBuffer Positions;
		Positions.Allocate(Points.Num());

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			const FPCGPoint& Point = Points[Index];
			const FVector Position = Point.Transform.GetLocation();

			Positions.Set(Index, FVector2D(Position));
		}

		const FVoxelQuery Query(
			LOD,
			*Layers,
			*SurfaceTypeTable,
			GetDependencyCollector());

		const FVoxelFloatBuffer Heights = Query.SampleHeightLayer(WeakLayer, Positions);

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			const float Height = Heights[Index];
			FPCGPoint& Point = Points[Index];

			if (FVoxelUtilities::IsNaN(Height))
			{
				Point.Density = 0.f;
				continue;
			}

			FVector LocalPosition = Point.Transform.GetLocation();
			LocalPosition.Z = Height;
			Point.Transform.SetLocation(LocalPosition);
		}
	}

	TVoxelMap<FVoxelSurfaceType, TVoxelArray<float>> SurfaceTypeToWeight;
	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;

	if (bQuerySurfaceTypes ||
		MetadatasToQuery.Num() > 0)
	{
		VOXEL_SCOPE_COUNTER("Surface type & metadata");

		FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
		if (bQuerySurfaceTypes)
		{
			SurfaceTypes.AllocateZeroed(Points.Num());
		}

		MetadataToBuffer.Reserve(MetadatasToQuery.Num());

		for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
		{
			MetadataToBuffer.Add_EnsureNew(
				MetadataToQuery,
				MetadataToQuery.MakeDefaultBuffer(Points.Num()));
		}

		const FVoxelQuery Query(
			LOD,
			*Layers,
			*SurfaceTypeTable,
			GetDependencyCollector());

		FVoxelDoubleVector2DBuffer HeightPositions;
		HeightPositions.Allocate(Points.Num());

		FVoxelDoubleVectorBuffer PointPositions;
		PointPositions.Allocate(Points.Num());

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			const FPCGPoint& Point = Points[Index];
			const FVector Position = Point.Transform.GetLocation();

			HeightPositions.Set(Index, FVector2D(Position));
			PointPositions.Set(Index, Position);
		}

		Query.SampleHeightLayer(
			WeakLayer,
			HeightPositions,
			SurfaceTypes.View(),
			MetadataToBuffer);

		if (bQuerySurfaceTypes)
		{
			if (bResolveSmartSurfaceTypes)
			{
				FVoxelSmartSurfaceTypeResolver Resolver(
					Query.LOD,
					WeakLayer,
					Query.Layers,
					Query.SurfaceTypeTable,
					Query.DependencyCollector,
					PointPositions,
					PointNormals,
					SurfaceTypes.View());

				Resolver.Resolve();
			}

			SurfaceTypeToWeight.Reserve(32);

			for (int32 Index = 0; Index < Points.Num(); Index++)
			{
				for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypes[Index].GetLayers())
				{
					if (!SurfaceTypeToWeight.Contains(Layer.Type))
					{
						SurfaceTypeToWeight.Add_CheckNew(Layer.Type).SetNumZeroed(Points.Num());
					}
					SurfaceTypeToWeight.FindChecked(Layer.Type)[Index] += Layer.Weight.ToFloat();
				}
			}
		}
	}

	return Voxel::GameTask([
		=,
		this,
		Points = MakeSharedCopy(MoveTemp(Points)),
		SurfaceTypeToWeight = MoveTemp(SurfaceTypeToWeight),
		MetadataToBuffer = MoveTemp(MetadataToBuffer)]
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		UPCGPointData* OutPointData = WeakOutPointData.Resolve();
		if (!ensureVoxelSlow(OutPointData))
		{
			return;
		}

		if (bQuerySurfaceTypes ||
			MetadatasToQuery.Num() > 0)
		{
			FVoxelPCGUtilities::AddPointsToMetadata(*OutPointData->Metadata, *Points);
		}

		if (bQuerySurfaceTypes)
		{
			for (const auto& It : SurfaceTypeToWeight)
			{
				FVoxelPCGUtilities::AddAttribute<float>(
					*OutPointData->Metadata,
					*Points,
					It.Key.GetFName(),
					MakeVoxelArrayView(It.Value).LeftOf(Points->Num()));
			}

			FVoxelSurfaceType::ForeachSurfaceType([&](const FVoxelSurfaceType& Type)
			{
				FVoxelPCGUtilities::AddDefaultAttributeIfNeeded<float>(
					*OutPointData->Metadata,
					Type.GetFName());
			});
		}

		for (const auto& It : MetadataToBuffer)
		{
			It.Key.AddToPCG(
				*OutPointData->Metadata,
				*Points,
				It.Key.GetFName(),
				*It.Value);
		}

		OutPointData->GetMutablePoints() = MoveTemp(*Points);
	});
}

FVoxelFuture FVoxelProjectionPCGOutput::Project3D(
	TVoxelArray<FPCGPoint> Points,
	const TVoxelObjectPtr<UPCGPointData>& WeakOutPointData) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelVectorBuffer PointNormals;
	PointNormals.Allocate(Points.Num());

	{
		VOXEL_SCOPE_COUNTER("Project");

		TVoxelArray<FVector> Positions;
		FVoxelUtilities::SetNumFast(Positions, Points.Num());

		{
			VOXEL_SCOPE_COUNTER("Copy positions");

			for (int32 Index = 0; Index < Points.Num(); Index++)
			{
				const FPCGPoint& Point = Points[Index];

				Positions[Index] = Point.Transform.GetLocation();
				PointNormals.Set(Index, FVector3f(Point.Transform.GetRotation().GetUpVector()));
				ensure(PointNormals[Index].IsNormalized());
			}
		}

		TVoxelArray<int32> PointsAlive;
		{
			VOXEL_SCOPE_COUNTER("Initialize PointsAlive");

			FVoxelUtilities::SetNumFast(PointsAlive, Points.Num());

			for (int32 Index = 0; Index < Points.Num(); Index++)
			{
				PointsAlive[Index] = Index;
			}
		}

		const FVoxelQuery Query(
			LOD,
			*Layers,
			*SurfaceTypeTable,
			GetDependencyCollector());

		FVoxelDoubleVectorBuffer QueryPositions;

		for (int32 Step = 0; Step < MaxSteps; Step++)
		{
			VOXEL_SCOPE_COUNTER("Iteration");

			{
				VOXEL_SCOPE_COUNTER("Set positions");

				QueryPositions.Allocate(7 * PointsAlive.Num());

				const float HalfGradientStep = GradientStep / 2.f;

				for (int32 Index = 0; Index < PointsAlive.Num(); Index++)
				{
					const FVector Position = Positions[PointsAlive[Index]];

					QueryPositions.Set(7 * Index + 0, FVector(Position.X - HalfGradientStep, Position.Y, Position.Z));
					QueryPositions.Set(7 * Index + 1, FVector(Position.X + HalfGradientStep, Position.Y, Position.Z));
					QueryPositions.Set(7 * Index + 2, FVector(Position.X, Position.Y - HalfGradientStep, Position.Z));
					QueryPositions.Set(7 * Index + 3, FVector(Position.X, Position.Y + HalfGradientStep, Position.Z));
					QueryPositions.Set(7 * Index + 4, FVector(Position.X, Position.Y, Position.Z - HalfGradientStep));
					QueryPositions.Set(7 * Index + 5, FVector(Position.X, Position.Y, Position.Z + HalfGradientStep));
					QueryPositions.Set(7 * Index + 6, FVector(Position.X, Position.Y, Position.Z));
				}
			}

			const FVoxelFloatBuffer Distances = Query.SampleVolumeLayer(WeakLayer, QueryPositions);

			for (int32 Index = 0; Index < PointsAlive.Num(); Index++)
			{
				const float DistanceMinX = Distances[7 * Index + 0];
				const float DistanceMaxX = Distances[7 * Index + 1];
				const float DistanceMinY = Distances[7 * Index + 2];
				const float DistanceMaxY = Distances[7 * Index + 3];
				const float DistanceMinZ = Distances[7 * Index + 4];
				const float DistanceMaxZ = Distances[7 * Index + 5];
				const float Distance = Distances[7 * Index + 6];
				const float AbsDistance = FMath::Abs(Distance);

				int32& PointIndex = PointsAlive[Index];
				FPCGPoint& Point = Points[PointIndex];

				if (FVoxelUtilities::IsNaN(Distance))
				{
					if (bDebugSteps)
					{
						FVoxelDebugDrawer()
							.DrawPoint(Positions[PointIndex])
							.Color(FColorList::Magenta);
					}

					Point.Density = 0.f;
					PointIndex = -1;
					continue;
				}

				if (Step == MaxSteps - 1 &&
					AbsDistance > KillDistance)
				{
					Point.Density = 0.f;
					PointIndex = -1;
					continue;
				}

				if (AbsDistance < Tolerance ||
					Step == MaxSteps - 1)
				{
					const FVector NewUpVector = FVector(
						DistanceMaxX - DistanceMinX,
						DistanceMaxY - DistanceMinY,
						DistanceMaxZ - DistanceMinZ).GetSafeNormal();

					if (bUpdateRotation)
					{
						Point.Transform.SetRotation(FRotationMatrix::MakeFromZX(
							NewUpVector,
							Point.Transform.GetRotation().GetAxisX()).ToQuat());
					}
					if (bQuerySurfaceTypes &&
						bResolveSmartSurfaceTypes)
					{
						PointNormals.Set(PointIndex, FVector3f(NewUpVector));
					}

					PointIndex = -1;
					continue;
				}

				const FVector3f Gradient =
					FVector3f(
						DistanceMaxX - DistanceMinX,
						DistanceMaxY - DistanceMinY,
						DistanceMaxZ - DistanceMinZ) / GradientStep;

				FVector3f Delta;

				if (bForceDirection)
				{
					const FVector3f Normal = PointNormals[PointIndex];
					const float LocalSpeed = FMath::Clamp(FVector3f::DotProduct(Gradient.GetSafeNormal(), Normal.GetSafeNormal()), 0.25f, 1.f);
					const float Value = LocalSpeed * Distance * Speed;

					Delta = Normal * Value;
				}
				else
				{
					Delta = Gradient.GetSafeNormal() * Distance * Speed;
				}

				Positions[PointIndex] -= FVector(Delta);

				if (bDebugSteps)
				{
					FVoxelDebugDrawer()
						.DrawPoint(Positions[PointIndex])
						.Color(FMath::Lerp(FLinearColor::Blue, FLinearColor::Red, Step / float(MaxSteps - 1)));
				}
			}

			if (Step == MaxSteps - 1)
			{
				break;
			}

			PointsAlive.RemoveAllSwap([](const int32 Index)
			{
				return Index == -1;
			});

			if (PointsAlive.Num() == 0)
			{
				break;
			}
		}

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			Points[Index].Transform.SetLocation(Positions[Index]);
		}
	}

	TVoxelMap<FVoxelSurfaceType, TVoxelArray<float>> SurfaceTypeToWeight;
	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;

	if (bQuerySurfaceTypes ||
		MetadatasToQuery.Num() > 0)
	{
		VOXEL_SCOPE_COUNTER("Surface type & metadata");

		FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
		if (bQuerySurfaceTypes)
		{
			SurfaceTypes.AllocateZeroed(Points.Num());
		}

		MetadataToBuffer.Reserve(MetadatasToQuery.Num());

		for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
		{
			MetadataToBuffer.Add_EnsureNew(
				MetadataToQuery,
				MetadataToQuery.MakeDefaultBuffer(Points.Num()));
		}

		const FVoxelQuery Query(
			LOD,
			*Layers,
			*SurfaceTypeTable,
			GetDependencyCollector());

		FVoxelDoubleVectorBuffer Positions;
		Positions.Allocate(Points.Num());

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			const FPCGPoint& Point = Points[Index];
			const FVector Position = Point.Transform.GetLocation();

			Positions.Set(Index, Position);
		}

		Query.SampleVolumeLayer(
			WeakLayer,
			Positions,
			SurfaceTypes.View(),
			MetadataToBuffer);

		if (bQuerySurfaceTypes)
		{
			if (bResolveSmartSurfaceTypes)
			{
				FVoxelSmartSurfaceTypeResolver Resolver(
					Query.LOD,
					WeakLayer,
					Query.Layers,
					Query.SurfaceTypeTable,
					Query.DependencyCollector,
					Positions,
					PointNormals,
					SurfaceTypes.View());

				Resolver.Resolve();
			}

			SurfaceTypeToWeight.Reserve(32);

			for (int32 Index = 0; Index < Points.Num(); Index++)
			{
				for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypes[Index].GetLayers())
				{
					if (!SurfaceTypeToWeight.Contains(Layer.Type))
					{
						SurfaceTypeToWeight.Add_CheckNew(Layer.Type).SetNumZeroed(Points.Num());
					}
					SurfaceTypeToWeight.FindChecked(Layer.Type)[Index] += Layer.Weight.ToFloat();
				}
			}
		}
	}

	return Voxel::GameTask([
		=,
		this,
		Points = MakeSharedCopy(MoveTemp(Points)),
		SurfaceTypeToWeight = MoveTemp(SurfaceTypeToWeight),
		MetadataToBuffer = MoveTemp(MetadataToBuffer)]
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		UPCGPointData* OutPointData = WeakOutPointData.Resolve();
		if (!ensureVoxelSlow(OutPointData))
		{
			return;
		}

		if (bQuerySurfaceTypes ||
			MetadataToBuffer.Num() > 0)
		{
			FVoxelPCGUtilities::AddPointsToMetadata(*OutPointData->Metadata, *Points);
		}

		if (bQuerySurfaceTypes)
		{
			for (const auto& It : SurfaceTypeToWeight)
			{
				FVoxelPCGUtilities::AddAttribute<float>(
					*OutPointData->Metadata,
					*Points,
					It.Key.GetFName(),
					MakeVoxelArrayView(It.Value).LeftOf(Points->Num()));
			}

			FVoxelSurfaceType::ForeachSurfaceType([&](const FVoxelSurfaceType& Type)
			{
				FVoxelPCGUtilities::AddDefaultAttributeIfNeeded<float>(
					*OutPointData->Metadata,
					Type.GetFName());
			});
		}

		for (const auto& It : MetadataToBuffer)
		{
			It.Key.AddToPCG(
				*OutPointData->Metadata,
				*Points,
				It.Key.GetFName(),
				*It.Value);
		}

		OutPointData->GetMutablePoints() = MoveTemp(*Points);
	});
}