// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "VoxelHeightLayer.h"
#include "VoxelVolumeLayer.h"

bool FVoxelQuery::CheckNoRecursion(const FVoxelWeakStackLayer& WeakLayer) const
{
	if (!QueriedLayers.Contains(WeakLayer))
	{
		return true;
	}

	TVoxelArray<FString> QueriedLayersString;
	for (const FVoxelWeakStackLayer& QueriedLayer : GetQueriedLayers())
	{
		QueriedLayersString.Add(QueriedLayer.ToString());
	}

	VOXEL_MESSAGE(Error, "Cannot query {1}: already queried in the stack. Queried layers: {2}",
		WeakLayer.ToString(),
		QueriedLayersString);

	return false;
}

FVoxelQuery FVoxelQuery::MakeChild_Layer(const FVoxelWeakStackLayer& WeakLayer) const
{
	ensureVoxelSlow(!QueriedLayers.Contains(WeakLayer));

	FVoxelQuery Result = FVoxelQuery(
		LOD,
		Layers,
		SurfaceTypeTable,
		DependencyCollector,
		Breadcrumbs);

	Result.ShouldStopTraversal_BeforeStamp = ShouldStopTraversal_BeforeStamp;
	Result.ShouldStopTraversal_AfterStamp = ShouldStopTraversal_AfterStamp;
	Result.QueriedLayers.Reserve(QueriedLayers.Num() + 1);
	Result.QueriedLayers.Append(QueriedLayers);
	Result.QueriedLayers.Add_EnsureNoGrow(WeakLayer);
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelQuery::HasStamps(
	const FVoxelWeakStackLayer& WeakLayer,
	const FVoxelBox& Bounds,
	const EVoxelStampBehavior BehaviorMask) const
{
	VOXEL_FUNCTION_COUNTER();

	if (WeakLayer.Type == EVoxelLayerType::Height)
	{
		const TSharedPtr<const FVoxelHeightLayer> Layer = Layers.FindHeightLayer(WeakLayer, DependencyCollector);
		if (!ensureVoxelSlow(Layer))
		{
			return false;
		}

		return Layer->HasStamps(*this, Bounds, BehaviorMask, false);
	}
	else
	{
		check(WeakLayer.Type == EVoxelLayerType::Volume);

		const TSharedPtr<const FVoxelVolumeLayer> Layer = Layers.FindVolumeLayer(WeakLayer, DependencyCollector);
		if (!ensureVoxelSlow(Layer))
		{
			return false;
		}

		return Layer->HasStamps(*this, Bounds, BehaviorMask);
	}
}

TVoxelOptional<FVoxelWeakStackLayer> FVoxelQuery::GetFirstHeightLayer(const FVoxelWeakStackLayer& WeakLayer) const
{
	if (WeakLayer.Type == EVoxelLayerType::Height)
	{
		return WeakLayer;
	}
	check(WeakLayer.Type == EVoxelLayerType::Volume);

	TSharedPtr<const FVoxelVolumeLayer> Layer = Layers.FindVolumeLayer(WeakLayer, DependencyCollector);
	if (!ensureVoxelSlow(Layer))
	{
		return {};
	}

	while (Layer->PreviousVolumeLayer)
	{
		Layer = Layer->PreviousVolumeLayer;
	}

	if (!Layer->PreviousHeightLayer)
	{
		return {};
	}

	return Layer->PreviousHeightLayer->WeakLayer;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer FVoxelQuery::SampleHeightLayer(
	const FVoxelWeakStackLayer& WeakLayer,
	const FVector2D& Start,
	const FIntPoint& Size,
	const float Step) const
{
	VOXEL_SCOPE_COUNTER_FORMAT("SampleHeightLayer %dx%d", Size.X, Size.Y);
	ensure(Step > 0);
	ensure(WeakLayer.Type == EVoxelLayerType::Height);

	FVoxelFloatBuffer OutHeights;
	OutHeights.Allocate(Size.X * Size.Y);
	OutHeights.SetAll(FVoxelUtilities::NaNf());

	const TSharedPtr<const FVoxelHeightLayer> Layer = Layers.FindHeightLayer(WeakLayer, DependencyCollector);
	if (ensureVoxelSlow(Layer))
	{
		Layer->Sample(FVoxelHeightBulkQuery::Create(
			*this,
			OutHeights.View(),
			Start,
			Size,
			Step));
	}

	return OutHeights;
}

FVoxelFloatBuffer FVoxelQuery::SampleHeightLayer(
	const FVoxelWeakStackLayer& WeakLayer,
	const FVoxelDoubleVector2DBuffer& Positions,
	const TVoxelArrayView<FVoxelSurfaceTypeBlend> OutSurfaceTypes,
	const TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>>& OutMetadataToBuffer) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num(), 128);
	ensure(WeakLayer.Type == EVoxelLayerType::Height);

	FVoxelFloatBuffer OutHeights;
	OutHeights.Allocate(Positions.Num());
	OutHeights.SetAll(FVoxelUtilities::NaNf());

	const TSharedPtr<const FVoxelHeightLayer> Layer = Layers.FindHeightLayer(WeakLayer, DependencyCollector);
	if (ensureVoxelSlow(Layer))
	{
		Layer->Sample(FVoxelHeightSparseQuery::Create(
			*this,
			OutHeights.View(),
			OutSurfaceTypes,
			OutMetadataToBuffer,
			Positions,
			OutSurfaceTypes.Num() > 0,
			OutMetadataToBuffer.KeyArray()));
	}

	return OutHeights;
}

