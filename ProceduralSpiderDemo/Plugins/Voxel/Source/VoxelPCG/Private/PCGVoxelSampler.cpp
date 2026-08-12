// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGVoxelSampler.h"
#include "VoxelPCGUtilities.h"
#include "VoxelPointGenerator.h"
#include "VoxelQuery.h"
#include "VoxelLayer.h"
#include "VoxelLayers.h"
#include "VoxelMetadata.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "Utilities/VoxelBufferGradientUtilities.h"

#include "PCGComponent.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGAsync.h"
#include "Helpers/PCGHelpers.h"

FString UPCGVoxelSamplerSettings::GetAdditionalTitleInformation() const
{
	FString Title = Layer.GetType() == EVoxelLayerType::Height ? "2D" : "3D";
	if (Layer.Layer)
	{
		Title += " (" + Layer.Layer->GetName() + ")";
	}

	return Title; 
}

TArray<FPCGPinProperties> UPCGVoxelSamplerSettings::InputPinProperties() const
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

TArray<FPCGPinProperties> UPCGVoxelSamplerSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;
	Properties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return Properties;
}

TSharedPtr<FVoxelPCGOutput> UPCGVoxelSamplerSettings::CreateOutput(FPCGContext& Context) const
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

	const FVoxelBox Bounds = INLINE_LAMBDA -> FVoxelBox
	{
		if (bUnbounded &&
			Context.InputData.GetInputCountByPin("Bounding Shape") == 0)
		{
			return FVoxelBox::Infinite;
		}

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

#if WITH_EDITOR
	UVoxelMetadata::Migrate(ConstCast(MetadatasToQuery), ConstCast(NewMetadatasToQuery));
#endif

	UPCGPointData* PointData = NewObject<UPCGPointData>();
	Context.OutputData.TaggedData.Emplace_GetRef().Data = PointData;

	return MakeShared<FVoxelSamplerPCGOutput>(
		FVoxelLayers::Get(Component->GetWorld()),
		FVoxelSurfaceTypeTable::Get(),
		Bounds,
		Layer,
		LOD,
		bResolveSmartSurfaceTypes,
		PointsPerSquaredMeter,
		CellSize,
		Looseness,
		Tolerance,
		bApplyDensityToPoints,
		PointData,
		FVoxelMetadataRef::GetUniqueValidRefs(NewMetadatasToQuery));
}

