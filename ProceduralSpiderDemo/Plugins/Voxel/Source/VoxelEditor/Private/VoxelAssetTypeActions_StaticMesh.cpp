// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "ContentBrowserModule.h"
#include "StaticMesh/VoxelStaticMesh.h"

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

			if (!Class->IsChildOf<UStaticMesh>())
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
					INVTEXT("Create Voxel Static Mesh Asset"),
					INVTEXT("Creates a new Voxel Static Mesh Asset from this mesh"),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "ShowFlagsMenu.BSP"),
					FUIAction(FExecuteAction::CreateLambda([=]
					{
						TArray<UObject*> ObjectsToSync;
						for (const FAssetData& Asset : Assets)
						{
							UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset.GetAsset());
							if (!ensureVoxelSlow(StaticMesh))
							{
								continue;
							}

							UVoxelStaticMesh* VoxelStaticMesh = FVoxelUtilities::CreateNewAsset_Direct<UVoxelStaticMesh>(StaticMesh, { "SM_" }, "VSM_", {});
							if (!VoxelStaticMesh)
							{
								continue;
							}

							VoxelStaticMesh->Mesh = StaticMesh;
							VoxelStaticMesh->PostEditChange();

							ObjectsToSync.Add(VoxelStaticMesh);
						}

						GEditor->SyncBrowserToObjects(ObjectsToSync);
					})));
			})
		);

		return Extender;
	}));
}