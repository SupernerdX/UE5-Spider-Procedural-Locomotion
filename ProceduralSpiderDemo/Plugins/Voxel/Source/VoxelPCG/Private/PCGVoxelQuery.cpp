// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGVoxelQuery.h"
#include "VoxelQuery.h"
#include "VoxelLayer.h"
#include "VoxelLayers.h"
#include "VoxelVersion.h"
#include "VoxelMetadata.h"
#include "VoxelPCGUtilities.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"
#include "Surface/VoxelSmartSurfaceTypeResolver.h"
#include "Utilities/VoxelBufferGradientUtilities.h"

#include "PCGComponent.h"
#include "Data/PCGPointData.h"

FString UPCGVoxelQuerySettings::GetAdditionalTitleInformation() const
{
	FString Title = Layer.GetType() == EVoxelLayerType::Height ? "2D" : "3D";
	if (Layer.Layer)
	{
		Title += " (" + Layer.Layer->GetName() + ")";
	}

	return Title; 
}

TArray<FPCGPinProperties> UPCGVoxelQuerySettings::InputPinProperties() const
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

TArray<FPCGPinProperties> UPCGVoxelQuerySettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;
	Properties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return Properties;
}

TSharedPtr<FVoxelPCGOutput> UPCGVoxelQuerySettings::CreateOutput(FPCGContext& Context) const
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

	const TSharedRef<FVoxelQueryPCGOutput> Output = MakeShared<FVoxelQueryPCGOutput>(
		FVoxelLayers::Get(Component->GetWorld()),
		FVoxelSurfaceTypeTable::Get(),
		Layer,
		LOD,
		HeightOrDistanceAttribute,
		bQuerySurfaceTypes,
		bResolveSmartSurfaceTypes,
		SurfaceAttributeSuffix,
		FVoxelMetadataRef::GetUniqueValidRefs(NewMetadatasToQuery),
		MetadataAttributeSuffix);

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

FString UPCGVoxelQuerySettings::GetNodeDebugInfo() const
{
	return Super::GetNodeDebugInfo() + " [Layer: " + FString(Layer.Layer ? Layer.Layer->GetName() : "None") + "]";
}

void UPCGVoxelQuerySettings::Serialize(FArchive& Ar)
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

FVoxelFuture FVoxelQueryPCGOutput::Run() const
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

		Futures.Add(Voxel::AsyncTask([
			this,
			Points = MakeSharedCopy(InputPoints->GetPoints()),
			WeakOutPointData = It.Value]
		{
			return Query(
				TVoxelArray<FPCGPoint>(MoveTemp(*Points)),
				WeakOutPointData);
		}));
	}

	return FVoxelFuture(Futures);
}

FVoxelFuture FVoxelQueryPCGOutput::Query(
	TVoxelArray<FPCGPoint> Points,
	const TVoxelObjectPtr<UPCGPointData>& WeakOutPointData) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDoubleVectorBuffer Positions;
	Positions.Allocate(Points.Num());

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		Positions.Set(Index, Points[Index].Transform.GetLocation());
	}

	FVoxelSurfaceTypeBlendBuffer SurfaceTypes;
	if (bQuerySurfaceTypes)
	{
		SurfaceTypes.AllocateZeroed(Positions.Num());
	}

	TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>> MetadataToBuffer;
	MetadataToBuffer.Reserve(MetadatasToQuery.Num());

	for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
	{
		MetadataToBuffer.Add_EnsureNew(
			MetadataToQuery,
			MetadataToQuery.MakeDefaultBuffer(Positions.Num()));
	}

	TVoxelMap<FVoxelSurfaceType, TVoxelArray<float>> SurfaceTypeToWeight;
	SurfaceTypeToWeight.Reserve(32);

	const FVoxelQuery Query(
		LOD,
		*Layers,
		*SurfaceTypeTable,
		GetDependencyCollector());

	FVoxelFloatBuffer Values;
	if (WeakLayer.Type == EVoxelLayerType::Height)
	{
		FVoxelDoubleVector2DBuffer Positions2D;
		Positions2D.X = Positions.X;
		Positions2D.Y = Positions.Y;

		Values = Query.SampleHeightLayer(
			WeakLayer,
			Positions2D,
			SurfaceTypes.View(),
			MetadataToBuffer);
	}
	else
	{
		Values = Query.SampleVolumeLayer(
			WeakLayer,
			Positions,
			SurfaceTypes.View(),
			MetadataToBuffer);
	}

	if (bQuerySurfaceTypes)
	{
		if (bResolveSmartSurfaceTypes)
		{
			FVoxelVectorBuffer PointNormals;
			if (WeakLayer.Type == EVoxelLayerType::Height)
			{
				const float GradientStep = 100.f;

				FVoxelDoubleVector2DBuffer GradientPositions = FVoxelBufferGradientUtilities::SplitPositions2D(Positions, GradientStep);

				const FVoxelFloatBuffer GradientHeights = Query.SampleHeightLayer(WeakLayer, GradientPositions);

				PointNormals = FVoxelBufferGradientUtilities::CollapseGradient2DToNormal(GradientHeights, Points.Num(), GradientStep);
			}
			else
			{
				const float GradientStep = 100.f;

				FVoxelDoubleVectorBuffer GradientPositions = FVoxelBufferGradientUtilities::SplitPositions3D(Positions, GradientStep);

				const FVoxelFloatBuffer GradientDistances = Query.SampleVolumeLayer(WeakLayer, GradientPositions);

				PointNormals = FVoxelBufferGradientUtilities::CollapseGradient3DToNormal(GradientDistances, Points.Num(), GradientStep);
			}

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

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			if (FVoxelUtilities::IsNaN(Values[Index]))
			{
				continue;
			}

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

	return Voxel::GameTask([
		=,
		this,
		Points = MakeSharedCopy(MoveTemp(Points)),
		Values = MoveTemp(Values),
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

		FVoxelPCGUtilities::AddPointsToMetadata(*OutPointData->Metadata, *Points);

		FVoxelPCGUtilities::AddAttribute<float>(
			*OutPointData->Metadata,
			*Points,
			FName(HeightOrDistanceAttribute),
			Values.View());

		if (bQuerySurfaceTypes)
		{
			for (const auto& It : SurfaceTypeToWeight)
			{
				FVoxelPCGUtilities::AddAttribute<float>(
					*OutPointData->Metadata,
					*Points,
					FName(It.Key.GetName() + SurfaceAttributeSuffix),
					MakeVoxelArrayView(It.Value).LeftOf(Points->Num()));
			}

			FVoxelSurfaceType::ForeachSurfaceType([&](const FVoxelSurfaceType& Type)
			{
				FVoxelPCGUtilities::AddDefaultAttributeIfNeeded<float>(
					*OutPointData->Metadata,
					FName(Type.GetName() + SurfaceAttributeSuffix));
			});
		}

		for (const FVoxelMetadataRef& MetadataToQuery : MetadatasToQuery)
		{
			MetadataToQuery.AddToPCG(
				*OutPointData->Metadata,
				*Points,
				MetadataToQuery.GetFName() + MetadataAttributeSuffix,
				*MetadataToBuffer[MetadataToQuery]);
		}

		OutPointData->GetMutablePoints() = MoveTemp(*Points);
	});
}