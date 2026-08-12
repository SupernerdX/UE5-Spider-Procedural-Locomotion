// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "ContentBrowserModule.h"
#include "Surface/VoxelSurfaceTypeAsset.h"

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	CBMenuExtenderDelegates.Add(MakeLambdaDelegate([](const TArray<FAssetData>& Assets)
	{
		TSharedRef<FExtender> Extender = MakeShared<FExtender>();

		for (const FAssetData& Asset : Assets)
		{
			const UClass* Class = Asset.GetClass();
			if (!ensureVoxelSlow(Class))
			{
				continue;
			}

			if (!Class->IsChildOf<UMaterialInterface>())
			{
				return Extender;
			}
		}

		Extender->AddMenuExtension(
			"GetAssetActions",
			EExtensionHook::After,
			nullptr,
			MakeLambdaDelegate([Assets](FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddMenuEntry(
					INVTEXT("Create Voxel Surface"),
					INVTEXT("Creates a new Voxel Surface Asset from the selected material"),
					FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "ClassIcon.VoxelSurfaceAsset"),
					FUIAction(FExecuteAction::CreateLambda([=]
					{
						TArray<UMaterialInterface*> Materials;
						for (const FAssetData& Asset : Assets)
						{
							UMaterialInterface* Material = Cast<UMaterialInterface>(Asset.GetAsset());
							if (!Material)
							{
								continue;
							}

							Materials.Add(Material);
						}

						if (Materials.Num() == 0)
						{
							return;
						}

						TArray<UObject*> ObjectsToSync;

						for (UMaterialInterface* Material : Materials)
						{
							FString PackageName = FPackageName::ObjectPathToPackageName(Material->GetPathName());
							if (!PackageName.StartsWith("/Game/"))
							{
								// Don't create assets in the engine
								PackageName = "/Game/VoxelMaterials/" + Material->GetName();
							}

							UVoxelSurfaceTypeAsset* Asset = FVoxelUtilities::CreateNewAsset_Direct<UVoxelSurfaceTypeAsset>(PackageName, { "M_", "MI_" }, "VST_", {});
							if (!Asset)
							{
								continue;
							}

							Asset->Material = Material;
							ObjectsToSync.Add(Asset);
						}
						GEditor->SyncBrowserToObjects(ObjectsToSync);
					})));
			})
		);

		return Extender;
	}));
}