FVoxelFloatBuffer FVoxelQuery::SampleHeightLayer(
	const FVoxelWeakStackLayer& WeakLayer,
	const FVoxelDoubleVector2DBuffer& Positions) const
{
	return SampleHeightLayer(
		WeakLayer,
		Positions,
		{},
		{});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer FVoxelQuery::SampleVolumeLayer(
	const FVoxelWeakStackLayer& WeakLayer,
	const FVector& Start,
	const FIntVector& Size,
	const float Step) const
{
	VOXEL_SCOPE_COUNTER_FORMAT("SampleVolumeLayer %dx%dx%d", Size.X, Size.Y, Size.Z);
	ensure(Step > 0);

	FVoxelFloatBuffer OutDistances;
	OutDistances.Allocate(Size.X * Size.Y * Size.Z);
	OutDistances.SetAll(FVoxelUtilities::NaNf());

	const FVoxelVolumeBulkQuery Query = FVoxelVolumeBulkQuery::Create(
		*this,
		OutDistances.View(),
		Start,
		Size,
		Step);

	if (WeakLayer.Type == EVoxelLayerType::Height)
	{
		const TSharedPtr<const FVoxelHeightLayer> Layer = Layers.FindHeightLayer(WeakLayer, DependencyCollector);
		if (ensureVoxelSlow(Layer))
		{
			Layer->SampleAsVolume(Query);
		}
	}
	else
	{
		check(WeakLayer.Type == EVoxelLayerType::Volume);

		const TSharedPtr<const FVoxelVolumeLayer> Layer = Layers.FindVolumeLayer(WeakLayer, DependencyCollector);
		if (ensureVoxelSlow(Layer))
		{
			Layer->Sample(Query);
		}
	}

	return OutDistances;
}

FVoxelFloatBuffer FVoxelQuery::SampleVolumeLayer(
	const FVoxelWeakStackLayer& WeakLayer,
	const FVoxelDoubleVectorBuffer& Positions,
	const TVoxelArrayView<FVoxelSurfaceTypeBlend> OutSurfaceTypes,
	const TVoxelMap<FVoxelMetadataRef, TSharedRef<FVoxelBuffer>>& OutMetadataToBuffer) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num(), 128);

	FVoxelFloatBuffer OutDistances;
	OutDistances.Allocate(Positions.Num());
	OutDistances.SetAll(FVoxelUtilities::NaNf());

	const FVoxelVolumeSparseQuery Query = FVoxelVolumeSparseQuery::Create(
		*this,
		OutDistances.View(),
		OutSurfaceTypes,
		OutMetadataToBuffer,
		Positions,
		OutSurfaceTypes.Num() > 0,
		OutMetadataToBuffer.KeyArray());

	if (WeakLayer.Type == EVoxelLayerType::Height)
	{
		const TSharedPtr<const FVoxelHeightLayer> Layer = Layers.FindHeightLayer(WeakLayer, DependencyCollector);
		if (ensureVoxelSlow(Layer))
		{
			Layer->SampleAsVolume(Query);
		}
	}
	else
	{
		check(WeakLayer.Type == EVoxelLayerType::Volume);

		const TSharedPtr<const FVoxelVolumeLayer> Layer = Layers.FindVolumeLayer(WeakLayer, DependencyCollector);
		if (ensureVoxelSlow(Layer))
		{
			Layer->Sample(Query);
		}
	}

	return OutDistances;
}

FVoxelFloatBuffer FVoxelQuery::SampleVolumeLayer(
	const FVoxelWeakStackLayer& WeakLayer,
	const FVoxelDoubleVectorBuffer& Positions) const
{
	return SampleVolumeLayer(
		WeakLayer,
		Positions,
		{},
		{});
}