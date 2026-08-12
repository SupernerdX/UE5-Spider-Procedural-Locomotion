// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.h"
#include "Interfaces/IPluginManager.h"

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_GAME()
{
	if (!FVoxelUtilities::IsDevWorkflow())
	{
		return;
	}

	const FString SourcePath = FVoxelUtilities::GetPlugin().GetBaseDir() / "Source";

	TVoxelSet<FString> VisitedPaths;

	IFileManager::Get().IterateDirectoryRecursively(*(SourcePath / "VoxelCore" / "Public" / "VoxelMinimal"), [&](const TCHAR* InPath, const bool bIsDirectory)
	{
		if (bIsDirectory)
		{
			return true;
		}

		FString Name = InPath;
		ensure(Name.RemoveFromStart(SourcePath / "VoxelCore" / "Public" / "VoxelMinimal/"));

		if (!Name.RemoveFromEnd(".h"))
		{
			ensure(Name.EndsWith(".isph"));
			return true;
		}

		FString File;
		File += "// Copyright Voxel Plugin SAS. All Rights Reserved.\n";
		File += "\n";
		File += "#include \"VoxelCoreMinimal.h\"\n";
		File += "#if VOXEL_DEV_WORKFLOW && VOXEL_DEBUG\n";
		File += "#include \"VoxelMinimal/" + Name + ".h\"\n";
		File += "#endif";

		Name.ReplaceCharInline(TEXT('/'), TEXT('_'));

		const FString FilePath = SourcePath / "VoxelTests" / "Private" / "VoxelCoreIncludeTest" / "VoxelCoreIncludeTest_" + Name + ".cpp";
		VisitedPaths.Add(FilePath);

		FString ExistingFile;
		FFileHelper::LoadFileToString(ExistingFile, *FilePath);

		// Normalize line endings
		ExistingFile.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

		if (!ExistingFile.Equals(File))
		{
			IFileManager::Get().Delete(*FilePath, false, true);
			ensure(FFileHelper::SaveStringToFile(File, *FilePath));
			LOG_VOXEL(Error, "%s written", *FilePath);
		}

		return true;
	});

	IFileManager::Get().IterateDirectoryRecursively(*(SourcePath / "VoxelTests" / "Private" / "VoxelCoreIncludeTest"), [&](const TCHAR* Path, const bool bIsDirectory)
	{
		if (ensure(VisitedPaths.Contains(Path)))
		{
			return true;
		}

		ensure(IFileManager::Get().Delete(Path));
		LOG_VOXEL(Error, "%s deleted", Path);
		return true;
	});
}
#endif