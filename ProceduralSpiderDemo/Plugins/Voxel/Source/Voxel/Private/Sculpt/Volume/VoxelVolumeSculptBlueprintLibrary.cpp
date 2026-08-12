// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeSculptBlueprintLibrary.h"
#include "VoxelSculptVolumeCache.h"
#include "Sculpt/Volume/VoxelSculptVolume.h"
#include "Sculpt/Volume/VoxelSculptVolumeAsset.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"

bool UVoxelVolumeSculptBlueprintLibrary::IsValidSave(FVoxelVolumeSculptSave Save)
{
	return Save.IsValid();
}

bool UVoxelVolumeSculptBlueprintLibrary::IsCompressedSave(FVoxelVolumeSculptSave Save)
{
	if (!Save.IsValid())
	{
		VOXEL_MESSAGE(Error, "Save is invalid");
		return false;
	}

	return Save.IsCompressed();
}

int64 UVoxelVolumeSculptBlueprintLibrary::GetSaveSize(FVoxelVolumeSculptSave Save)
{
	if (!Save.IsValid())
	{
		VOXEL_MESSAGE(Error, "Save is invalid");
		return false;
	}

	return Save.GetSize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelVolumeSculptBlueprintLibrary::ClearSculptData(AVoxelSculptVolume* SculptActor)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is invalid");
		return;
	}

	SculptActor->SetSculptData(MakeVoxelBulkRef(MakeShared<FVoxelSculptVolumeData>()));
}

void UVoxelVolumeSculptBlueprintLibrary::ClearSculptCache(AVoxelSculptVolume* SculptActor)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is invalid");
		return;
	}

	SculptActor->ClearSculptCache();
}

void UVoxelVolumeSculptBlueprintLibrary::SetSculptAsset(
	AVoxelSculptVolume* SculptActor,
	UVoxelSculptVolumeAsset* Asset)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is invalid");
		return;
	}

	if (!Asset)
	{
		SculptActor->SetSculptData(MakeVoxelBulkRef(MakeShared<FVoxelSculptVolumeData>()));
	}
	else
	{
		SculptActor->SetSculptData(Asset->GetData(), Asset->GetBulkLoader());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFuture UVoxelVolumeSculptBlueprintLibrary::K2_GetSave(
	FVoxelVolumeSculptSave& Save,
	AVoxelSculptVolume* SculptActor,
	const bool bCompress)
{
	VOXEL_FUNCTION_COUNTER();

	Save = {};

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is null");
		return {};
	}

	return SculptActor->GetSave(bCompress).Then_GameThread([&Save](const TSharedRef<FVoxelVolumeSculptSave>& NewSave)
	{
		Save = *NewSave;
	});
}

FVoxelFuture UVoxelVolumeSculptBlueprintLibrary::LoadFromSave(
	AVoxelSculptVolume* SculptActor,
	const FVoxelVolumeSculptSave Save)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is null");
		return {};
	}

	return SculptActor->LoadFromSave(Save);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFuture UVoxelVolumeSculptBlueprintLibrary::ApplyModifier(
	AVoxelSculptVolume* SculptActor,
	const TSharedRef<FVoxelVolumeModifier>& Modifier)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is invalid");
		return {};
	}

	return SculptActor->ApplyModifier(Modifier);
}