FString UPCGVoxelSamplerSettings::GetNodeDebugInfo() const
{
	return Super::GetNodeDebugInfo() + " [Layer: " + FString(Layer.Layer ? Layer.Layer->GetName() : "None") + "]";
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FVoxelFuture FVoxelSamplerPCGOutput::Run() const
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

FVoxelFuture FVoxelSamplerPCGOutput::Generate2D() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Layers->HasLayer(WeakLayer, GetDependencyCollector()))
	{
		return {};
	}

	const FIntPoint CellMin = FVoxelUtilities::CeilToInt(FVector2D(Bounds.Min) / CellSize);
	const FIntPoint CellMax = FVoxelUtilities::FloorToInt(FVector2D(Bounds.Max) / CellSize);

	if (CellMin.X > CellMax.X ||
		CellMin.Y > CellMax.Y)
	{
		VOXEL_MESSAGE(Error, "Invalid cell size: {0}", CellSize);
		return {};
	}

	if (FInt64Point(CellMax - CellMin + 1).SizeSquared() >= MAX_int32)
	{
		VOXEL_MESSAGE(Error, "Invalid cell size: overflow: {0}", CellSize);
		return {};
	}

	const int32 CellCount = (CellMax - CellMin + 1).SizeSquared();

	const double TargetPointCount = FVector2D(Bounds.Max - Bounds.Min).SizeSquared() * PointsPerSquaredMeter / (100.f * 100.f);
	const float Ratio = float(FMath::Clamp(TargetPointCount / double(CellCount), 0, 1));

	if (Ratio < UE_SMALL_NUMBER)
	{
		VOXEL_MESSAGE(Warning, "Ratio too small, skipping: {0}", Ratio);
		return {};

	}

	if (PCGFeatureSwitches::CVarCheckSamplerMemory.GetValueOnAnyThread() &&
		PCGFeatureSwitches::CVarSamplerMemoryThreshold.GetValueOnAnyThread() * FPlatformMemory::GetStats().AvailablePhysical < sizeof(FPCGPoint) * CellCount)
	{
		VOXEL_MESSAGE(Warning, "Tried to generate too many points: {0}", CellCount);
		return {};

	}

	// Drop points slightly by an epsilon otherwise point can be culled. If the sampler has a volume connected as the Bounding Shape,
	// the volume will call through to PCGHelpers::IsInsideBounds() which is a one sided test and points at the top of the volume
	// will fail it. TODO perhaps the one-sided check can be isolated to component-bounds
	constexpr double ZMultiplier = 1.0 - UE_DOUBLE_SMALL_NUMBER;
	// Try to use a multiplier instead of a simply offset to combat loss of precision in floats. However if MaxZ is very small,
	// then multiplier will not work, so just use an offset.
	double SampleZ = (FMath::Abs(Bounds.Max.Z) > UE_DOUBLE_SMALL_NUMBER) ? Bounds.Max.Z * ZMultiplier : -UE_DOUBLE_SMALL_NUMBER;
	// Make sure we're still in bounds though!
	SampleZ = FMath::Max(SampleZ, Bounds.Min.Z);

	FVoxelDoubleVector2DBuffer Positions;
	Positions.Allocate(CellCount);

	TVoxelArray<float> DensityMultipliers;
	TVoxelArray<int32> Seeds;
	DensityMultipliers.Reserve(CellCount);
	Seeds.Reserve(CellCount);

	int32 NumSamples = 0;
	for (int32 IndexX = CellMin.X; IndexX <= CellMax.X; IndexX++)
	{
		for (int32 IndexY = CellMin.Y; IndexY <= CellMax.Y; IndexY++)
		{
			const double CurrentX = IndexX * CellSize;
			const double CurrentY = IndexY * CellSize;

			FRandomStream RandomSource(PCGHelpers::ComputeSeed(GetSeed(), IndexX, IndexY));
			const float Chance = RandomSource.FRand();

			if (Chance >= Ratio)
			{
				continue;
			}

			const float RandX = RandomSource.FRand();
			const float RandY = RandomSource.FRand();

			const FVector2D Position
			{
				CurrentX + RandX * Looseness * CellSize,
				CurrentY + RandY * Looseness * CellSize
			};

			Positions.Set(NumSamples++, Position);

			DensityMultipliers.Add(bApplyDensityToPoints ? (Ratio - Chance) / Ratio : 1.0f);
			Seeds.Add(RandomSource.GetCurrentSeed());
		}
	}

	Positions.ShrinkTo(NumSamples);

	check(NumSamples == DensityMultipliers.Num());
	check(NumSamples == Seeds.Num());

	FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
	SurfaceTypes.AllocateZeroed(Positions.Num());

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
	MetadataToBuffer.Reserve(MetadatasToQuery.Num());

	for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
	{
		MetadataToBuffer.Add_EnsureNew(
			MetadataToQuery,
			MetadataToQuery.MakeDefaultBuffer(Positions.Num()));
	}

	const FVoxelQuery Query(
		LOD,
		*Layers,
		*SurfaceTypeTable,
		GetDependencyCollector());

	const FVoxelFloatBuffer Heights = Query.SampleHeightLayer(
		WeakLayer,
		Positions,
		SurfaceTypes.View(),
		MetadataToBuffer);

	if (Voxel::ShouldCancel())
	{
		return {};
	}

	FVoxelDoubleVectorBuffer PointPositions;
	PointPositions.Allocate(NumSamples);

	for (int32 Index = 0; Index < NumSamples; Index++)
	{
		PointPositions.Set(Index, FVector(Positions.X[Index], Positions.Y[Index], Heights[Index]));
	}

	FVoxelVectorBuffer PointNormals;
	{
		VOXEL_SCOPE_COUNTER("Compute Gradient");

		// TODO?
		const float GradientStep = 100.f;

		FVoxelDoubleVector2DBuffer GradientPositions = FVoxelBufferGradientUtilities::SplitPositions2D(Positions, GradientStep);

		const FVoxelFloatBuffer GradientHeights = Query.SampleHeightLayer(WeakLayer, GradientPositions);

		PointNormals = FVoxelBufferGradientUtilities::CollapseGradient2DToNormal(GradientHeights, NumSamples, GradientStep);
	}

	FVoxelSmartSurfaceTypeResolver Resolver(
		0,
		WeakLayer,
		*Layers,
		*SurfaceTypeTable,
		GetDependencyCollector(),
		PointPositions,
		PointNormals,
		SurfaceTypes.View());

	Resolver.Resolve();

	TVoxelMap<FVoxelSurfaceType, TVoxelArray<float>> SurfaceTypeToWeight;
	SurfaceTypeToWeight.Reserve(32);

	TVoxelArray<int32> Indirection;
	Indirection.Reserve(NumSamples);

	TVoxelArray<FPCGPoint> Points;
	Points.Reserve(NumSamples);

	for (int32 ReadIndex = 0; ReadIndex < NumSamples; ReadIndex++)
	{
		const float Height = Heights[ReadIndex];

		if (FVoxelUtilities::IsNaN(Height) ||
			Height < Bounds.Min.Z ||
			Height > Bounds.Max.Z)
		{
			continue;
		}

		if (PointNormals[ReadIndex].ContainsNaN())
		{
			continue;
		}

		const int32 WriteIndex = Points.Emplace();
		FPCGPoint& Point = Points[WriteIndex];
		Indirection.Add(ReadIndex);

		for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypes[ReadIndex].GetLayers())
		{
			if (!SurfaceTypeToWeight.Contains(Layer.Type))
			{
				SurfaceTypeToWeight.Add_CheckNew(Layer.Type).SetNumZeroed(NumSamples);
			}
			SurfaceTypeToWeight.FindChecked(Layer.Type)[WriteIndex] += Layer.Weight.ToFloat();
		}

		Point.Transform = FTransform(FRotationMatrix::MakeFromZ(FVector(PointNormals[ReadIndex])).ToQuat(),

			FVector(
				Positions[ReadIndex],
				Height));

		Point.Density = DensityMultipliers[ReadIndex];
		Point.Seed = Seeds[ReadIndex];
		Point.MetadataEntry = WriteIndex;

		Point.SetLocalBounds(FBox(FVector(-CellSize), FVector(CellSize)));
	}

	return Voxel::GameTask([
		=,
		this,
		MetadataToBuffer = MoveTemp(MetadataToBuffer),
		Points = MakeSharedCopy(MoveTemp(Points)),
		SurfaceTypeToWeight = MoveTemp(SurfaceTypeToWeight),
		Indirection = MoveTemp(Indirection)]
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		UPCGPointData* PointData = WeakPointData.Resolve();
		if (!ensure(PointData))
		{
			return;
		}

		FVoxelPCGUtilities::AddPointsToMetadata(*PointData->Metadata, *Points);

		for (const auto& It : SurfaceTypeToWeight)
		{
			FVoxelPCGUtilities::AddAttribute<float>(
				*PointData->Metadata,
				*Points,
				It.Key.GetFName(),
				MakeVoxelArrayView(It.Value).LeftOf(Points->Num()));
		}

		FVoxelSurfaceType::ForeachSurfaceType([&](const FVoxelSurfaceType& Type)
		{
			FVoxelPCGUtilities::AddDefaultAttributeIfNeeded<float>(
				*PointData->Metadata,
				Type.GetFName());
		});

		for (const FVoxelMetadataRef MetadataToQuery : MetadatasToQuery)
		{
			MetadataToQuery.AddToPCG(
				*PointData->Metadata,
				*Points,
				MetadataToQuery.GetFName(),
				*MetadataToBuffer[MetadataToQuery]->Gather(Indirection));
		}

		PointData->GetMutablePoints() = MoveTemp(*Points);
	});
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

