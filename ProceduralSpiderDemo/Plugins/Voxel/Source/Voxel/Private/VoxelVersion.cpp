// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelVersion.h"

FCustomVersionRegistration GRegisterVoxelCustomVersion(
	GVoxelCustomVersionGUID,
	FVoxelVersion::LatestVersion,
	TEXT("VoxelPluginVer"));

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

using FVoxelSerializeVoxelVersion = void(*)(FArchive&);
extern VOXELCORE_API FVoxelSerializeVoxelVersion SerializeVoxelVersionPtr;

int32 GVoxelRegisterSerializeVoxelVersionPtr = []
{
	SerializeVoxelVersionPtr = [](FArchive& Ar)
	{
		Ar.UsingCustomVersion(GVoxelCustomVersionGUID);
	};

	return 0;
}();