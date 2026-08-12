// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightSculptBlueprintLibrary.h"
#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Sculpt/Height/VoxelSculptHeightAsset.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"

bool UVoxelHeightSculptBlueprintLibrary::IsValidSave(FVoxelHeightSculptSave Save)
{
	return Save.IsValid();
}

bool UVoxelHeightSculptBlueprintLibrary::IsCompressedSave(FVoxelHeightSculptSave Save)
{
	if (!Save.IsValid())
	{
		VOXEL_MESSAGE(Error, "Save is invalid");
		return false;
	}

	return Save.IsCompressed();
}

int64 UVoxelHeightSculptBlueprintLibrary::GetSaveSize(FVoxelHeightSculptSave Save)
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

void UVoxelHeightSculptBlueprintLibrary::ClearSculptData(AVoxelSculptHeight* SculptActor)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is invalid");
		return;
	}

	SculptActor->SetSculptData(MakeVoxelBulkRef(MakeShared<FVoxelSculptHeightData>()));
}

void UVoxelHeightSculptBlueprintLibrary::ClearSculptCache(AVoxelSculptHeight* SculptActor)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is invalid");
		return;
	}

	SculptActor->ClearSculptCache();
}

void UVoxelHeightSculptBlueprintLibrary::SetSculptAsset(
	AVoxelSculptHeight* SculptActor,
	UVoxelSculptHeightAsset* Asset)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is invalid");
		return;
	}

	if (!Asset)
	{
		SculptActor->SetSculptData(MakeVoxelBulkRef(MakeShared<FVoxelSculptHeightData>()));
	}
	else
	{
		SculptActor->SetSculptData(Asset->GetData(), Asset->GetBulkLoader());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFuture UVoxelHeightSculptBlueprintLibrary::K2_GetSave(
	FVoxelHeightSculptSave& Save,
	AVoxelSculptHeight* SculptActor,
	const bool bCompress)
{
	VOXEL_FUNCTION_COUNTER();

	Save = {};

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is null");
		return {};
	}

	return SculptActor->GetSave(bCompress).Then_GameThread([&Save](const TSharedRef<FVoxelHeightSculptSave>& NewSave)
	{
		Save = *NewSave;
	});
}

FVoxelFuture UVoxelHeightSculptBlueprintLibrary::LoadFromSave(
	AVoxelSculptHeight* SculptActor,
	const FVoxelHeightSculptSave Save)
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

FVoxelFuture UVoxelHeightSculptBlueprintLibrary::ApplyModifier(
	AVoxelSculptHeight* SculptActor,
	const TSharedRef<FVoxelHeightModifier>& Modifier)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SculptActor)
	{
		VOXEL_MESSAGE(Error, "SculptActor is invalid");
		return {};
	}

	return SculptActor->ApplyModifier(Modifier);
}