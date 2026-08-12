// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelVersionEditor.h"
#include "VoxelVersion.h"
#include "FileHelpers.h"
#include "UnrealEdMisc.h"
#include "Misc/ScopedSlowTask.h"
#include "ISettingsEditorModule.h"
#include "UObject/CoreRedirects.h"
#include "Interfaces/IPluginManager.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"

VOXEL_CONSOLE_COMMAND(
	"voxel.UpgradeVoxelAssets",
	"Re-save all assets referencing an outdated voxel version")
{
    FVoxelVersionUtilities::UpgradeVoxelAssets();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelVersionUtilities::UpgradeVoxelAssets()
{
	VOXEL_FUNCTION_COUNTER();

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	if (!FPlatformProperties::RequiresCookedData() &&
		!AssetRegistry.IsSearchAllAssets())
	{
		// Force search all assets in standalone
		AssetRegistry.SearchAllAssets(true);
	}

	if (AssetRegistry.IsLoadingAssets())
	{
		AssetRegistry.WaitForCompletion();
		ensure(!AssetRegistry.IsLoadingAssets());
	}

	TVoxelArray<FName> PackageNames;
	AssetRegistry.EnumerateAllPackages([&](const FName PackageName, const FAssetPackageData& PackageData)
	{
		if (!PackageName.ToString().StartsWith("/Game/"))
		{
			return;
		}

		const FStringView ScriptPath = TEXTVIEW("/Script/");

		for (const FName ImportedClass : PackageData.ImportedClasses)
		{
			const FString PackagePath = ImportedClass.ToString();
			if (!PackagePath.StartsWith(ScriptPath))
			{
				continue;
			}

			FString ModuleName = PackagePath.RightChop(ScriptPath.Len());

			int32 Index = 0;
			if (ModuleName.FindChar(TEXT('.'), Index))
			{
				ModuleName = ModuleName.Left(Index);
			}

			const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().GetModuleOwnerPlugin(FName(ModuleName));
			if (Plugin.Get() != &FVoxelUtilities::GetPlugin())
			{
				continue;
			}

			FCoreRedirectObjectName RedirectedName = FCoreRedirects::GetRedirectedName(
				ECoreRedirectFlags::Type_Struct |
				ECoreRedirectFlags::Type_Class,
				FCoreRedirectObjectName(ImportedClass.ToString()));

			UStruct* Struct = FindObject<UStruct>(nullptr, *RedirectedName.ToString());
			if (!Struct)
			{
				LOG_VOXEL(Error, "Failed to load %s referenced by %s",
					*ImportedClass.ToString(),
					*PackageName.ToString());

				continue;
			}

			if (const UScriptStruct* ScriptStruct = Cast<UScriptStruct>(Struct))
			{
				if (!ScriptStruct->UseNativeSerialization())
				{
					// Will not write voxel version, skip
					// If we don't have a native serializer we probably don't have anything to upgrade either
					continue;
				}
			}

			LOG_VOXEL(Display, "%s: Referencing %s",
				*PackageName.ToString(),
				*ImportedClass.ToString());

			PackageNames.Add(PackageName);
			break;
		}
	});

	TArray<UPackage*> PackagesToSave;
	{
		FScopedSlowTask SlowTask(PackageNames.Num(), INVTEXT("Loading packages"));

		for (const FName PackageName : PackageNames)
		{
			SlowTask.EnterProgressFrame(1.f, FText::FromString("Loading " + PackageName.ToString()));

			UPackage* Package = LoadPackage(nullptr, *PackageName.ToString(), LOAD_None);
			if (!Package)
			{
				LOG_VOXEL(Error, "Failed to load %s", *PackageName.ToString());
				continue;
			}

			const int32 Version = Package->GetLinkerCustomVersion(GVoxelCustomVersionGUID);
			if (Version == FVoxelVersion::LatestVersion)
			{
				LOG_VOXEL(Display, "Skipping %s", *PackageName.ToString());
				continue;
			}

			Package->Modify();
			PackagesToSave.Add(Package);
		}
	}

	if (PackagesToSave.Num() == 0)
	{
		VOXEL_MESSAGE(Info, "No package to upgrade");
		return true;
	}

	for (const UPackage* Package : PackagesToSave)
	{
		LOG_VOXEL(Display, "Saving %s", *Package->GetPathName());
	}

	if (IsRunningCommandlet())
	{
		if (UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false))
		{
			return true;
		}
		else
		{
			LOG_VOXEL(Error, "Some packages failed to save");
			return false;
		}
	}
	else
	{
		FEditorFileUtils::FPromptForCheckoutAndSaveParams SaveParams;
		SaveParams.bPromptToSave = true;
		SaveParams.bCanBeDeclined = false;
		SaveParams.Title = INVTEXT("Upgrade Voxel Plugin assets");
		SaveParams.Message = INVTEXT("The following assets need to be upgraded");

		if (FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, SaveParams) != FEditorFileUtils::PR_Success)
		{
			VOXEL_MESSAGE(Error, "Some packages failed to save");
			return false;
		}

		if (EAppReturnType::Yes == FMessageDialog::Open(
			EAppMsgType::YesNo,
			FText::FromString("Restarting is HIGHLY recommended after upgrading voxel assets. Restart now?")))
		{
			FUnrealEdMisc::Get().RestartEditor(false);
		}
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 UUpgradeVoxelAssetsCommandlet::Main(const FString& Params)
{
	const bool bSuccess = FVoxelVersionUtilities::UpgradeVoxelAssets();

	// Ensure any FVoxelUtilities::DelayedCall runs
	FTSTicker::GetCoreTicker().Tick(0.);

	return bSuccess ? 0 : 1;
}