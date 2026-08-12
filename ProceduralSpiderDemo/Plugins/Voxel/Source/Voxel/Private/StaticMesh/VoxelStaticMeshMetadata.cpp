// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "StaticMesh/VoxelStaticMeshMetadata.h"
#include "VoxelBuffer.h"
#include "UObject/CoreRedirects.h"

bool FVoxelStaticMeshMetadataData::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	SerializeVoxelVersion(Ar);

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion
	);

	int32 Version = FVersion::LatestVersion;
	Ar << Version;
	ensure(Version == FVersion::LatestVersion);

	FVoxelSerializationGuard Guard(Ar);

	FVoxelBuffer::Serialize(Ar, Buffer);
	return true;
}