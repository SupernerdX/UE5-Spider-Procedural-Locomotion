// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelPlaceStampsTile.h"

#include "VoxelActorFactories.h"
#include "SVoxelPlaceStampsTab.h"
#include "VoxelPlaceStampsSubsystem.h"

#include "LevelEditorActions.h"
#include "LevelEditorViewport.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Internationalization/BreakIterator.h"
#include "Toolkits/GlobalEditorCommonCommands.h"

VOXEL_INITIALIZE_STYLE(VoxelPlaceStampsTile)
{
	const FCheckBoxStyle FavoriteToggleStyle = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::CheckBox)
		.SetUncheckedImage( CORE_IMAGE_BRUSH("Icons/EmptyStar_16x", CoreStyleConstants::Icon16x16, FLinearColor(0.8f, 0.8f, 0.8f, 1.f)) )
		.SetUncheckedHoveredImage( CORE_IMAGE_BRUSH("Icons/EmptyStar_16x", CoreStyleConstants::Icon16x16, FLinearColor(2.5f, 2.5f, 2.5f, 1.f)) )
		.SetUncheckedPressedImage( CORE_IMAGE_BRUSH("Icons/EmptyStar_16x", CoreStyleConstants::Icon16x16, FLinearColor(0.8f, 0.8f, 0.8f, 1.f)) )
		.SetCheckedImage( CORE_IMAGE_BRUSH("Icons/Star_16x", CoreStyleConstants::Icon16x16, FLinearColor(0.2f, 0.2f, 0.2f, 1.f)) )
		.SetCheckedHoveredImage( CORE_IMAGE_BRUSH("Icons/Star_16x", CoreStyleConstants::Icon16x16, FLinearColor(0.4f, 0.4f, 0.4f, 1.f)) )
		.SetCheckedPressedImage( CORE_IMAGE_BRUSH("Icons/Star_16x", CoreStyleConstants::Icon16x16, FLinearColor(0.2f, 0.2f, 0.2f, 1.f)) );
	Set("PlaceStampFavoriteCheckbox", FavoriteToggleStyle);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelPlaceStampDragDropOp : public FAssetDragDropOp
{
public:
	static TSharedRef<FVoxelPlaceStampDragDropOp> New(
		const FAssetData& InAssetData,
		const TScriptInterface<IAssetFactoryInterface>& Factory)
	{
		TArray<FAssetData> AssetDataArray;
		AssetDataArray.Emplace(InAssetData);

		TSharedRef<FVoxelPlaceStampDragDropOp> Operation = MakeShared<FVoxelPlaceStampDragDropOp>();
		Operation->Init(AssetDataArray, {}, Factory);
		Operation->Construct();

		Operation->ActorsPlacedDelegate = FEditorDelegates::OnNewActorsPlaced.AddLambda([](UObject*, const TArray<AActor*>& Actors)
		{
			UVoxelPlaceStampsSubsystem::UpdateActors(Actors);
		});

		Operation->ActorsDroppedDelegate = FEditorDelegates::OnNewActorsDropped.AddLambda([](const TArray<UObject*>&, const TArray<AActor*>& Actors)
		{
			UVoxelPlaceStampsSubsystem::UpdateActors(Actors);
		});

		return Operation;
	}

	virtual ~FVoxelPlaceStampDragDropOp()
	{
		FEditorDelegates::OnNewActorsPlaced.Remove(ActorsPlacedDelegate);
		FEditorDelegates::OnNewActorsPlaced.Remove(ActorsDroppedDelegate);
	}

private:
	FDelegateHandle ActorsPlacedDelegate;
	FDelegateHandle ActorsDroppedDelegate;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelPlaceStampsTile::Construct(const FArguments& InArgs, const TSharedPtr<FVoxelPlaceStampsItem>& InItem)
{
	WeakItem = InItem;

	const TSharedRef<SWrapBox> TagsContainer = SNew(SWrapBox).UseAllottedSize(true);

	TArray<FName> ContentTags = InItem->UserTags.Array();
	ContentTags.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });
	for (const FName ContentTag : ContentTags)
	{
		const FLinearColor TagColor = SVoxelPlaceStampsTab::GetStampTagColor(ContentTag);

		TagsContainer->AddSlot()
		.Padding(1.f)
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(10.f, 6.f))
			.ColorAndOpacity_Lambda([ContentTag, TagColor, VisibleTags = InArgs._VisibleTags]
			{
				FLinearColor Color = TagColor;
				if (!VisibleTags.Get().Contains(ContentTag))
				{
					Color.A = 0.1f;
				}

				return Color;
			})
			.Image(FAppStyle::GetBrush("WhiteBrush"))
		];
	}

	TSharedPtr<SWidget> Thumbnail;
	EHorizontalAlignment HorizontalThumbnailAlignment = HAlign_Fill;
	EVerticalAlignment VerticalThumbnailAlignment = VAlign_Fill;

	if (InItem->AssetThumbnail)
	{
		Thumbnail = InItem->AssetThumbnail->MakeThumbnailWidget();
	}
	else
	{
		Thumbnail =
			SNew(SImage)
			.Image(FAppStyle::GetBrush(*InItem->BrushName));

		HorizontalThumbnailAlignment = HAlign_Center;
		VerticalThumbnailAlignment = VAlign_Center;
	}

	ChildSlot
	.Padding(2.0f)
	[
		SNew(SBorder)
		.ToolTipText_Lambda([InItem]
		{
			return FText::FromString(InItem->Description);
		})
		.Padding(0.f, 0.f, 5.f, 5.f)
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
					.WidthOverride(115.f)
					.HeightOverride(115.f)
					.HAlign(HorizontalThumbnailAlignment)
					.VAlign(VerticalThumbnailAlignment)
					[
						Thumbnail.ToSharedRef()
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SNew(SBorder)
					.VAlign(VAlign_Fill)
					.Padding(3.f, 6.f, 3.f, 3.f)
					.BorderImage_Lambda([this]
					{
						if (IsHovered())
						{
							return FAppStyle::Get().GetBrush("ProjectBrowser.ProjectTile.NameAreaHoverBackground");
						}

						return FAppStyle::Get().GetBrush("ProjectBrowser.ProjectTile.NameAreaBackground");
					})
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.VAlign(VAlign_Top)
						[
							SNew(STextBlock)
							.Font(FAppStyle::Get().GetFontStyle("ProjectBrowser.ProjectTile.Font"))
							.AutoWrapText(true)
							.LineBreakPolicy(FBreakIterator::CreateCamelCaseBreakIterator())
							.Text_Lambda([InItem]
							{
								return FText::FromString(InItem->Name);
							})
							.ColorAndOpacity_Lambda([this]
							{
								return
									IsHovered()
									? FStyleColors::White
									: FSlateColor::UseForeground();
							})
						]
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Bottom)
						.AutoHeight()
						.Padding(0.f, 3.f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.TextStyle(&FAppStyle::GetWidgetStyle<FTextBlockStyle>("ContentBrowser.ClassFont"))
								.AutoWrapText(true)
								.LineBreakPolicy(FBreakIterator::CreateCamelCaseBreakIterator())
								.Text_Lambda([InItem]
								{
									return FText::FromString(InItem->Type);
								})
								.ColorAndOpacity_Lambda([this]
								{
									return
										IsHovered()
										? FStyleColors::White
										: FSlateColor::UseSubduedForeground();
								})
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Right)
							[
								TagsContainer
							]
						]
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Visibility(EVisibility::HitTestInvisible)
				.Image_Lambda([this]
				{
					if (IsHovered())
					{
						return FAppStyle::Get().GetBrush("ProjectBrowser.ProjectTile.HoverBorder");
					}

					return FStyleDefaults::GetNoBrush();
				})
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			[
				SNew(SCheckBox)
				.Style(&FVoxelEditorStyle::GetWidgetStyle<FCheckBoxStyle>("PlaceStampFavoriteCheckbox"))
				.Cursor(EMouseCursor::Hand)
				.IsChecked_Lambda([this]
				{
					const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
					if (!Item)
					{
						return ECheckBoxState::Unchecked;
					}

					return Item->bIsFavorite ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([this, Delegate = InArgs._OnFavoriteStateChanged](const ECheckBoxState NewState)
				{
					const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
					if (!Item)
					{
						return;
					}

					Item->bIsFavorite = NewState == ECheckBoxState::Checked;
					Delegate.ExecuteIfBound(Item->bIsFavorite);
				})
			]
		]
	];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TOptional<EMouseCursor::Type> SVoxelPlaceStampsTile::GetCursor() const
{
	return EMouseCursor::GrabHand;
}

