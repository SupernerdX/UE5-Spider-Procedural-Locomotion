// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelSculptVolumeDataSource.h"
#include "Sculpt/Volume/VoxelSculptVolume.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "Sculpt/Volume/VoxelSculptVolumeInvalidationBuilder.h"
#include "VoxelAABBTree.h"
#include "VoxelDependency.h"
#include "VoxelInvalidationCallstack.h"

IVoxelSculptVolumeDataSource::IVoxelSculptVolumeDataSource(AVoxelSculptVolume& SculptActor)
	: WeakSculptActor(SculptActor)
	, PrivateData(MakeShared<FVoxelSculptVolumeData>())
{
}

TVoxelBulkRef<FVoxelSculptVolumeData> IVoxelSculptVolumeDataSource::GetData() const
{
	return PrivateData;
}

bool IVoxelSculptVolumeDataSource::IsEditor() const
{
	const AVoxelSculptVolume* SculptActor = WeakSculptActor.Resolve();
	if (!ensureVoxelSlow(SculptActor))
	{
		return false;
	}

	const UWorld* World = SculptActor->GetWorld();
	if (!ensureVoxelSlow(World))
	{
		return false;
	}

	return !World->IsGameWorld();
}

TVoxelOptional<FVoxelSculptVolumeContext> IVoxelSculptVolumeDataSource::GetContext() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const AVoxelSculptVolume* SculptActor = WeakSculptActor.Resolve();
	if (!ensureVoxelSlow(SculptActor))
	{
		return {};
	}

	return SculptActor->GetSculptContext();
}

void IVoxelSculptVolumeDataSource::SetData(const TVoxelBulkRef<FVoxelSculptVolumeData>& Data)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread() || IsInAsyncLoadingThread());

	if (PrivateData.GetHash() == Data.GetHash())
	{
		return;
	}

	const TSharedRef<const FVoxelAABBTree> InvalidationTree = FVoxelSculptVolumeInvalidationBuilder::Create(
		PrivateData->GetKeyToFarChunk(),
		Data->GetKeyToFarChunk());

	PrivateData = Data;

	if (!IsInGameThread())
	{
		return;
	}

	const AVoxelSculptVolume* SculptActor = WeakSculptActor.Resolve();
	if (!ensureVoxelSlow(SculptActor))
	{
		return;
	}

	if (GIsEditor &&
		!SculptActor->GetComponent().ExternalAsset)
	{
		(void)SculptActor->MarkPackageDirty();
	}

	const FVoxelInvalidationScope LocalScope("Update sculpt");

	SculptActor->GetDependency()->Invalidate(InvalidationTree);

	const TSharedPtr<const FVoxelStampRuntime> StampRuntime = SculptActor->GetStamp()->ResolveStampRuntime();
	if (!StampRuntime)
	{
		return;
	}

	StampRuntime->RequestUpdate();

	extern bool GVoxelStampShowInvalidations;

	if (GVoxelStampShowInvalidations)
	{
		FVoxelDebugDrawer Drawer(StampRuntime->GetWorld());

		InvalidationTree->DrawLeaves(
			Drawer.Color(FLinearColor::Red).LifeTime(1.f),
			FScaleMatrix(StampRuntime->GetStamp().AsChecked<FVoxelVolumeSculptStamp>().Scale) *
			StampRuntime->GetLocalToWorld().ToMatrixWithScale());
	}
}