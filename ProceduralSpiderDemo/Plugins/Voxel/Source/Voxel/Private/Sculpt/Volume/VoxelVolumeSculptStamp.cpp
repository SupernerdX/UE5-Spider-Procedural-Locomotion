// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeSculptStamp.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "VoxelQuery.h"
#include "VoxelWorld.h"
#include "VoxelLayerStack.h"
#include "Sculpt/Volume/VoxelSculptVolume.h"

FVoxelVolumeSculptStamp::FVoxelVolumeSculptStamp()
{
	BlendMode = EVoxelVolumeBlendMode::Override;
}

TVoxelOptional<FVoxelWeakStackLayer> FVoxelVolumeSculptStamp::GetWeakStackLayer(const UWorld& World) const
{
	VOXEL_FUNCTION_COUNTER();

	if (!Layer)
	{
		VOXEL_MESSAGE(Error, "Cannot sculpt: Stamp has no layer assigned");
		return {};
	}

	if (StackOverride)
	{
		if (!StackOverride->VolumeLayers.Contains(Layer))
		{
			VOXEL_MESSAGE(Error,
				"Cannot sculpt: Failed to find a matching layer in StackOverride.\n"
				"StackOverride: {0}\n"
				"StackOverride layers: {1}\n"
				"Stamp layer: {2}",
				StackOverride,
				StackOverride->VolumeLayers,
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

		if (Stack->VolumeLayers.Contains(Layer))
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
void FVoxelVolumeSculptStamp::GetPropertyInfo(FPropertyInfo& Info) const
{
	Info.bIsSmoothnessVisible = false;
	Info.bIsMetadataOverridesVisible = false;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelVolumeSculptStampRuntime::Initialize(FVoxelDependencyCollector& DependencyCollector)
{
	VOXEL_FUNCTION_COUNTER();
	checkUObjectAccess();

	const AVoxelSculptVolume* SculptActor = Stamp.WeakSculptActor.Resolve();
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

	return SculptData->GetBounds().IsValid();
}

FVoxelBox FVoxelVolumeSculptStampRuntime::GetLocalBounds() const
{
	FVoxelBox Bounds = SculptData->GetBounds().ToVoxelBox().Scale(Stamp.Scale);

	if (!Bounds.IsValidAndNotEmpty() &&
		Stamp.bIsInfinite)
	{
		// Always initialize stamp if infinite to avoid huge first refresh
		Bounds = FVoxelBox(0, 1);
	}
	ensure(Bounds.IsValidAndNotEmpty());

	return Bounds;
}

bool FVoxelVolumeSculptStampRuntime::HasCollectDependencies() const
{
	return Stamp.bIsInfinite;
}

bool FVoxelVolumeSculptStampRuntime::CanPartiallyInvalidate() const
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelVolumeSculptStampRuntime::TryToPartiallyInvalidate(
	const FVoxelStampRuntime& PreviousRuntime,
	TVoxelArray<FVoxelBox>& OutLocalBoundsToInvalidate) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelVolumeSculptStampRuntime& TypedPreviousRuntime = CastStructChecked<FVoxelVolumeSculptStampRuntime>(PreviousRuntime);
	if (TypedPreviousRuntime.WeakSculptActor != WeakSculptActor ||
		TypedPreviousRuntime.Stamp.Scale != Stamp.Scale ||
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeSculptStampRuntime::CollectDependencies(
	FVoxelDependencyCollector& DependencyCollector,
	const FVoxelVolumeTransform& StampToQuery,
	const FVoxelBox& Bounds) const
{
	if (Bounds.IsInfinite())
	{
		DependencyCollector.AddDependency(*Dependency, FVoxelBox::Infinite);
		return;
	}

	const FVoxelBox LocalBounds = StampToQuery.InverseTransform(Bounds).Scale(1. / Stamp.Scale);
	DependencyCollector.AddDependency(*Dependency, LocalBounds);
}

void FVoxelVolumeSculptStampRuntime::Apply(
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelBox Bounds = StampToQuery.InverseTransform(Query.PositionBounds).Scale(1. / Stamp.Scale);
	Query.AddDependency(*Dependency, Bounds);

	const EVoxelVolumeChunkQuality Quality =
		Query.Query.LOD <= Stamp.NearMaxLOD
		? EVoxelVolumeChunkQuality::Near
		: Query.Query.LOD <= Stamp.MidMaxLOD
		? EVoxelVolumeChunkQuality::Mid
		: EVoxelVolumeChunkQuality::Far;

	SculptData->Apply(
		*BulkLoader,
		Query,
		StampToQuery,
		Stamp.Scale,
		Stamp.Behavior,
		Stamp.BlendMode,
		Stamp.bApplyOnVoid,
		Quality,
		RootHash);
}