FReply SVoxelPlaceStampsTile::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPressed = true;
		return FReply::Handled().DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
	}

	if (MouseEvent.GetEffectingButton() != EKeys::RightMouseButton)
	{
		return FReply::Unhandled();
	}

	const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
	if (!Item)
	{
		return FReply::Unhandled();
	}

	if (Item->Asset.GetClass()->IsChildOf<UScriptStruct>())
	{
		return FReply::Unhandled();
	}

	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("BasicOperations");
	{
		MenuBuilder.AddMenuEntry(
			INVTEXT("Browse to Asset"),
			INVTEXT("Browses to the associated asset and selects it in the most recently used Content Browser (summoning one if necessary)"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "SystemWideCommands.FindInContentBrowser.Small"),
			MakeLambdaDelegate([this]
			{
				const TSharedPtr<FVoxelPlaceStampsItem> PinnedItem = WeakItem.Pin();
				if (!PinnedItem)
				{
					return;
				}

				GEditor->SyncBrowserToObjects({ PinnedItem->Asset });
			}));
	}
	MenuBuilder.EndSection();

	const FWidgetPath WidgetPath =
		MouseEvent.GetEventPath() != nullptr
		? *MouseEvent.GetEventPath()
		: FWidgetPath();

	FSlateApplication::Get().PushMenu(
		AsShared(),
		WidgetPath,
		MenuBuilder.MakeWidget(),
		MouseEvent.GetScreenSpacePosition(),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

	return FReply::Handled();
}

