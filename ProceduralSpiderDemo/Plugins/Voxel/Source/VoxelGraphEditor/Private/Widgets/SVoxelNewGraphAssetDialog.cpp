// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelNewGraphAssetDialog.h"
#include "VoxelGraph.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Internationalization/BreakIterator.h"

struct FVoxelGraphTile
{
	FString Name;
	FString Description;
	TWeakObjectPtr<UVoxelGraph> Template;
	const FSlateBrush* Thumbnail = nullptr;
};

void SVoxelNewGraphAssetDialog::Construct(const FArguments& InArgs)
{
	WeakParentWindow = InArgs._ParentWindow;

	for (const TSubclassOf<UVoxelGraph>& Class : GetDerivedClasses<UVoxelGraph>())
	{
		const UVoxelGraph::FFactoryInfo FactoryInfo = Class->GetDefaultObject<UVoxelGraph>()->GetFactoryInfo();

		const TSharedRef<FVoxelGraphTile> GraphTile = MakeShared<FVoxelGraphTile>();
		GraphTile->Name = FactoryInfo.GetDisplayName(Class);

		GraphTile->Description = FactoryInfo.Description;
		
		if (FactoryInfo.Icon)
		{
			GraphTile->Thumbnail = FactoryInfo.Icon;
		}
		else
		{
			GraphTile->Thumbnail = FAppStyle::GetBrush("ClassThumbnail.Blueprint");
		}

		if (ensure(FactoryInfo.Template))
		{
			GraphTile->Template = FactoryInfo.Template;
		}
		else
		{
			GraphTile->Template = Class->GetDefaultObject<UVoxelGraph>();
		}

		CategoryToGraphsList.FindOrAdd(FactoryInfo.Category).Add(GraphTile);
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(0.7f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Brushes.Recessed"))
				.Padding(3.f, 0.f)
				[
					ConstructCategories()
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(0.3f)
			.Padding(0.f)
			[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(3.f, 5.f)
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(INVTEXT("Description"))
							.TextStyle(FVoxelEditorStyle::Get(), "AddContent.Category")
						]
						+ SHorizontalBox::Slot()
						.Padding(FMargin(14.f, 0.f, 0.f, 0.f))
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SNew(SSeparator)
							.Orientation(Orient_Horizontal)
							.Thickness(1.f)
							.SeparatorImage(FAppStyle::GetBrush("PinnedCommandList.Separator"))
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(DescriptionWidget, STextBlock)
							.AutoWrapText(true)
						]
					]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
			.Thickness(2.f)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.f, 16.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.TextStyle(FAppStyle::Get(), "DialogButtonText")
				.OnClicked_Lambda([this]
				{
					bCreateAsset = false;

					if (const TSharedPtr<SWindow> Parent = WeakParentWindow.Pin())
					{
						Parent->RequestDestroyWindow();
					}

					return FReply::Handled();
				})
				.Text(INVTEXT("Cancel"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.ButtonStyle(&FAppStyle::GetWidgetStyle<FButtonStyle>("PrimaryButton"))
				.TextStyle(&FAppStyle::GetWidgetStyle<FTextBlockStyle>("PrimaryButtonText"))
				.OnClicked_Lambda([this]
				{
					if (!ensure(SelectedTile))
					{
						return FReply::Handled();
					}

					bCreateAsset = true;
					SelectedGraph = SelectedTile->Template;

					if (const TSharedPtr<SWindow> Parent = WeakParentWindow.Pin())
					{
						Parent->RequestDestroyWindow();
					}

					return FReply::Handled();
				})
				.IsEnabled_Lambda([this]
				{
					return SelectedTile != nullptr;
				})
				.Text(INVTEXT("Create"))
			]
		]
	];
}

DEFINE_PRIVATE_ACCESS(STableViewBase, bEnableRightClickScrolling);

