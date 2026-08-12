// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class UVoxelGraph;
struct FVoxelGraphTile;

class VOXELGRAPHEDITOR_API SVoxelNewGraphAssetDialog : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
	};

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> ConstructCategories();

private:
	TSharedPtr<STextBlock> DescriptionWidget;
	TMap<FName, TSharedPtr<STileView<TSharedPtr<FVoxelGraphTile>>>> CategoryToTilesView;
	TMap<FName, TArray<TSharedPtr<FVoxelGraphTile>>> CategoryToGraphsList;
	TWeakPtr<SWindow> WeakParentWindow;

public:
	bool bCreateAsset = false;
	TWeakObjectPtr<UVoxelGraph> SelectedGraph;

	TSharedPtr<FVoxelGraphTile> SelectedTile;
};