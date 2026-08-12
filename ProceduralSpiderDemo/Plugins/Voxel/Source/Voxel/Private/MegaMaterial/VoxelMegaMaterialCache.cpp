// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "MegaMaterial/VoxelMegaMaterialCache.h"
#include "VoxelSettings.h"
#include "MegaMaterial/VoxelMegaMaterial.h"
#include "MegaMaterial/VoxelMegaMaterialGeneratedData.h"
#include "UObject/SavePackage.h"
#include "HAL/PlatformFileManager.h"
#include "MegaMaterial/VoxelMegaMaterialSwitch.h"

#if WITH_EDITOR
UVoxelMegaMaterialGeneratedData& UVoxelMegaMaterialCache::GetGeneratedData(UVoxelMegaMaterial& MegaMaterial)
{
	VOXEL_FUNCTION_COUNTER();

	if (MegaMaterial.bEditorOnly)
	{
		UVoxelMegaMaterialGeneratedData* GeneratedData = NewObject<UVoxelMegaMaterialGeneratedData>();
		GeneratedData->SetMegaMaterial(&MegaMaterial);
		return *GeneratedData;
	}

	UVoxelMegaMaterialCache* Cache = nullptr;
	ForEachAssetOfClass<UVoxelMegaMaterialCache>([&](UVoxelMegaMaterialCache& Asset)
	{
		Cache = &Asset;
	});

	if (Cache)
	{
		LOG_VOXEL(Log, "Found MegaMaterialCache %s", *Cache->GetPathName());
	}
	else
	{
		const FString DevelopersFolderPath = FPackageName::FilenameToLongPackageName(FPaths::GameUserDeveloperDir()) / "MegaMaterialCache";
		LOG_VOXEL(Log, "Creating new MegaMaterialCache: %s", *DevelopersFolderPath);

		Cache = FVoxelUtilities::CreateNewAsset_Direct<UVoxelMegaMaterialCache>(DevelopersFolderPath, {}, {}, {});
	}

	if (!ensure(Cache))
	{
		Cache = GetMutableDefault<UVoxelMegaMaterialCache>();
	}

	if (Cache->Version != FVoxelMegaMaterialSwitch::FVersion::LatestVersion)
	{
		LOG_VOXEL(Log, "MegaMaterialCache is out of date, clearing it");

		Cache->GeneratedDatas.Empty();
		Cache->Version = FVoxelMegaMaterialSwitch::FVersion::LatestVersion;
	}

	for (auto It = Cache->GeneratedDatas.CreateIterator(); It; ++It)
	{
		UVoxelMegaMaterialGeneratedData* GeneratedData = *It;
		if (!GeneratedData)
		{
			It.RemoveCurrent();
			continue;
		}

		const UVoxelMegaMaterial* OtherMegaMaterial = GeneratedData->GetMegaMaterial();
		if (!OtherMegaMaterial)
		{
			It.RemoveCurrent();
			continue;
		}

		if (OtherMegaMaterial == &MegaMaterial)
		{
			LOG_VOXEL(Log, "Found generated data for %s in %s", *MegaMaterial.GetPathName(), *Cache->GetPathName());
			return *GeneratedData;
		}
	}

	LOG_VOXEL(Log, "Creating generated data for %s in %s", *MegaMaterial.GetPathName(), *Cache->GetPathName());

	UVoxelMegaMaterialGeneratedData* GeneratedData = NewObject<UVoxelMegaMaterialGeneratedData>(Cache);
	GeneratedData->SetMegaMaterial(&MegaMaterial);

	Cache->GeneratedDatas.Add(GeneratedData);
	(void)Cache->MarkPackageDirty();

	Cache->AutoSaveIfEnabled();

	return *GeneratedData;
}

void UVoxelMegaMaterialCache::AutoSaveIfEnabled() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!bAutoSave ||
		IsRunningCommandlet() ||
		HasAnyFlags(RF_NeedLoad | RF_NeedPostLoad) ||
		GetDefault<UVoxelSettings>()->bDisableMegaMaterialCacheAutoSave)
	{
		return;
	}

	UPackage* Package = GetOutermost();
	if (!ensure(Package) ||
		!Package->IsDirty())
	{
		return;
	}

	Package->FullyLoad();

	if (!ensureVoxelSlow(Package->IsFullyLoaded()))
	{
		return;
	}

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone;

	const FString PackagePathName = Package->GetPathName();
	const FString Filename = FPackageName::LongPackageNameToFilename(PackagePathName, FPackageName::GetAssetPackageExtension());

	const bool bIsReadOnly = IFileManager::Get().IsReadOnly(*Filename);
	if (bIsReadOnly)
	{
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*Filename, false);
	}

	// Use a low level API to not add the cache to source control
	UPackage::Save(Package, nullptr, *Filename, SaveArgs);

	if (bIsReadOnly)
	{
		// Restore read-only state
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*Filename, true);
	}

	LOG_VOXEL(Log, "%s saved", *GetPathName());
}
#endif

void UVoxelMegaMaterialCache::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}