FReply SVoxelPlaceStampsTile::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	bIsPressed = false;

	return FReply::Handled();
}

FReply SVoxelPlaceStampsTile::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
	if (!Item)
	{
		return FReply::Unhandled();
	}

	if (Item->Asset.GetClass()->IsChildOf<UScriptStruct>())
	{
		return FReply::Unhandled();
	}

	if (UAssetEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		Subsystem->OpenEditorForAsset(Item->Asset.GetSoftObjectPath());
	}

	return FReply::Handled();
}

FReply SVoxelPlaceStampsTile::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsPressed = false;

	const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
	if (!Item)
	{
		return FReply::Unhandled();
	}

	if (FEditorDelegates::OnAssetDragStarted.IsBound() &&
		Item->Asset.IsValid())
	{
		TArray<FAssetData> DraggedAssetDatas;
		DraggedAssetDatas.Add(Item->Asset);
		FEditorDelegates::OnAssetDragStarted.Broadcast(DraggedAssetDatas, nullptr);
		return FReply::Handled();
	}

	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (ensure(Item->Asset.IsValid()))
		{
			const TScriptInterface<IAssetFactoryInterface> AssetFactory = Item->ActorFactory;
			if (ensure(AssetFactory))
			{
				return FReply::Handled().BeginDragDrop(FVoxelPlaceStampDragDropOp::New(Item->Asset, AssetFactory));
			}
		}
	}

	return FReply::Handled();
}

void SVoxelPlaceStampsTile::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);

	const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
	if (!Item ||
		!Item->AssetThumbnail)
	{
		return;
	}

	Item->AssetThumbnail->SetRealTime(true);
}

void SVoxelPlaceStampsTile::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseLeave(MouseEvent);

	const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
	if (!Item ||
		!Item->AssetThumbnail)
	{
		return;
	}

	Item->AssetThumbnail->SetRealTime(false);
}