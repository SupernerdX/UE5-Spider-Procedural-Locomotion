// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "VoxelGraph.h"
#include "VoxelTerminalGraph.h"
#include "VoxelAssetTypeActions.h"
#include "Styling/SlateIconFinder.h"
#include "Surface/VoxelSmartSurfaceType.h"
#include "Surface/VoxelSmartSurfaceTypeGraph.h"

class FVoxelSmartSurfaceTypeGraphAssetTypeActions : public FVoxelInstanceAssetTypeActions
{
public:
	//~ Begin FVoxelInstanceAssetTypeActions Interface
	virtual UClass* GetInstanceClass() const override
	{
		return UVoxelSmartSurfaceTypeGraph::StaticClass();
	}
	virtual FSlateIcon GetInstanceActionIcon() const override
	{
		return FSlateIconFinder::FindIconForClass(UVoxelSmartSurfaceTypeGraph::StaticClass());
	}
	virtual void SetParent(UObject* InstanceAsset, UObject* ParentAsset) const override
	{
		UVoxelGraph* InstanceGraph = CastChecked<UVoxelGraph>(InstanceAsset);
		InstanceGraph->SetBaseGraph(CastChecked<UVoxelGraph>(ParentAsset));

		UVoxelTerminalGraph& MainTerminalGraph = InstanceGraph->GetMainTerminalGraph();
		MainTerminalGraph.Modify();
		InstanceGraph->RemoveTerminalGraph(GVoxelMainTerminalGraphGuid);
		MainTerminalGraph.MarkAsGarbage();
	}
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override
	{
		FVoxelInstanceAssetTypeActions::GetActions(InObjects, MenuBuilder);

		MenuBuilder.AddMenuEntry(
			INVTEXT("Create Smart Surface Type"),
			INVTEXT("Creates a new Smart Surface Type using this graph"),
			FSlateIconFinder::FindIconForClass(UVoxelSmartSurfaceType::StaticClass()),
			FUIAction(MakeWeakPtrDelegate(this, [this, InObjects]
			{
				IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

				TArray<UObject*> ObjectsToSync;
				for (UObject* GraphAsset : InObjects)
				{
					if (!ensure(GraphAsset))
					{
						continue;
					}

					UVoxelSmartSurfaceType* SmartSurfaceTypeAsset = FVoxelUtilities::CreateNewAsset_Direct<UVoxelSmartSurfaceType>(GraphAsset, { "VSG_", "VG_", "VSTG_" }, "VSST_", {});
					if (!ensure(SmartSurfaceTypeAsset))
					{
						continue;
					}

					SmartSurfaceTypeAsset->Graph = Cast<UVoxelSmartSurfaceTypeGraph>(GraphAsset);
					SmartSurfaceTypeAsset->PostEditChange();

					ObjectsToSync.Add(SmartSurfaceTypeAsset);
				}

				if (ObjectsToSync.Num() == 0)
				{
					return;
				}

				IContentBrowserSingleton& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
				ContentBrowserModule.SyncBrowserToAssets(ObjectsToSync);
			})));
	}
	//~ End FVoxelInstanceAssetTypeActions Interface
};

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FVoxelAssetTypeActions::Register(
		UVoxelSmartSurfaceTypeGraph::StaticClass(),
		MakeShared<FVoxelSmartSurfaceTypeGraphAssetTypeActions>());
}