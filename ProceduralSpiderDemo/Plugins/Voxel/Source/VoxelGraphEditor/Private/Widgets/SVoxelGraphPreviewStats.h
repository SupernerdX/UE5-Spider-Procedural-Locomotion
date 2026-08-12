// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class SVoxelGraphPreviewStats : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		FArguments()
			: _Padding(15.f)
			, _BulletPointSize(16.f)
			, _BulletPointVisibility(EVisibility::Visible)
			, _ListViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("ListView"))
			, _RowStyle(&FAppStyle::GetWidgetStyle<FTableRowStyle>("TableView.Row"))
			, _BorderImage(FAppStyle::GetBrush("Brushes.Recessed"))
		{}

		SLATE_ARGUMENT(float, Padding)
		SLATE_ARGUMENT(float, BulletPointSize)
		SLATE_ARGUMENT(EVisibility, BulletPointVisibility)
		SLATE_STYLE_ARGUMENT(FTableViewStyle, ListViewStyle)
		SLATE_STYLE_ARGUMENT(FTableRowStyle, RowStyle)
		SLATE_ATTRIBUTE(const FSlateBrush*, BorderImage)
	};

	struct FRow
	{
		FText Header;
		TAttribute<FText> Tooltip;
		TAttribute<TArray<FString>> Values;
		bool bGlobalSpacing = false;
	};
	TArray<TSharedPtr<FRow>> Rows;
	TSharedPtr<SListView<TSharedPtr<FRow>>> RowsView;


	void Construct(const FArguments& Args);
	virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
	void Reset();

private:
	TArray<float> ValueWidths = { 0.f, 0.f, 0.f, 0.f };
	TArray<TArray<TSharedPtr<SWidget>>> RowValueWidgets;
};