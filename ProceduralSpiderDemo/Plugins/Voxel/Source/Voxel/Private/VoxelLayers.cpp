// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelLayers.h"
#include "VoxelQuery.h"
#include "VoxelDependency.h"
#include "VoxelStampDelta.h"
#include "VoxelStackLayer.h"
#include "VoxelHeightLayer.h"
#include "VoxelVolumeLayer.h"
#include "VoxelHeightStamp.h"
#include "VoxelBreadcrumbs.h"
#include "VoxelLayerTracker.h"
#include "Surface/VoxelSurfaceTypeTable.h"

FVoxelLayers& FVoxelLayers::Empty()
{
	static TSharedPtr<FVoxelLayers> Empty = INLINE_LAMBDA
	{
		GOnVoxelModuleUnloaded_DoCleanup.AddLambda([]
		{
			Empty.Reset();
		});

		return MakeShareable(new FVoxelLayers(
			{},
			FVoxelDependency::Create("FVoxelLayers::Empty"),
			0,
			{},
			{}));
	};

	return *Empty;
}

TSharedRef<FVoxelLayers> FVoxelLayers::Get(const TVoxelObjectPtr<UWorld> World)
{
	return FVoxelLayerTrackerSubsystem::Get(World)->GetLayerTracker()->GetLayers();
}

TSharedRef<FVoxelLayers> FVoxelLayers::Get(
	const TVoxelObjectPtr<UWorld> World,
	const AActor& Actor)
{
	ensure(Actor.GetWorld() == World);
	return FVoxelLayerTrackerSubsystem::Get(World)->GetLayerTracker(Actor)->GetLayers();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelLayers::~FVoxelLayers()
{
	VOXEL_FUNCTION_COUNTER();

	ConstCast(WeakLayerToHeightLayer).Empty();
	ConstCast(WeakLayerToVolumeLayer).Empty();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelLayers::HasLayer(
	const FVoxelWeakStackLayer& WeakLayer,
	FVoxelDependencyCollector& DependencyCollector) const
{
	DependencyCollector.AddDependency(*Dependency);

	if (WeakLayer.Type == EVoxelLayerType::Height)
	{
		return WeakLayerToHeightLayer.Contains(WeakLayer);
	}
	else
	{
		return WeakLayerToVolumeLayer.Contains(WeakLayer);
	}
}

TSharedPtr<const FVoxelHeightLayer> FVoxelLayers::FindHeightLayer(
	const FVoxelWeakStackLayer& WeakLayer,
	FVoxelDependencyCollector& DependencyCollector) const
{
	if (!ensure(WeakLayer.Type == EVoxelLayerType::Height))
	{
		return nullptr;
	}

	DependencyCollector.AddDependency(*Dependency);

	return WeakLayerToHeightLayer.FindRef(WeakLayer);
}

TSharedPtr<const FVoxelVolumeLayer> FVoxelLayers::FindVolumeLayer(
	const FVoxelWeakStackLayer& WeakLayer,
	FVoxelDependencyCollector& DependencyCollector) const
{
	if (!ensure(WeakLayer.Type == EVoxelLayerType::Volume))
	{
		return nullptr;
	}

	DependencyCollector.AddDependency(*Dependency);

	return WeakLayerToVolumeLayer.FindRef(WeakLayer);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelOptionalBox FVoxelLayers::GetBoundsToGenerate(
	const FVoxelWeakStackLayer& WeakLayer,
	FVoxelDependencyCollector& DependencyCollector) const
{
	VOXEL_FUNCTION_COUNTER();

	if (WeakLayer.Type == EVoxelLayerType::Height)
	{
		const TSharedPtr<const FVoxelHeightLayer> Layer = FindHeightLayer(WeakLayer, DependencyCollector);
		if (!Layer)
		{
			return {};
		}

		return Layer->GetBoundsToGenerate(DependencyCollector);
	}
	else
	{
		check(WeakLayer.Type == EVoxelLayerType::Volume);

		const TSharedPtr<const FVoxelVolumeLayer> Layer = FindVolumeLayer(WeakLayer, DependencyCollector);
		if (!Layer)
		{
			return {};
		}

		return Layer->GetBoundsToGenerate(DependencyCollector);
	}
}

TVoxelArray<FVoxelStampDelta> FVoxelLayers::GetStampDeltas(
	const FVoxelWeakStackLayer& WeakLayer,
	const FVector& Position,
	const int32 LOD) const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	TVoxelArray<FVoxelStampDelta> StampDeltas;
	float LastValue = FVoxelUtilities::NaNf();

	const auto AddDelta = [&](
		const FVoxelWeakStackLayer& Layer,
		const FVoxelStampRuntime& Stamp,
		const TConstVoxelArrayView<float> Values)
	{
		if (!ensure(Values.Num() == 1))
		{
			return;
		}

		float Value = Values[0];

		if (Stamp.IsA<FVoxelHeightStampRuntime>())
		{
			Value = Position.Z - Value;
		}

		StampDeltas.Add(FVoxelStampDelta
		{
			Layer,
			Stamp.AsShared(),
			LastValue,
			Value
		});

		LastValue = Value;
	};

	FVoxelBreadcrumbs Breadcrumbs;
	Breadcrumbs.PostApplyStamp.BulkHeight = [&](
		const FVoxelWeakStackLayer& Layer,
		const FVoxelHeightStampRuntime& Stamp,
		const FVoxelHeightBulkQuery& Query)
	{
		AddDelta(Layer, Stamp, Query.Heights);
	};
	Breadcrumbs.PostApplyStamp.SparseHeight = [&](
		const FVoxelWeakStackLayer& Layer,
		const FVoxelHeightStampRuntime& Stamp,
		const FVoxelHeightSparseQuery& Query)
	{
		AddDelta(Layer, Stamp, Query.IndirectHeights);
	};
	Breadcrumbs.PostApplyStamp.BulkVolume = [&](
		const FVoxelWeakStackLayer& Layer,
		const FVoxelVolumeStampRuntime& Stamp,
		const FVoxelVolumeBulkQuery& Query)
	{
		AddDelta(Layer, Stamp, Query.Distances);
	};
	Breadcrumbs.PostApplyStamp.SparseVolume = [&](
		const FVoxelWeakStackLayer& Layer,
		const FVoxelVolumeStampRuntime& Stamp,
		const FVoxelVolumeSparseQuery& Query)
	{
		AddDelta(Layer, Stamp, Query.IndirectDistances);
	};

	const FVoxelQuery Query(
		LOD,
		*this,
		*FVoxelSurfaceTypeTable::Get(),
		FVoxelDependencyCollector::Null,
		&Breadcrumbs);

	(void)Query.SampleVolumeLayer(WeakLayer, Position);

	return StampDeltas;
}