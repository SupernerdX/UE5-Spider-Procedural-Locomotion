// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterFunctionLibrary.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "Scatter/VoxelScatterUtilities.h"
#include "Graphs/VoxelStampGraphParameters.h"

FVoxelGraphParameters::FPointSetChunkBounds::FPointSetChunkBounds(
	const FVoxelBox& Bounds,
	const bool bCanExtendBounds)
	: Bounds(Bounds)
	, bCanExtendBounds(bCanExtendBounds)
{}

FVoxelGraphParameters::FPointSetChunkBounds::FPointSetChunkBounds(const FPointSetChunkBounds& Other)
	: Bounds(Other.Bounds)
	, ExtendedBounds(Other.ExtendedBounds)
	, bCanExtendBounds(Other.bCanExtendBounds)
{
}

void FVoxelGraphParameters::FPointSetChunkBounds::ExtendBounds(const FVoxelBox& NewBounds)
{
	if (!bCanExtendBounds)
	{
		return;
	}

	if (ExtendedBounds.IsSet())
	{
		ExtendedBounds = ExtendedBounds.GetValue().UnionWith(NewBounds);
	}
	else
	{
		ExtendedBounds = NewBounds;
	}
}

FVoxelBox FVoxelGraphParameters::FPointSetChunkBounds::GetBounds() const
{
	if (ExtendedBounds.IsSet())
	{
		return ExtendedBounds.GetValue();
	}
	return Bounds;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox UVoxelScatterFunctionLibrary::GetScatterBounds(const bool bExtendedBounds) const
{
	const FVoxelGraphParameters::FPointSetChunkBounds* Parameter = Query->FindParameter<FVoxelGraphParameters::FPointSetChunkBounds>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot query scatter bounds here", this);
		return {};
	}

	if (bExtendedBounds)
	{
		return Parameter->GetBounds();
	}

	return Parameter->GetInitialBounds();
}

FVoxelPointSet UVoxelScatterFunctionLibrary::GeneratePoints(
	const FVoxelWeakStackLayer& Layer,
	const float DistanceBetweenPoints,
	const EVoxelHeightScatterType HeightScatterType,
	const float Looseness,
	const float MinCellArea,
	const FVoxelSeed& Seed,
	const bool bQuerySurfaceTypes,
	const bool bResolveSurfaceTypes,
	const FVoxelMetadataRefBuffer& MetadatasToQuery)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelGraphParameters::FPointSetChunkBounds* BoundsParameter = Query->FindParameter<FVoxelGraphParameters::FPointSetChunkBounds>();
	if (!BoundsParameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot query scatter bounds here", this);
		return {};
	}

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot query here", this);
		return {};
	}
	const FVoxelQuery& VoxelQuery = Parameter->Query;

	if (!VoxelQuery.CheckNoRecursion(Layer) ||
		!VoxelQuery.Layers.HasLayer(Layer, Query->Context.DependencyCollector))
	{
		return {};
	}

	const FVoxelBox Bounds = BoundsParameter->GetBounds();
	if (!VoxelQuery.HasStamps(Layer, Bounds, EVoxelStampBehavior::AffectShape))
	{
		return {};
	}

	TOptional<FVoxelBox> OptionalInitialBounds;
	if (BoundsParameter->HasExtendedBounds())
	{
		OptionalInitialBounds = BoundsParameter->GetInitialBounds();
	}

	TVoxelArray<FVoxelMetadataRef> Metadatas;
	Metadatas.Reserve(MetadatasToQuery.Num());

	for (int32 Index = 0; Index < MetadatasToQuery.Num(); Index++)
	{
		FVoxelMetadataRef MetadataRef = MetadatasToQuery[Index];
		if (!MetadataRef.IsValid())
		{
			continue;
		}

		Metadatas.Add(MetadatasToQuery[Index]);
	}

	if (Layer.Type == EVoxelLayerType::Height)
	{
		return
			FVoxelScatterUtilities::ScatterPoints2D(
				VoxelQuery,
				Bounds,
				OptionalInitialBounds,
				DistanceBetweenPoints,
				Seed.Seed,
				Looseness,
				HeightScatterType,
				Layer,
				bQuerySurfaceTypes,
				bResolveSurfaceTypes,
				Metadatas);
	}

	return
		FVoxelScatterUtilities::ScatterPoints3D(
			VoxelQuery,
			Bounds,
			OptionalInitialBounds,
			DistanceBetweenPoints,
			Seed.Seed,
			Looseness,
			MinCellArea,
			Layer,
			bQuerySurfaceTypes,
			bResolveSurfaceTypes,
			Metadatas);
}

FVoxelPointSet UVoxelScatterFunctionLibrary::CubicPointGenerator(
	const FVoxelWeakStackLayer& Layer,
	const int32 VoxelSize,
	const int32 ChunkSize,
	const FVoxelSeed& Seed,
	const bool bQuerySurfaceTypes,
	const bool bResolveSurfaceTypes,
	const bool bSkipNaNFaces,
	const FVoxelMetadataRefBuffer& MetadatasToQuery)
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelGraphParameters::FPointSetChunkBounds* BoundsParameter = Query->FindParameter<FVoxelGraphParameters::FPointSetChunkBounds>();
	if (!BoundsParameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot query scatter bounds here", this);
		return {};
	}

	const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: Cannot query here", this);
		return {};
	}
	const FVoxelQuery& VoxelQuery = Parameter->Query;

	if (!VoxelQuery.CheckNoRecursion(Layer) ||
		!VoxelQuery.Layers.HasLayer(Layer, Query->Context.DependencyCollector))
	{
		return {};
	}

	const FVoxelBox Bounds = BoundsParameter->GetBounds();
	if (!VoxelQuery.HasStamps(Layer, Bounds, EVoxelStampBehavior::AffectShape))
	{
		return {};
	}

	TVoxelArray<FVoxelMetadataRef> Metadatas;
	FVoxelUtilities::SetNum(Metadatas, MetadatasToQuery.Num());
	for (int32 Index = 0; Index < MetadatasToQuery.Num(); Index++)
	{
		Metadatas.Add(MetadatasToQuery[Index]);
	}

	return
		FVoxelScatterUtilities::ScatterPointsCubicFaces(
			VoxelQuery,
			Bounds,
			VoxelSize,
			ChunkSize,
			Seed.Seed,
			Layer,
			bQuerySurfaceTypes,
			bResolveSurfaceTypes,
			bSkipNaNFaces,
			Metadatas);
}