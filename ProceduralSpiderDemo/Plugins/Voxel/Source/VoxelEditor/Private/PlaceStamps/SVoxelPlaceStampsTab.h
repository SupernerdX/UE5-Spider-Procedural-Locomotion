// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Misc/TextFilter.h"

struct FVoxelPlaceStampsItem;

class SVoxelPlaceStampsTab : public SCompoundWidget
{
public:
	using FContentSourceTextFilter = TTextFilter<TSharedPtr<FVoxelPlaceStampsItem>>;
	using FCollectItemsFunction = TDelegate<void(TArray<TSharedPtr<FVoxelPlaceStampsItem>>&)>;

public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(const FArguments& InArgs, const TSharedPtr<SDockTab>& ParentTab);
	void FocusSearchBox() const;

	//~ Begin SCompoundWidget Interface
	virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
	//~ End SCompoundWidget Interface

private:
	TSharedRef<SWidget> CreateCategoriesWidget();
	void SetActiveTab(FName Name);
	void UpdateContentForCategory(FName CategoryName);
	void UpdateTags(bool bResetVisibleTags);
	void UpdateFilteredItems();
	void UpdateFavorites();

private:
	void OnAssetsAdded(TArrayView<const FAssetData> AssetDatas);
	void OnAssetRenamed(const FAssetData& AssetData, const FString& OldName);
	void OnAssetUpdated(const FAssetData& AssetData);
	void OnAssetRemoved(const FAssetData& AssetData);

private:
	static TSharedPtr<FVoxelPlaceStampsItem> ConstructGraph(const FAssetData& AssetData);
	static TSharedPtr<FVoxelPlaceStampsItem> ConstructMesh(const FAssetData& AssetData);
	static TSharedPtr<FVoxelPlaceStampsItem> ConstructHeightmap(const FAssetData& AssetData);
	static TSharedPtr<FVoxelPlaceStampsItem> ConstructSculpt(const FAssetData& AssetData);
	static TSharedPtr<FVoxelPlaceStampsItem> ConstructShape(const FAssetData& AssetData);

private:
	void FillFavoritesList();
	void CollectAllFavorites(TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems);
	bool UseSystemTags() const;
	static void CollectAllRecent(TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems);

public:
	static FLinearColor GetStampTagColor(FName StampTag);

private:
	TSharedPtr<SSearchBox> SearchBox;
	TSharedPtr<STextBlock> MessageTextBlock;
	TSharedPtr<SWrapBox> TagsBox;
	TSharedPtr<SWidgetSwitcher> WidgetSwitcher;
	TSharedPtr<STileView<TSharedPtr<FVoxelPlaceStampsItem>>> TileView;

	TSharedPtr<SSplitter> ContentSplitter;
	TSharedPtr<IStructureDetailsView> DetailsView;

	TSet<FSoftObjectPath> Favorites;

private:
	TArray<TSharedPtr<FVoxelPlaceStampsItem>> AllCategoryItems;
	TArray<TSharedPtr<FVoxelPlaceStampsItem>> FilteredCategoryItems;
	TMap<FSoftObjectPath, TSharedPtr<FVoxelPlaceStampsItem>> TrackedAssetToItem;

	TSharedPtr<FContentSourceTextFilter> Filter;

private:
	TMap<FName, FCollectItemsFunction> CategoryToCollectFunction;
	FName ActiveTabName;
	TSet<FName> Tags;
	TSet<FName> VisibleTags;
};