// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelSculptHeightDataSource.h"
#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "Sculpt/Height/VoxelSculptHeightContext.h"
#include "Sculpt/Height/VoxelSculptHeightInvalidationBuilder.h"
#include "VoxelAABBTree2D.h"
#include "VoxelDependency.h"
#include "VoxelInvalidationCallstack.h"

IVoxelSculptHeightDataSource::IVoxelSculptHeightDataSource(AVoxelSculptHeight& SculptActor)
	: WeakSculptActor(SculptActor)
	, PrivateData(MakeShared<FVoxelSculptHeightData>())
{
}

TVoxelBulkRef<FVoxelSculptHeightData> IVoxelSculptHeightDataSource::GetData() const
{
	return PrivateData;
}

bool IVoxelSculptHeightDataSource::IsEditor() const
{
	const AVoxelSculptHeight* SculptActor = WeakSculptActor.Resolve();
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

TVoxelOptional<FVoxelSculptHeightContext> IVoxelSculptHeightDataSource::GetContext() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const AVoxelSculptHeight* SculptActor = WeakSculptActor.Resolve();
	if (!ensureVoxelSlow(SculptActor))
	{
		return {};
	}

	return SculptActor->GetSculptContext();
}

void IVoxelSculptHeightDataSource::SetData(const TVoxelBulkRef<FVoxelSculptHeightData>& Data)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread() || IsInAsyncLoadingThread());

	if (PrivateData.GetHash() == Data.GetHash())
	{
		return;
	}
	const TSharedRef<const FVoxelAABBTree2D> InvalidationTree = FVoxelSculptHeightInvalidationBuilder::Create(
		PrivateData->GetKeyToFarChunk(),
		Data->GetKeyToFarChunk());

	PrivateData = Data;

	if (!IsInGameThread())
	{
		return;
	}

	const AVoxelSculptHeight* SculptActor = WeakSculptActor.Resolve();
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

	// Invalidate the entire bounds since we're replacing the whole data
	SculptActor->GetDependency()->Invalidate(InvalidationTree);

	const TSharedPtr<const FVoxelStampRuntime> StampRuntime = SculptActor->GetStamp()->ResolveStampRuntime();
	if (!StampRuntime)
	{
		return;
	}

	StampRuntime->RequestUpdate();

	// TODO: 
	/*extern bool GVoxelStampShowInvalidations;

	if (GVoxelStampShowInvalidations)
	{
		FVoxelDebugDrawer Drawer(StampRuntime->GetWorld());

		InvalidationTree->DrawLeaves(
			Drawer.Color(FLinearColor::Red).LifeTime(1.f),
			FScaleMatrix(StampRuntime->GetStamp().AsChecked<FVoxelHeightSculptStamp>().ScaleXY) *
			StampRuntime->GetLocalToWorld().ToMatrixWithScale());
	}*/
}