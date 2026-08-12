// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightSculptStamp.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "VoxelQuery.h"
#include "VoxelWorld.h"
#include "VoxelLayerStack.h"
#include "Sculpt/Height/VoxelSculptHeight.h"

FVoxelHeightSculptStamp::FVoxelHeightSculptStamp()
{
	BlendMode = EVoxelHeightBlendMode::Override;
}

TVoxelOptional<FVoxelWeakStackLayer> FVoxelHeightSculptStamp::GetWeakStackLayer(const UWorld& World) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Layer)
	{
		VOXEL_MESSAGE(Error, "Cannot sculpt: Stamp has no layer assigned");
		return {};
	}

	if (StackOverride)
	{
		if (!StackOverride->HeightLayers.Contains(Layer))
		{
			VOXEL_MESSAGE(Error,
				"Cannot sculpt: Failed to find a matching layer in StackOverride.\n"
				"StackOverride: {0}\n"
				"StackOverride layers: {1}\n"
				"Stamp layer: {2}",
				StackOverride,
				StackOverride->HeightLayers,
				Layer);

			return {};
		}

		return FVoxelStackLayer(StackOverride, Layer);
	}

	for (const AVoxelWorld* VoxelWorld : TActorRange<AVoxelWorld>(&World))
	{
		UVoxelLayerStack* Stack = VoxelWorld->LayerStack;
		if (!Stack)
		{
			continue;
		}

		if (Stack->HeightLayers.Contains(Layer))
		{
			return FVoxelStackLayer(Stack, Layer);
		}
	}

	TVoxelArray<const AVoxelWorld*> VoxelWorlds;
	for (const AVoxelWorld* VoxelWorld : TActorRange<AVoxelWorld>(&World))
	{
		VoxelWorlds.Add(VoxelWorld);
	}

	VOXEL_MESSAGE(Error,
		"Cannot sculpt: Failed to find a Voxel World with a compatible stack.\n"
		"You need a Voxel World with a LayerStack containing the following layer in your scene: {0}\n"
		"If this sculpt stamp is not meant to be rendered with a Voxel World, manually set StackOverride\n"
		"Voxel Worlds checked: {1}",
		Layer,
		VoxelWorlds);

	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void FVoxelHeightSculptStamp::GetPropertyInfo(FPropertyInfo& Info) const
{
	Info.bIsSmoothnessVisible = false;
	Info.bIsMetadataOverridesVisible = false;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelHeightSculptStampRuntime::Initialize(FVoxelDependencyCollector& DependencyCollector)
{
	VOXEL_FUNCTION_COUNTER();
	checkUObjectAccess();

	const AVoxelSculptHeight* SculptActor = Stamp.WeakSculptActor.Resolve();
	if (!ensure(SculptActor))
	{
		return false;
	}

	WeakSculptActor = SculptActor;
	Dependency = SculptActor->GetDependency();
	BulkLoader = SculptActor->GetBulkLoader();
	SculptData = SculptActor->GetSculptData().GetShared();
	RootHash = SculptActor->GetSculptData().GetHash();

	if (Stamp.bIsInfinite)
	{
		// Always initialize stamp if infinite to avoid huge first refresh
		return true;
	}

	return SculptData->GetBounds().IsValidAndNotEmpty();
}

FVoxelBox FVoxelHeightSculptStampRuntime::GetLocalBounds() const
{
	FVoxelBox Bounds = SculptData->GetBounds().Scale(FVector(Stamp.ScaleXY, Stamp.ScaleXY, 1.f));

	if (!Bounds.IsValidAndNotEmpty() &&
		Stamp.bIsInfinite)
	{
		// Always initialize stamp if infinite to avoid huge first refresh
		Bounds = FVoxelBox(0, 1);
	}
	ensure(Bounds.IsValidAndNotEmpty());

	return Bounds;
}

bool FVoxelHeightSculptStampRuntime::HasCollectDependencies() const
{
	return Stamp.bIsInfinite;
}

bool FVoxelHeightSculptStampRuntime::CanPartiallyInvalidate() const
{
	return true;
}

bool FVoxelHeightSculptStampRuntime::HasRelativeHeightRange() const
{
	return Stamp.bStoreRelativeHeights;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelHeightSculptStampRuntime::TryToPartiallyInvalidate(
	const FVoxelStampRuntime& PreviousRuntime,
	TVoxelArray<FVoxelBox>& OutLocalBoundsToInvalidate) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelHeightSculptStampRuntime& TypedPreviousRuntime = CastStructChecked<FVoxelHeightSculptStampRuntime>(PreviousRuntime);
	if (TypedPreviousRuntime.WeakSculptActor != WeakSculptActor ||
		TypedPreviousRuntime.Stamp.ScaleXY != Stamp.ScaleXY ||
		TypedPreviousRuntime.Stamp.bIsInfinite != Stamp.bIsInfinite ||
		TypedPreviousRuntime.Stamp.NearMaxLOD != Stamp.NearMaxLOD ||
		TypedPreviousRuntime.Stamp.MidMaxLOD != Stamp.MidMaxLOD)
	{
		return false;
	}

	if (!Stamp.bIsInfinite)
	{
		// Invalidate any new bounds
		GetLocalBounds().Remove_Split(
			TypedPreviousRuntime.GetLocalBounds(),
			OutLocalBoundsToInvalidate);
	}

	return true;
}

void FVoxelHeightSculptStampRuntime::CollectDependencies(
	FVoxelDependencyCollector& DependencyCollector,
	const FVoxelHeightTransform& StampToQuery,
	const FVoxelBox2D& Bounds) const
{
	if (Bounds.IsInfinite())
	{
		DependencyCollector.AddDependency(*Dependency, FVoxelBox2D::Infinite);
		return;
	}

	const FVoxelBox2D LocalBounds = StampToQuery.InverseTransform(Bounds).Scale(1. / Stamp.ScaleXY);
	DependencyCollector.AddDependency(*Dependency, LocalBounds);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightSculptStampRuntime::Apply(
	const FVoxelHeightSparseQuery& Query,
	const FVoxelHeightTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelBox2D Bounds = StampToQuery.InverseTransform(Query.PositionBounds).Scale(1. / Stamp.ScaleXY);
	Query.AddDependency(*Dependency, Bounds);

	const EVoxelHeightChunkQuality Quality =
		Query.Query.LOD <= Stamp.NearMaxLOD
		? EVoxelHeightChunkQuality::Near
		: Query.Query.LOD <= Stamp.MidMaxLOD
		? EVoxelHeightChunkQuality::Mid
		: EVoxelHeightChunkQuality::Far;

	SculptData->Apply(
		*BulkLoader,
		Query,
		StampToQuery,
		Stamp.ScaleXY,
		Stamp.Behavior,
		Stamp.BlendMode,
		Stamp.bApplyOnVoid,
		Quality,
		RootHash);
}