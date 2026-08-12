// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelAssetIcon.h"
#include "VoxelDeveloperSettings.h"
#include "VoxelEditorSettings.generated.h"

UCLASS(config=EditorPerProjectUserSettings)
class VOXELEDITOR_API UVoxelEditorSettings : public UVoxelDeveloperSettings
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelDeveloperSettings Interface
	virtual FName GetContainerName() const override
	{
		return "Editor";
	}
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UVoxelDeveloperSettings Interface

public:
	UPROPERTY(Config, EditAnywhere, Category = "Config")
	TArray<FLinearColor> DefaultLayerColors = { FVoxelAssetIcon().Color };

	UPROPERTY(Config, EditAnywhere, Category = "Config")
	TArray<FLinearColor> DefaultStackColors = { FVoxelAssetIcon().Color };

	UPROPERTY(Config, EditAnywhere, Category = "UI")
	bool bEnablePlaceStampsDrawer = true;

	UPROPERTY(Config, EditAnywhere, Category = "UI")
	bool bEnableContentBrowserActions = true;

	UPROPERTY(Config, EditAnywhere, Category = "UI")
	bool bEnableToolbarActions = true;

	UPROPERTY(Config, EditAnywhere, Category = "UI")
	bool bEnableViewportContextMenuActions = true;

	UPROPERTY(Config, EditAnywhere, Category = "Examples", meta = (Units = "Megabytes"))
	double ExamplesCacheSize = 1024;

public:
	static FLinearColor GetRandomLayerColor();
	static FLinearColor GetRandomStackColor();
};