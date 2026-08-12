// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStamp.h"
#include "VoxelGraph.h"
#include "VoxelStampRef.h"
#include "VoxelStampRuntime.h"
#include "VoxelTerminalGraph.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelStamp);

void FVoxelStamp::FixupProperties()
{
	MetadataOverrides.Fixup();

	if (StampSeed.Seed.IsEmpty())
	{
		StampSeed.Randomize();
	}
}

#if WITH_EDITOR
FString FVoxelStamp::GetIdentifier() const
{
	const UObject* Asset = GetAsset();
	if (!Asset)
	{
		return {};
	}

	if (const UVoxelGraph* Graph = Cast<UVoxelGraph>(Asset))
	{
		return Graph->GetMetadata().DisplayName;
	}

	return Asset->GetName();
}
#endif

FVoxelStampRef FVoxelStamp::GetStampRef() const
{
	const FVoxelStampRef StampRef = WeakStampRef.Pin();
	if (!StampRef ||
		// Stamp was changed in-between
		StampRef.GetStampData() != this)
	{
		return {};
	}

	return StampRef;
}

TVoxelObjectPtr<USceneComponent> FVoxelStamp::GetComponent() const
{
	const TSharedPtr<const FVoxelStampRuntime> Runtime = ResolveStampRuntime();
	if (!Runtime)
	{
		return {};
	}

	return Runtime->GetComponent();
}

TSharedPtr<const FVoxelStampRuntime> FVoxelStamp::ResolveStampRuntime() const
{
	const FVoxelStampRef StampRef = GetStampRef();
	if (!StampRef)
	{
		return {};
	}

	const TSharedPtr<const FVoxelStampRuntime> Runtime = StampRef.ResolveStampRuntime();
	ensure(Runtime || !StampRef.IsRegistered());
	return Runtime;
}