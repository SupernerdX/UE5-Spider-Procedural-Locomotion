// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampGraphViewportInterface.h"
#include "VoxelStampActor.h"
#include "VoxelWorld.h"

void FVoxelStampGraphViewportInterface::SetupPreview(
	UVoxelGraph& Graph,
	const TSharedRef<SVoxelViewport>& Viewport)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelGraphViewportInterface::SetupPreview(Graph, Viewport);

	AVoxelWorld* VoxelWorld = Viewport->SpawnActor<AVoxelWorld>();
	if (!ensure(VoxelWorld))
	{
		return;
	}

	AVoxelStampActor* StampActor = Viewport->SpawnActor<AVoxelStampActor>();
	if (!ensure(StampActor))
	{
		return;
	}

	WeakStampActor = StampActor;

	VoxelWorld->SetupForPreview();

	SetupVoxelWorld(*VoxelWorld);

	StampActor->SetStamp(MakeStampRef(Graph));

	float SkyScale = 2000.f;
	const TOptional<float> BoundsSize = GetBoundsSize(StampActor->GetStamp());
	if (BoundsSize.IsSet())
	{
		SkyScale = FMath::Max(SkyScale, BoundsSize.GetValue() / 100.f);
	}

	Viewport->SetSkyScale(SkyScale);
}

FRotator FVoxelStampGraphViewportInterface::GetInitialViewRotation() const
{
	return FRotator(-45.0f, 45.0f, 0.f);
}

TOptional<float> FVoxelStampGraphViewportInterface::GetInitialViewDistance() const
{
	VOXEL_FUNCTION_COUNTER();

	const AVoxelStampActor* StampActor = WeakStampActor.Resolve();
	if (!ensureVoxelSlow(StampActor))
	{
		return {};
	}

	const TOptional<float> BoundsSize = GetBoundsSize(StampActor->GetStamp());
	if (BoundsSize.IsSet())
	{
		return FMath::Min(BoundsSize.GetValue(), 100000.f);
	}

	return 10000.f;
}

void FVoxelStampGraphViewportInterface::UpdateStamp()
{
	if (AVoxelStampActor* StampActor = WeakStampActor.Resolve())
	{
		StampActor->UpdateStamp();
	}
}