// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/VoxelSculptSave.h"

bool FVoxelSculptSaveBase::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	SerializeVoxelVersion(Ar);

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	bool bIsValid = Data.IsValid();
	Ar << bIsValid;

	if (!bIsValid)
	{
		Data.Reset();
		return true;
	}

	if (Ar.IsLoading())
	{
		Data = MakeShared<FData>();
	}

	Ar << Data->bIsCompressed;
	Ar << Data->Data;
	return true;
}

bool FVoxelSculptSaveBase::Identical(const FVoxelSculptSaveBase* Other, uint32 PortFlags) const
{
	return
		Other &&
		Other->Data == Data;
}