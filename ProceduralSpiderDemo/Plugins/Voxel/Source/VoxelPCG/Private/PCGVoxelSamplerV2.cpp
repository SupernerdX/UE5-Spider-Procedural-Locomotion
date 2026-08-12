// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGVoxelSamplerV2.h"
#include "VoxelLayer.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "VoxelPointId.h"
#include "PCGComponent.h"
#include "VoxelPointSet.h"
#include "VoxelPCGUtilities.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGHelpers.h"
#include "Data/PCGSpatialData.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Scatter/VoxelScatterUtilities.h"
#include "Surface/VoxelSurfaceTypeTable.h"
#include "Surface/VoxelSurfaceTypeBlendBuffer.h"

FString UPCGVoxelSamplerV2Settings::GetAdditionalTitleInformation() const
{
	FString Title = Layer.GetType() == EVoxelLayerType::Height ? "2D" : "3D";
	if (Layer.Layer)
	{
		Title += " (" + Layer.Layer->GetName() + ")";
	}

	return Title; 
}

TArray<FPCGPinProperties> UPCGVoxelSamplerV2Settings::InputPinProperties() const
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

TArray<FPCGPinProperties> UPCGVoxelSamplerV2Settings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Properties;
	Properties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return Properties;
}

TSharedPtr<FVoxelPCGOutput> UPCGVoxelSamplerV2Settings::CreateOutput(FPCGContext& Context) const
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

	UPCGPointData* PointData = NewObject<UPCGPointData>();
	Context.OutputData.TaggedData.Emplace_GetRef().Data = PointData;

	return MakeShared<FVoxelSamplerV2PCGOutput>(
		FVoxelLayers::Get(Component->GetWorld()),
		FVoxelSurfaceTypeTable::Get(),
		Bounds,
		Layer,
		LOD,
		bResolveSmartSurfaceTypes,
		DistanceBetweenPoints,
		Looseness,
		HeightScatterType,
		MinCellArea,
		PointData,
		FVoxelMetadataRef::GetUniqueValidRefs(MetadatasToQuery));
}

FString UPCGVoxelSamplerV2Settings::GetNodeDebugInfo() const
{
	return Super::GetNodeDebugInfo() + " [Layer: " + FString(Layer.Layer ? Layer.Layer->GetName() : "None") + "]";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFuture FVoxelSamplerV2PCGOutput::Run() const
{
	return Voxel::AsyncTask([this]
	{
		return GeneratePoints(WeakLayer.Type == EVoxelLayerType::Height);
	});
}

FVoxelFuture FVoxelSamplerV2PCGOutput::GeneratePoints(const bool bHeight) const
{
	if (!Layers->HasLayer(WeakLayer, GetDependencyCollector()))
	{
		return {};
	}

	const FIntVector CellMin = FVoxelUtilities::CeilToInt(Bounds.Min / DistanceBetweenPoints);
	const FIntVector CellMax = FVoxelUtilities::FloorToInt(Bounds.Max / DistanceBetweenPoints);

	if (CellMin.X > CellMax.X ||
		CellMin.Y > CellMax.Y ||
		CellMin.Z > CellMax.Z)
	{
		VOXEL_MESSAGE(Error, "Invalid distance between points: {0}", DistanceBetweenPoints);
		return {};
	}

	const FVoxelQuery Query(
		LOD,
		*Layers,
		*SurfaceTypeTable,
		GetDependencyCollector());

	FVoxelPointSet PointSet;
	if (bHeight)
	{
		PointSet = FVoxelScatterUtilities::ScatterPoints2D(
			Query,
			Bounds,
			{},
			DistanceBetweenPoints,
			GetSeed(),
			Looseness,
			HeightScatterType,
			WeakLayer,
			true,
			bResolveSmartSurfaceTypes,
			MetadatasToQuery);
	}
	else
	{
		PointSet = FVoxelScatterUtilities::ScatterPoints3D(
			Query,
			Bounds,
			{},
			DistanceBetweenPoints,
			GetSeed(),
			Looseness,
			MinCellArea,
			WeakLayer,
			true,
			bResolveSmartSurfaceTypes,
			MetadatasToQuery);
	}

	if (PointSet.Num() == 0)
	{
		return {};
	}

	TVoxelArray<FPCGPoint> Points;
	FVoxelUtilities::SetNumFast(Points, PointSet.Num());

	const FVoxelPointIdBuffer* IdBuffer = PointSet.Find<FVoxelPointIdBuffer>(FVoxelPointAttributes::Id);
	const FVoxelDoubleVectorBuffer* PositionBuffer = PointSet.Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
	const FVoxelQuaternionBuffer* RotationBuffer = PointSet.Find<FVoxelQuaternionBuffer>(FVoxelPointAttributes::Rotation);
	const FVoxelSurfaceTypeBlendBuffer* SurfaceTypesBuffer = PointSet.Find<FVoxelSurfaceTypeBlendBuffer>(FVoxelPointAttributes::SurfaceTypes);

	if (!ensure(IdBuffer) ||
		!ensure(PositionBuffer) ||
		!ensure(RotationBuffer) ||
		!ensure(SurfaceTypesBuffer))
	{
		return {};
	}

	const FBox PointBounds(FVector(-0.5f * DistanceBetweenPoints), FVector(0.5f * DistanceBetweenPoints));

	TVoxelMap<FVoxelSurfaceType, TVoxelArray<float>> SurfaceTypeToWeight;
	SurfaceTypeToWeight.Reserve(32);

	const FRandomStream Stream(GetSeed());
	for (int32 Index = 0; Index < PointSet.Num(); Index++)
	{
		FPCGPoint& Point = Points[Index];
		Point = FPCGPoint();

		Point.Transform = FTransform(
			FQuat((*RotationBuffer)[Index]),
			(*PositionBuffer)[Index]);

		Point.Density = Stream.FRand();
		Point.Seed = (*IdBuffer)[Index].PointId;
		Point.MetadataEntry = Index;

		Point.SetLocalBounds(PointBounds);

		for (const FVoxelSurfaceTypeBlendLayer& Layer : (*SurfaceTypesBuffer)[Index].GetLayers())
		{
			if (!SurfaceTypeToWeight.Contains(Layer.Type))
			{
				SurfaceTypeToWeight.Add_CheckNew(Layer.Type).SetNumZeroed(PointSet.Num());
			}
			SurfaceTypeToWeight.FindChecked(Layer.Type)[Index] += Layer.Weight.ToFloat();
		}
	}

	return Voxel::GameTask([
		=,
		this,
		Points = MakeSharedCopy(MoveTemp(Points)),
		PointSet = MoveTemp(PointSet),
		SurfaceTypeToWeight = MoveTemp(SurfaceTypeToWeight)]
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
			const FVoxelBuffer* MetadataBuffer = PointSet.Find(MetadataToQuery.GetFName());
			if (!MetadataBuffer)
			{
				continue;
			}

			MetadataToQuery.AddToPCG(
				*PointData->Metadata,
				*Points,
				MetadataToQuery.GetFName(),
				*MetadataBuffer);
		}

		PointData->GetMutablePoints() = MoveTemp(*Points);
	});
}