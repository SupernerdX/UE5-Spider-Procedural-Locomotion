// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.h"
#include "Interfaces/IPluginManager.h"

VOXEL_DEFAULT_MODULE(VoxelTests);

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_GAME()
{
	if (!FParse::Param(FCommandLine::Get(), TEXT("VoxelTests")) &&
		!FParse::Param(FCommandLine::Get(), TEXT("RunVoxelTests")))
	{
		return;
	}

	const FString DiskPath = FPaths::ConvertRelativePathToFull(FVoxelUtilities::GetPlugin().GetBaseDir() / "Tests");

	LOG_VOXEL(Display, "Mounting %s to /Game/VoxelTests", *DiskPath)

	const FString ContentPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / "VoxelTests");
	if (FPaths::DirectoryExists(ContentPath))
	{
		LOG_VOXEL(Error, "Cannot mount: %s exists", *ContentPath)
		return;
	}

	FPackageName::RegisterMountPoint(
		"/Game/VoxelTests/",
		DiskPath);
}
#endif