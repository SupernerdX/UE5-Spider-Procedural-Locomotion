// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphPreviewStats.h"

VOXEL_INITIALIZE_STYLE(GraphPreviewStats)
{
	FTextBlockStyle NormalText = FAppStyle::GetWidgetStyle<FTextBlockStyle>("NormalText");
	Set("GraphPreviewValues", FTextBlockStyle(NormalText)
		.SetFont(DEFAULT_FONT("Mono", 8))
		.SetShadowOffset(FVector2f::ZeroVector));
}

void SVoxelGraphPreviewStats::Construct(const FArguments& Args)
{
	const TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(Args._BorderImage)
		.Padding(Args._Padding)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SScrollBox)
					.Orientation(Orient_Horizontal)
					+ SScrollBox::Slot()
					[
						SAssignNew(RowsView, SListView<TSharedPtr<FRow>>)
						.ListViewStyle(Args._ListViewStyle)
						.ListItemsSource(&Rows)
						.OnGenerateRow_Lambda([this, Args](TSharedPtr<FRow> StatsRow, const TSharedRef<STableViewBase>& OwnerTable)
						{
							TArray<TSharedPtr<SWidget>> Widgets;
							for (int32 Index = 0; Index < 4; Index++)
							{
								Widgets.Add(
									SNew(STextBlock)
									.Text_Lambda([StatsRow, Index]() -> FText
									{
										TArray<FString> Values = StatsRow->Values.Get();
										if (!Values.IsValidIndex(Index))
										{
											return {};
										}
										return FText::FromString(Values[Index]);
									})
									.ColorAndOpacity(FSlateColor::UseForeground())
									.TextStyle(&FVoxelEditorStyle::GetWidgetStyle<FTextBlockStyle>("GraphPreviewValues")));
							}

							if (StatsRow->bGlobalSpacing)
							{
								RowValueWidgets.Add(Widgets);
							}

							return 
								SNew(STableRow<TSharedPtr<FRow>>, OwnerTable)
								.Style(Args._RowStyle)
								[
									SNew(SVerticalBox)
									.ToolTipText(StatsRow->Tooltip)
									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										.Padding(2.f)
										[
											SNew(SBox)
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											.WidthOverride(Args._BulletPointSize)
											.HeightOverride(Args._BulletPointSize)
											.Visibility(Args._BulletPointVisibility)
											[
												SNew(SImage)
												.Image(FAppStyle::GetBrush("Icons.BulletPoint"))
											]
										]
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.HAlign(HAlign_Left)
										.VAlign(VAlign_Center)
										[
											SNew(SBox)
											.WidthOverride(60.f)
											[
												SNew(STextBlock)
												.Text(FText::FromString(StatsRow->Header.ToString() + ": "))
												.ColorAndOpacity(FSlateColor::UseForeground())
												.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
											]
										]
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Center)
										.Padding(4.f, 0.f)
										[
											SNew(SBox)
											.WidthOverride_Lambda([this, StatsRow]
											{
												return StatsRow->bGlobalSpacing ? ValueWidths[0] : FOptionalSize();
											})
											.HAlign(HAlign_Fill)
											[
												Widgets[0].ToSharedRef()
											]
										]
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Center)
										.Padding(4.f, 0.f)
										[
											SNew(SBox)
											.WidthOverride_Lambda([this, StatsRow]
											{
												return StatsRow->bGlobalSpacing ? ValueWidths[1] : FOptionalSize();
											})
											[
												Widgets[1].ToSharedRef()
											]
										]
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Center)
										.Padding(4.f, 0.f)
										[
											SNew(SBox)
											.WidthOverride_Lambda([this, StatsRow]
											{
												return StatsRow->bGlobalSpacing ? ValueWidths[2] : FOptionalSize();
											})
											[
												Widgets[2].ToSharedRef()
											]
										]
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Center)
										.Padding(4.f, 0.f, 5.f, 0.f)
										[
											SNew(SBox)
											.WidthOverride_Lambda([this, StatsRow]
											{
												return StatsRow->bGlobalSpacing ? ValueWidths[3] : FOptionalSize();
											})
											[
												Widgets[3].ToSharedRef()
											]
										]
									]
								];
						})
						.ExternalScrollbar(ScrollBar)
						.ConsumeMouseWheel(EConsumeMouseWheel::Always)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(FOptionalSize(16))
					[
						ScrollBar
					]
				]
			]
		]
	];
}

void SVoxelGraphPreviewStats::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (RowsView &&
		RowsView->IsPendingRefresh())
	{
		return;
	}

	ValueWidths = { 0.f, 0.f, 0.f, 0.f };

	for (const TArray<TSharedPtr<SWidget>>& RowWidgets : RowValueWidgets)
	{
		for (int32 Index = 0; Index < RowWidgets.Num(); Index++)
		{
			ValueWidths[Index] = FMath::Max(RowWidgets[Index]->GetDesiredSize().X, ValueWidths[Index]);
		}
	}
}

void SVoxelGraphPreviewStats::Reset()
{
	Rows.Reset();
	RowValueWidgets.Reset();
}