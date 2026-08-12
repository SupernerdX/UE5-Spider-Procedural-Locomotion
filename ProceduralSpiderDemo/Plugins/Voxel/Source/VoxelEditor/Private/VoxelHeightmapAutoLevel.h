// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelHeightmapAutoLevel.generated.h"

USTRUCT()
struct FVoxelHeightmapAutoLevelSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bShowChannelSelector = true;

	UPROPERTY(EditAnywhere, Category = "Config")
	FString Suffix = "_Heightmap";

	UPROPERTY(EditAnywhere, Category = "Config", meta = (EditCondition = "bShowChannelSelector", EditConditionHides))
	EVoxelTextureChannel Channel = EVoxelTextureChannel::R;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bNormalizeValues = true;

	// The percentage of values to ignore when remapping the texture's brightness. You can use this to ignore extreme values. Set to 0 to disable.
	UPROPERTY(EditAnywhere, Category = "Config", meta = (EditCondition = "bNormalizeValues", Units = "%", ClampMin = "0", ClampMax = "100"))
	float DarkCutoffPercentage = 1.f;

	// The percentage of values to ignore when remapping the texture's brightness. You can use this to ignore extreme values. Set to 0 to disable.
	UPROPERTY(EditAnywhere, Category = "Config", meta = (EditCondition = "bNormalizeValues", Units = "%", ClampMin = "0", ClampMax = "100"))
	float BrightCutoffPercentage = 1.f;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bUpdateMidTones = true;
};

class FVoxelHeightmapAutoLevel
{
public:
	static UTexture2D* ConstructNewHeightmap(const UTexture2D* Texture, const FVoxelHeightmapAutoLevelSettings& Settings);

private:
	static void PrepareHeightmap(
		const UTexture2D* Texture,
		const FVoxelHeightmapAutoLevelSettings& Settings,
		ETextureSourceFormat NewTextureFormat,
		TVoxelArray<uint8>& OutValues);
};

class SVoxelHeightmapAutoLevelSettingsDialog : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
		SLATE_ARGUMENT(bool, ShowChannelSelector)
	};

	void Construct(const FArguments& InArgs);
	static bool OpenSettings(bool bShowChannelSelector, FVoxelHeightmapAutoLevelSettings& OutSettings);

private:
	FReply OnCreateClicked();
	void CreateDetailsView(bool bShowChannelSelector);

private:
	TWeakPtr<SWindow> WeakParentWindow;

	TSharedPtr<IStructureDetailsView> DetailsView;
	TSharedPtr<TStructOnScope<FVoxelHeightmapAutoLevelSettings>> StructOnScope;

public:
	bool bConfirm = false;
};