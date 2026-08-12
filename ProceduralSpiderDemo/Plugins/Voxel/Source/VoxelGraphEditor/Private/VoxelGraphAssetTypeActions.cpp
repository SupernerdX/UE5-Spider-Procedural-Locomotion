// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraph.h"
#include "VoxelTerminalGraph.h"
#include "VoxelAssetTypeActions.h"
#include "Styling/SlateIconFinder.h"

class FVoxelGraphAssetTypeActions : public FVoxelInstanceAssetTypeActions
{
public:
	FVoxelGraphAssetTypeActions(UClass* Class)
		: ParentClass(Class)
	{
		
	}

	//~ Begin FVoxelInstanceAssetTypeActions Interface
	virtual UClass* GetInstanceClass() const override
	{
		return ParentClass;
	}
	virtual FSlateIcon GetInstanceActionIcon() const override
	{
		return FSlateIconFinder::FindIconForClass(ParentClass);
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
	//~ End FVoxelInstanceAssetTypeActions Interface

private:
	UClass* ParentClass;
};

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	for (const TSubclassOf<UVoxelGraph>& Class : GetDerivedClasses<UVoxelGraph>())
	{
		if (Class->HasMetaData("CustomAssetTypeActions"))
		{
			continue;
		}

		FVoxelAssetTypeActions::Register(
			Class,
			MakeShared<FVoxelGraphAssetTypeActions>(Class));
	}
}