// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Misc/TextFilter.h"

class SVoxelConsoleCommandsTab;

class FVoxelConsoleCommandsTabManager : public FVoxelSingleton
{
public:
	const FName GlobalConsoleCommandsTabId = "VoxelConsoleCommands";

	FVoxelConsoleCommandsTabManager() = default;

	//~ Begin FVoxelSingleton Interface
	virtual void Initialize() override;
	//~ End FVoxelSingleton Interface

	void OpenGlobalCommands() const;
};
extern FVoxelConsoleCommandsTabManager* GVoxelCommandsTabManager;

struct FVoxelConsoleCommandItem
{
	FText DisplayName;
	FText ToolTip;
	FName FullCommand;
	IConsoleObject* Object;
	TArray<TSharedPtr<FVoxelConsoleCommandItem>> Children;
};

class SVoxelConsoleCommandRow : public SMultiColumnTableRow<TSharedPtr<FVoxelConsoleCommandItem>>
{
public:
	VOXEL_SLATE_ARGS()
	{
		SLATE_ATTRIBUTE(FText, HighlightText)
		SLATE_ATTRIBUTE(bool, IsFavorite)
		SLATE_EVENT(FSimpleDelegate, OnFavoriteStateChanged)
	};

	void Construct(
		const FArguments& Args,
		const TSharedRef<STableViewBase>& OwnerTableView,
		const TSharedPtr<FVoxelConsoleCommandItem>& InItem);

	//~ Begin STableRow Interface
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	//~ End STableRow Interface

private:
	TSharedPtr<FVoxelConsoleCommandItem> Item;
	TSharedPtr<SEditableTextBox> EditableTextBox;
	TAttribute<FText> HighlightText;
	TAttribute<bool> IsFavorite;
	FSimpleDelegate OnFavoriteStateChanged;
};

class SVoxelConsoleCommandsTab : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(const FArguments& InArgs);

private:
	void CollectCommands();
	void UpdateFilteredCommands();
	void UpdateFavorites();
	void UpdateFavoritesCategory();

private:
	TSharedPtr<SSearchBox> SearchTextField;
	TSharedPtr<STreeView<TSharedPtr<FVoxelConsoleCommandItem>>> TreeView;

	TArray<TSharedPtr<FVoxelConsoleCommandItem>> RootItems;
	TArray<TSharedPtr<FVoxelConsoleCommandItem>> AllRootItems;
	TArray<TSharedPtr<FVoxelConsoleCommandItem>> AllFlattenedItems;

	TSharedPtr<FVoxelConsoleCommandItem> FavoriteCommandsCategory;

	using FConsoleCommandTextFilter = TTextFilter<TSharedPtr<FVoxelConsoleCommandItem>>;
	TSharedPtr<FConsoleCommandTextFilter> Filter;

	FText HighlightText;

	TSet<FName> FavoriteCommands;
};