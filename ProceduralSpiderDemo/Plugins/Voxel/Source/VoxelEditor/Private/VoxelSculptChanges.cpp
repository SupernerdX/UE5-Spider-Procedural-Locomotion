// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelSculptChanges.h"
#include "VoxelStampComponent.h"
#include "Sculpt/Volume/VoxelSculptVolume.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"

FString FVoxelHeightSculptChange::ToString() const
{
	return "Sculpt height";
}

TUniquePtr<FChange> FVoxelHeightSculptChange::Execute(UObject* Object)
{
	VOXEL_FUNCTION_COUNTER();

	AVoxelSculptHeight* SculptActor = Cast<AVoxelSculptHeight>(Object);
	if (!ensure(SculptActor))
	{
		return MakeUnique<FVoxelHeightSculptChange>(MakeVoxelBulkRef(MakeShared<FVoxelSculptHeightData>()));
	}

	TUniquePtr<FVoxelHeightSculptChange> Result = MakeUnique<FVoxelHeightSculptChange>(SculptActor->GetSculptData());
	SculptActor->SetSculptData(Snapshot);
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FString FVoxelVolumeSculptChange::ToString() const
{
	return "Sculpt volume";
}

TUniquePtr<FChange> FVoxelVolumeSculptChange::Execute(UObject* Object)
{
	VOXEL_FUNCTION_COUNTER();

	AVoxelSculptVolume* SculptActor = Cast<AVoxelSculptVolume>(Object);
	if (!ensure(SculptActor))
	{
		return MakeUnique<FVoxelVolumeSculptChange>(MakeVoxelBulkRef(MakeShared<FVoxelSculptVolumeData>()));
	}

	TUniquePtr<FVoxelVolumeSculptChange> Result = MakeUnique<FVoxelVolumeSculptChange>(SculptActor->GetSculptData());
	SculptActor->SetSculptData(Snapshot);
	return Result;
}