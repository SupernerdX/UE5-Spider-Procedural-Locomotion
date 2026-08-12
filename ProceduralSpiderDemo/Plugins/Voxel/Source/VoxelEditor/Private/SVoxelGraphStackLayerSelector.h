// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelStackLayer.h"

class UVoxelLayerStack;

class SVoxelGraphStackLayerSelector : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		FArguments()
			: _ThumbnailSize(FIntPoint(48, 48))
		{}

		SLATE_ATTRIBUTE(FVoxelStackLayer, StackLayer)
		SLATE_ARGUMENT(FIntPoint, ThumbnailSize)
		SLATE_ARGUMENT(TSharedPtr<FAssetThumbnailPool>, ThumbnailPool)
		SLATE_ARGUMENT(TOptional<EVoxelLayerType>, LayerType)

		SLATE_EVENT(TDelegate<void(FVoxelStackLayer)>, OnUpdateStackLayer)
	};

	void Construct(const FArguments& InArgs);

private:
	void SetStack(UVoxelLayerStack* NewStack);
	void SetLayer(UVoxelLayer* NewLayer);
	FText GetAssetName(const UObject* Asset) const;

	bool CanSelectLayer(const UVoxelLayerStack* Stack, const FAssetData& AssetData) const;

private:
	TAttribute<FVoxelStackLayer> StackLayerAttribute;
	TOptional<EVoxelLayerType> LayerType;

	TDelegate<void(FVoxelStackLayer)> OnUpdateStackLayer;

	TSharedPtr<SComboButton> StackComboButton;
	TSharedPtr<SComboButton> LayerComboButton;
	TSharedPtr<SBorder> ThumbnailBorder;

	TSharedPtr<FAssetThumbnail> StackThumbnail;
	TSharedPtr<FAssetThumbnail> LayerThumbnail;
};