TSharedRef<SWidget> SVoxelNewGraphAssetDialog::ConstructCategories()
{
	const TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);
	ScrollBox->SetScrollBarRightClickDragAllowed(true);

	for (const auto& It : CategoryToGraphsList)
	{
		TSharedPtr<STileView<TSharedPtr<FVoxelGraphTile>>> TilesView;

		ScrollBox->AddSlot()
		[
			SNew(SExpandableArea)
			.BorderImage(FStyleDefaults::GetNoBrush())
			.BodyBorderImage(FAppStyle::Get().GetBrush("Brushes.Recessed"))
			.HeaderPadding(FMargin(5.0f, 3.0f))
			.AllowAnimatedTransition(false)
			.InitiallyCollapsed(false)
			.HeaderContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromName(It.Key))
					.TextStyle(FVoxelEditorStyle::Get(), "AddContent.Category")
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(14.f, 0.f, 0.f, 0.f))
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SSeparator)
					.Orientation(Orient_Horizontal)
					.Thickness(1.f)
					.SeparatorImage(FAppStyle::GetBrush("PinnedCommandList.Separator"))
				]
			]
			.BodyContent()
			[
				SAssignNew(TilesView, STileView<TSharedPtr<FVoxelGraphTile>>)
				.ConsumeMouseWheel(EConsumeMouseWheel::Never)
				.ListItemsSource(&It.Value)
				.OnGenerateTile_Lambda([this](const TSharedPtr<FVoxelGraphTile> Item, const TSharedRef<STableViewBase>& OwnerTable)
				{
					TSharedRef<STableRow<TSharedPtr<FVoxelGraphTile>>> Widget =
						SNew(STableRow<TSharedPtr<FVoxelGraphTile>>, OwnerTable)
						.Style(FAppStyle::Get(), "ProjectBrowser.TableRow")
						.Padding(2.f);

					Widget->SetContent(
						SNew(SBorder)
						.Padding(FMargin(0.f, 0.f, 5.f, 5.f))
						.BorderImage(FAppStyle::Get().GetBrush("ProjectBrowser.ProjectTile.DropShadow"))
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SBox)
									.WidthOverride(102.f)
									.HeightOverride(102.f)
									[
										SNew(SImage)
										.DesiredSizeOverride(FVector2D(102.f))
										.Image(Item->Thumbnail)
									]
								]
								+ SVerticalBox::Slot()
								.FillHeight(1.f)
								[
									SNew(SBorder)
									.Padding(5.f, 0.f)
									.VAlign(VAlign_Fill)
									.Padding(3.f, 6.f, 3.f, 3.f)
									.BorderImage(FAppStyle::Get().GetBrush("ProjectBrowser.ProjectTile.NameAreaBackground"))
									[
										SNew(STextBlock)
										.Font(FAppStyle::Get().GetFontStyle("ProjectBrowser.ProjectTile.Font"))
										.AutoWrapText(true)
										.LineBreakPolicy(FBreakIterator::CreateCamelCaseBreakIterator())
										.Text(FText::FromString(Item->Name))
										.ColorAndOpacity(FAppStyle::Get().GetSlateColor("Colors.Foreground"))
									]
								]
							]
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Visibility(EVisibility::HitTestInvisible)
								.Image_Lambda([this, Item, WeakWidget = MakeWeakPtr(Widget)]
								{
									const bool bSelected = SelectedTile == Item;
									bool bHovered = false;
									if (const TSharedPtr<STableRow<TSharedPtr<FVoxelGraphTile>>> PinnedWidget = WeakWidget.Pin())
									{
										bHovered = PinnedWidget->IsHovered();
									}

									if (bSelected && bHovered)
									{
										static const FName SelectedHover("ProjectBrowser.ProjectTile.SelectedHoverBorder");
										return FAppStyle::Get().GetBrush(SelectedHover);
									}
									else if (bSelected)
									{
										static const FName Selected("ProjectBrowser.ProjectTile.SelectedBorder");
										return FAppStyle::Get().GetBrush(Selected);
									}
									else if (bHovered)
									{
										static const FName Hovered("ProjectBrowser.ProjectTile.HoverBorder");
										return FAppStyle::Get().GetBrush(Hovered);
									}

									return FStyleDefaults::GetNoBrush();
								})
							]
						]);
					return Widget;
				})
				.OnSelectionChanged_Lambda([this, Category = It.Key](const TSharedPtr<FVoxelGraphTile> NewSelection, const ESelectInfo::Type SelectInfo)
				{
					for (const auto& InnerIt : CategoryToTilesView)
					{
						if (InnerIt.Key != Category)
						{
							InnerIt.Value->ClearSelection();
						}
					}

					if (!NewSelection)
					{
						DescriptionWidget->SetText({});
						return;
					}

					DescriptionWidget->SetText(FText::FromString(NewSelection->Description));
					SelectedTile = NewSelection;
				})
				.OnMouseButtonDoubleClick_Lambda([this](const TSharedPtr<FVoxelGraphTile> NewSelection)
				{
					bCreateAsset = true;
					SelectedGraph = NewSelection->Template;

					if (const TSharedPtr<SWindow> Parent = WeakParentWindow.Pin())
					{
						Parent->RequestDestroyWindow();
					}
				})
				.ClearSelectionOnClick(false)
				.ItemAlignment(EListItemAlignment::LeftAligned)
				.ItemWidth(102.f)
				.ItemHeight(153.f)
				.SelectionMode(ESelectionMode::Single)
			]
		];

		PrivateAccess::bEnableRightClickScrolling(*TilesView) = false;
		CategoryToTilesView.Add(It.Key, TilesView);
	}

	return ScrollBox;
}