// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "Materials/Material.h"
#include "VoxelAssetTypeActions.h"
#include "Styling/SlateIconFinder.h"
#include "Surface/VoxelSurfaceTypeAsset.h"

class FVoxelSurfaceTypeAssetTypeActions : public FVoxelAssetTypeActions
{
public:
	//~ Begin FVoxelAssetTypeActions Interface
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override
	{
		FVoxelAssetTypeActions::GetActions(InObjects, MenuBuilder);

		MenuBuilder.AddMenuEntry(
			INVTEXT("Open Surface Type Material"),
			INVTEXT("Opens assigned surface type material"),
			FSlateIconFinder::FindIconForClass(UMaterial::StaticClass()),
			FUIAction(MakeWeakPtrDelegate(this, [this, InObjects]
			{
				UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
				if (!AssetEditorSubsystem)
				{
					return;
				}

				TArray<UObject*> ObjectsToSync;
				for (UObject* Object : InObjects)
				{
					if (!ensure(Object))
					{
						continue;
					}

					const UVoxelSurfaceTypeAsset* SurfaceTypeAsset = Cast<UVoxelSurfaceTypeAsset>(Object);
					if (!ensure(SurfaceTypeAsset))
					{
						continue;
					}

					FText ErrorMsg;
					if (AssetEditorSubsystem->CanOpenEditorForAsset(SurfaceTypeAsset->Material, EAssetTypeActivationOpenedMethod::Edit, &ErrorMsg))
					{
						AssetEditorSubsystem->OpenEditorForAsset(SurfaceTypeAsset->Material);
					}
				}
			})));
	}
	//~ End FVoxelAssetTypeActions Interface
};

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FVoxelAssetTypeActions::Register(
		UVoxelSurfaceTypeAsset::StaticClass(),
		MakeShared<FVoxelSurfaceTypeAssetTypeActions>());
}