FVoxelFuture FVoxelSamplerPCGOutput::Generate3D() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Layers->HasLayer(WeakLayer, GetDependencyCollector()))
	{
		return {};
	}

	const FIntVector CellMin = FVoxelUtilities::CeilToInt(Bounds.Min / CellSize);
	const FIntVector CellMax = FVoxelUtilities::FloorToInt(Bounds.Max / CellSize);

	if (CellMin.X > CellMax.X ||
		CellMin.Y > CellMax.Y ||
		CellMin.Z > CellMax.Z)
	{
		VOXEL_MESSAGE(Error, "Invalid cell size: {0}", CellSize);
		return {};
	}

	if (FVoxelUtilities::SizeSquared(CellMax - CellMin + 1) >= MAX_int32)
	{
		VOXEL_MESSAGE(Error, "Invalid cell size: overflow: {0}", CellSize);
		return {};
	}

	const int32 CellCount = FVoxelUtilities::SizeSquared(CellMax - CellMin + 1);

	// Compute Ratio in 2D
	const double TargetPointCount = FVector2D(Bounds.Max - Bounds.Min).SizeSquared() * PointsPerSquaredMeter / (100.f * 100.f);
	const float Ratio = float(FMath::Clamp(TargetPointCount / double((CellMax.X - CellMin.X + 1) * (CellMax.Y - CellMin.Y + 1)), 0, 1));

	if (Ratio < UE_SMALL_NUMBER)
	{
		VOXEL_MESSAGE(Warning, "Ratio too small, skipping: {0}", Ratio);
		return {};

	}

	if (PCGFeatureSwitches::CVarCheckSamplerMemory.GetValueOnAnyThread() &&
		PCGFeatureSwitches::CVarSamplerMemoryThreshold.GetValueOnAnyThread() * FPlatformMemory::GetStats().AvailablePhysical < sizeof(FPCGPoint) * CellCount)
	{
		VOXEL_MESSAGE(Warning, "Tried to generate too many points: {0}", CellCount);
		return {};

	}

	const FVector Start = FVector(CellMin) * CellSize;
	const FIntVector Size = CellMax - CellMin + 2;

	const TSharedRef<FVoxelPointGenerator> Generator = MakeShared<FVoxelPointGenerator>(
		GetSharedDependencyCollector(),
		Layers,
		SurfaceTypeTable,
		Start,
		Size,
		CellSize,
		WeakLayer,
		LOD,
		bResolveSmartSurfaceTypes);

	return Generator->Initialize(Tolerance).Then_AsyncThread([=, this]
	{
		FVoxelSurfaceTypeBlendBuffer SurfaceTypes;

		TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
		MetadataToBuffer.Reserve(MetadatasToQuery.Num());

		const TVoxelArray<FVoxelPointGenerator::FPoint> GeneratorPoints = Generator->Generate(
			Bounds,
			GetSeed(),
			Looseness,
			Ratio,
			bApplyDensityToPoints,
			MetadatasToQuery,
			SurfaceTypes,
			MetadataToBuffer);

		TVoxelArray<FPCGPoint> Points;
		FVoxelUtilities::SetNumFast(Points, GeneratorPoints.Num());

		for (int32 Index = 0; Index < GeneratorPoints.Num(); Index++)
		{
			const FVoxelPointGenerator::FPoint& GeneratorPoint = GeneratorPoints[Index];

			FPCGPoint& Point = Points[Index];
			Point = FPCGPoint();

			Point.Transform = FTransform(
				FRotationMatrix::MakeFromZ(FVector(GeneratorPoint.Normal)).ToQuat(),
				GeneratorPoint.Position);

			Point.Density = GeneratorPoint.Density;
			Point.Seed = GeneratorPoint.Seed;
			Point.MetadataEntry = Index;

			Point.SetLocalBounds(FBox(FVector(-CellSize), FVector(CellSize)));
		}

		TVoxelMap<FVoxelSurfaceType, TVoxelArray<float>> SurfaceTypeToAlphas;
		SurfaceTypeToAlphas.Reserve(32);
		{
			VOXEL_SCOPE_COUNTER("Extract surfaces");

			for (int32 Index = 0; Index < GeneratorPoints.Num(); Index++)
			{
				for (const FVoxelSurfaceTypeBlendLayer& Layer : SurfaceTypes[Index].GetLayers())
				{
					if (!SurfaceTypeToAlphas.Contains(Layer.Type))
					{
						SurfaceTypeToAlphas.Add_CheckNew(Layer.Type).SetNumZeroed(GeneratorPoints.Num());
					}
					SurfaceTypeToAlphas.FindChecked(Layer.Type)[Index] += Layer.Weight.ToFloat();
				}
			}
		}

		return Voxel::GameTask([
			=,
			this,
			Points = MakeSharedCopy(MoveTemp(Points)),
			SurfaceTypeToAlphas = MoveTemp(SurfaceTypeToAlphas),
			MetadataToBuffer = MoveTemp(MetadataToBuffer)]
		{
			VOXEL_FUNCTION_COUNTER();
			check(IsInGameThread());

			UPCGPointData* PointData = WeakPointData.Resolve();
			if (!ensure(PointData))
			{
				return;
			}

			FVoxelPCGUtilities::AddPointsToMetadata(*PointData->Metadata, *Points);

			for (const auto& It : SurfaceTypeToAlphas)
			{
				FVoxelPCGUtilities::AddAttribute<float>(
					*PointData->Metadata,
					*Points,
					It.Key.GetFName(),
					MakeVoxelArrayView(It.Value).LeftOf(Points->Num()));
			}

			FVoxelSurfaceType::ForeachSurfaceType([&](const FVoxelSurfaceType& Type)
			{
				FVoxelPCGUtilities::AddDefaultAttributeIfNeeded<float>(
					*PointData->Metadata,
					Type.GetFName());
			});

			for (const FVoxelMetadataRef MetadataToQuery : MetadatasToQuery)
			{
				MetadataToQuery.AddToPCG(
					*PointData->Metadata,
					*Points,
					MetadataToQuery.GetFName(),
					*MetadataToBuffer[MetadataToQuery]);
			}

			PointData->GetMutablePoints() = MoveTemp(*Points);
		});
	});
}