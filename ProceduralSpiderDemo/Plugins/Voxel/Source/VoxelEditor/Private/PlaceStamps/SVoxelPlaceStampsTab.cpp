// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelPlaceStampsTab.h"

#include "VoxelGraph.h"
#include "Shape/VoxelShape.h"
#include "VoxelActorFactories.h"
#include "SVoxelPlaceStampsTile.h"
#include "Heightmap/VoxelHeightmap.h"
#include "VoxelPlaceStampsSubsystem.h"
#include "StaticMesh/VoxelStaticMesh.h"
#include "SVoxelPlaceStampsTagCheckBox.h"
#include "Sculpt/Volume/VoxelSculptVolume.h"
#include "Sculpt/Height/VoxelSculptHeight.h"

#include "WidgetDrawerConfig.h"
#include "IPlacementModeModule.h"
#include "IStructureDetailsView.h"
#include "Toolkits/AssetEditorModeUILayer.h"

void SVoxelPlaceStampsTab::Construct(const FArguments& InArgs, const TSharedPtr<SDockTab>& ParentTab)
{
	FillFavoritesList();

	Filter = MakeShared<FContentSourceTextFilter>(
		FContentSourceTextFilter::FItemToStringArray::CreateLambda([](const TSharedPtr<FVoxelPlaceStampsItem> Item, TArray<FString>& Array)
		{
			Array.Add(Item->Name);
		})
	);

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName).Get();
	AssetRegistry.OnAssetsAdded().AddSP(this, &SVoxelPlaceStampsTab::OnAssetsAdded);
	AssetRegistry.OnAssetRenamed().AddSP(this, &SVoxelPlaceStampsTab::OnAssetRenamed);
	AssetRegistry.OnAssetUpdated().AddSP(this, &SVoxelPlaceStampsTab::OnAssetUpdated);
	AssetRegistry.OnAssetRemoved().AddSP(this, &SVoxelPlaceStampsTab::OnAssetRemoved);

	if (ParentTab)
	{
		ParentTab->SetOnTabDrawerOpened(MakeWeakPtrDelegate(this, [this]
		{
			FSlateApplication::Get().SetKeyboardFocus(SearchBox, EFocusCause::SetDirectly);
		}));
	}

	{
		UVoxelPlaceStampsSubsystem* PlaceStampsSubsystem = GEditor->GetEditorSubsystem<UVoxelPlaceStampsSubsystem>();

		FDetailsViewArgs Args;
		Args.bShowOptions = false;
		Args.bHideSelectionTip = true;
		Args.bShowPropertyMatrixButton = false;
		Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
		FStructureDetailsViewArgs StructArgs;
		StructArgs.bShowObjects = true;
		StructArgs.bShowInterfaces = true;

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		DetailsView = PropertyModule.CreateStructureDetailView(Args, StructArgs, PlaceStampsSubsystem->GetStructOnScope());
		DetailsView->GetDetailsView()->SetDisableCustomDetailLayouts(false);
		DetailsView->GetDetailsView()->SetGenericLayoutDetailsDelegate(MakeLambdaDelegate([this]() -> TSharedRef<IDetailCustomization>
		{
			return MakeShared<FVoxelPlaceStampDefaultsCustomization>(ActiveTabName);
		}));
		DetailsView->GetDetailsView()->ForceRefresh();

		DetailsView->GetOnFinishedChangingPropertiesDelegate().Add(MakeWeakPtrDelegate(this, [](const FPropertyChangedEvent& Event)
		{
			UVoxelPlaceStampsSubsystem::SaveDefaults();
		}));
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		.Padding(0.f)
		[
			SNew(SSplitter)
			.PhysicalSplitterHandleSize(2.f)
			+ SSplitter::Slot()
			.Resizable(false)
			.SizeRule(SSplitter::SizeToContent)
			[
				CreateCategoriesWidget()
			]
			+ SSplitter::Slot()
			.SizeRule(SSplitter::FractionOfParent)
			[
				SAssignNew(ContentSplitter, SSplitter)
				.PhysicalSplitterHandleSize(2.f)
				+ SSplitter::Slot()
				.Value(0.8f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Brushes.Recessed"))
					[
						SNew( SVerticalBox )
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
							.Padding(8.f)
							[
								SAssignNew(SearchBox, SSearchBox)
								.HintText(INVTEXT("Search Stamps"))
								.OnTextChanged_Lambda([this](const FText& Text)
								{
									Filter->SetRawFilterText(Text);
									UpdateFilteredItems();
								})
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 0.f, 0.f, 8.f)
						[
							SAssignNew(TagsBox, SWrapBox)
							.UseAllottedSize(true)
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SAssignNew(WidgetSwitcher, SWidgetSwitcher)
							+ SWidgetSwitcher::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SAssignNew(MessageTextBlock, STextBlock)
								.ColorAndOpacity(FSlateColor::UseSubduedForeground())
								.Text({})
							]
							+ SWidgetSwitcher::Slot()
							[
								SAssignNew(TileView, STileView<TSharedPtr<FVoxelPlaceStampsItem>>)
								.ListItemsSource(&FilteredCategoryItems)
								.OnGenerateTile_Lambda([this](const TSharedPtr<FVoxelPlaceStampsItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
								{
									return
										SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
										.Style(FAppStyle::Get(), "ProjectBrowser.TableRow")
										.Padding(2.f)
										[
											SNew(SVoxelPlaceStampsTile, Item)
											.VisibleTags_Lambda([this]
											{
												return VisibleTags;
											})
											.OnFavoriteStateChanged_Lambda([this, Item](const bool bNewFavorite)
											{
												const FSoftObjectPath ObjectPath = Item->Asset.GetSoftObjectPath();
												if (bNewFavorite)
												{
													Favorites.Add(ObjectPath);
												}
												else
												{
													Favorites.Remove(ObjectPath);
													if (ActiveTabName == "Favorites")
													{
														AllCategoryItems.Remove(Item);
														TrackedAssetToItem.Remove(ObjectPath);
														UpdateTags(false);
														UpdateFilteredItems();
													}
												}

												UpdateFavorites();
											})
										];
								})
								.ClearSelectionOnClick(false)
								.ItemAlignment(EListItemAlignment::EvenlySize)
								.ItemWidth(122.f)
								.ItemHeight(197.f)
								.SelectionMode(ESelectionMode::None)
							]
						]
					]
				]
				+ SSplitter::Slot()
				.Value(0.2f)
				[
					DetailsView->GetWidget().ToSharedRef()
				]
			]
		]
	];

	SetActiveTab("Shapes");
}

void SVoxelPlaceStampsTab::FocusSearchBox() const
{
	FSlateApplication::Get().SetKeyboardFocus(SearchBox, EFocusCause::SetDirectly);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelPlaceStampsTab::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	const FVector2D Size = AllottedGeometry.GetLocalSize();
	EOrientation Orientation = Orient_Horizontal;
	if (Size.X < 350.f ||
		Size.Y > Size.X)
	{
		Orientation = Orient_Vertical;
	}

	ContentSplitter->SetOrientation(Orientation);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> SVoxelPlaceStampsTab::CreateCategoriesWidget()
{
	FVerticalToolBarBuilder ToolbarBuilder(
		nullptr,
		FMultiBoxCustomization::None,
		TSharedPtr<FExtender>(),
		true);
	ToolbarBuilder.SetLabelVisibility(EVisibility::Visible);
	ToolbarBuilder.SetStyle(&FAppStyle::Get(), "CategoryDrivenContentBuilderToolbarWithLabels");

	const auto AddCategory = [&](const FName Name, const FSlateIcon& Icon, const FCollectItemsFunction& Function)
	{
		CategoryToCollectFunction.Add(Name, Function);
		ToolbarBuilder.AddToolBarButton(
			FUIAction(
				MakeWeakPtrDelegate(this, [this, Name]
				{
					SetActiveTab(Name);
				}),
				{},
				MakeWeakPtrDelegate(this, [this, Name]
				{
					return ActiveTabName == Name;
				})),
			{},
			FText::FromName(Name),
			{},
			Icon,
			EUserInterfaceActionType::ToggleButton);
	};

	AddCategory(
		"Favorites",
		FSlateIcon(FAppStyle::Get().GetStyleSetName(), "Icons.Favorites.Small"),
		MakeLambdaDelegate([this](TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems)
		{
			CollectAllFavorites(OutItems);
		}));
	AddCategory(
		"Recent",
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "PlacementBrowser.Icons.Recent"),
		MakeLambdaDelegate([](TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems)
		{
			CollectAllRecent(OutItems);
		}));
	AddCategory(
		"Shapes",
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Cube"),
		MakeLambdaDelegate([](TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems)
		{
			for (UScriptStruct* Struct : GetDerivedStructs<FVoxelShape>())
			{
				OutItems.Add(ConstructShape(FAssetData(Struct)));
			}
		}));
	AddCategory(
		"Graphs",
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.Blueprint"),
		MakeLambdaDelegate([](TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems)
		{
			ForEachAssetDataOfClass<UVoxelGraph>([&](const FAssetData& Asset)
			{
				OutItems.Add(ConstructGraph(Asset));
			});
		}));
	AddCategory(
		"Meshes",
		FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "ClassIcon.VoxelStaticMesh"),
		MakeLambdaDelegate([](TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems)
		{
			ForEachAssetDataOfClass<UVoxelStaticMesh>([&](const FAssetData& Asset)
			{
				OutItems.Add(ConstructMesh(Asset));
			});
		}));
	AddCategory(
		"Heightmaps",
		FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "ClassIcon.VoxelGraphHeightmap"),
		MakeLambdaDelegate([](TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems)
		{
			ForEachAssetDataOfClass<UVoxelHeightmap>([&](const FAssetData& Asset)
			{
				OutItems.Add(ConstructHeightmap(Asset));
			});
		}));
	AddCategory(
		"Sculpt",
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LandscapeEditor.SculptTool"),
		MakeLambdaDelegate([this](TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems)
		{
			OutItems.Add(ConstructSculpt(FAssetData(AVoxelSculptHeight::StaticClass())));
			OutItems.Add(ConstructSculpt(FAssetData(AVoxelSculptVolume::StaticClass())));
		}));

	return ToolbarBuilder.MakeWidget();
}

void SVoxelPlaceStampsTab::SetActiveTab(const FName Name)
{
	ActiveTabName = Name;
	UpdateContentForCategory(Name);

	if (DetailsView)
	{
		DetailsView->GetDetailsView()->ForceRefresh();
	}
}

void SVoxelPlaceStampsTab::UpdateContentForCategory(const FName CategoryName)
{
	TrackedAssetToItem = {};
	AllCategoryItems = {};

	if (const FCollectItemsFunction* Delegate = CategoryToCollectFunction.Find(CategoryName))
	{
		if (Delegate->IsBound())
		{
			Delegate->Execute(AllCategoryItems);
		}
	}

	for (auto It = AllCategoryItems.CreateIterator(); It; ++It)
	{
		const TSharedPtr<FVoxelPlaceStampsItem> Item = *It;
		if (!ensure(Item) ||
			!ensure(Item->Asset.IsValid()))
		{
			It.RemoveCurrent();
			continue;
		}

		Item->Setup();

		if (!ensureVoxelSlow(Item->ActorFactory))
		{
			It.RemoveCurrent();
			continue;
		}

		if (Item->Asset.IsValid())
		{
			TrackedAssetToItem.Add(Item->Asset.GetSoftObjectPath(), Item);
			if (Favorites.Contains(Item->Asset.GetSoftObjectPath()))
			{
				Item->bIsFavorite = true;
			}
		}
	}

	UpdateTags(true);
	UpdateFilteredItems();
}

void SVoxelPlaceStampsTab::UpdateTags(const bool bResetVisibleTags)
{
	TagsBox->ClearChildren();

	Tags = {};
	bool bAddEmptyTag = false;
	for (const TSharedPtr<FVoxelPlaceStampsItem>& Item : AllCategoryItems)
	{
		Tags.Append(Item->UserTags);
		if (UseSystemTags())
		{
			Tags.Append(Item->SystemTags);
		}
		if (Item->UserTags.Num() == 0)
		{
			bAddEmptyTag = true;
		}
	}

	if (Tags.Num() > 0 &&
		bAddEmptyTag)
	{
		Tags.Add("Untagged");
	}

	if (bResetVisibleTags)
	{
		VisibleTags = Tags;
	}
	else
	{
		for (auto It = VisibleTags.CreateIterator(); It; ++It)
		{
			if (!Tags.Contains(*It))
			{
				It.RemoveCurrent();
			}
		}
	}

	if (Tags.Num() == 0)
	{
		TagsBox->SetVisibility(EVisibility::Collapsed);
		return;
	}

	TagsBox->SetVisibility(EVisibility::Visible);

	const auto AllTagsVisible = [this]
	{
		for (const FName ContentTag : Tags)
		{
			if (!VisibleTags.Contains(ContentTag))
			{
				return false;
			}
		}

		return true;
	};

	TagsBox->AddSlot()
	.Padding(5.f)
	[
		SNew(SBorder)
		.BorderImage(FVoxelEditorStyle::GetBrush("Graph.NewAssetDialog.FilterCheckBox.Border"))
		.ToolTipText(INVTEXT("Show all"))
		.Padding(3.f)
		[
			SNew(SCheckBox)
			.Style(FVoxelEditorStyle::Get(), "Graph.NewAssetDialog.FilterCheckBox")
			.BorderBackgroundColor_Lambda([=]() -> FSlateColor
			{
				return AllTagsVisible() ? FLinearColor::White : FSlateColor::UseForeground();
			})
			.IsChecked_Lambda([=]() -> ECheckBoxState
			{
				return AllTagsVisible() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([=, this](ECheckBoxState)
			{
				VisibleTags = AllTagsVisible() ? TSet<FName>() : Tags;
				UpdateFilteredItems();
			})
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(2.f)
				[
					SNew(STextBlock)
					.Text(INVTEXT("Show all"))
					.ShadowOffset(0.f)
					.ColorAndOpacity_Lambda([=]() -> FSlateColor
					{
						return AllTagsVisible() ? FLinearColor::Black : FLinearColor::Gray;
					})
					.TextStyle(FVoxelEditorStyle::Get(), "Graph.NewAssetDialog.ActionFilterTextBlock")
				]
			]
		]
	];

	TArray<FName> SortedTags = Tags.Array();
	SortedTags.Sort([](const FName& A, const FName& B) { return A.LexicalLess(B); });

	for (const FName ContentTag : SortedTags)
	{
		TagsBox->AddSlot()
		.Padding(3.f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush(TEXT("NoBorder")))
			.Padding(3.f)
			[
				SNew(SVoxelPlaceStampsTagCheckBox)
				.ContentTag(ContentTag)
				.IsChecked_Lambda([=, this]
				{
					return VisibleTags.Contains(ContentTag) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnContentTagClicked_Lambda([=, this](const bool bState)
				{
					if (bState)
					{
						VisibleTags.Add(ContentTag);
					}
					else
					{
						VisibleTags.Remove(ContentTag);
					}
					UpdateFilteredItems();
				})
				.OnContentTagShiftClicked_Lambda([=, this]
				{
					VisibleTags = { ContentTag };
					UpdateFilteredItems();
				})
			]
		];
	}
}

void SVoxelPlaceStampsTab::UpdateFilteredItems()
{
	FilteredCategoryItems = AllCategoryItems.FilterByPredicate([&](const TSharedPtr<FVoxelPlaceStampsItem>& Item)
	{
		if (!Filter->PassesFilter(Item))
		{
			return false;
		}

		if (UseSystemTags())
		{
			for (const FName ContentTag : Item->SystemTags)
			{
				if (VisibleTags.Contains(ContentTag))
				{
					return true;
				}
			}
		}

		if (Item->UserTags.Num() == 0)
		{
			if (Tags.Num() > 0)
			{
				return VisibleTags.Contains(STATIC_FNAME("Untagged"));
			}

			return true;
		}

		for (const FName ContentTag : Item->UserTags)
		{
			if (VisibleTags.Contains(ContentTag))
			{
				return true;
			}
		}

		return false;
	});

	TileView->RequestListRefresh();

	if (FilteredCategoryItems.Num() == 0)
	{
		WidgetSwitcher->SetActiveWidgetIndex(0);
		// TODO: Texts
		if (Tags.Num() > 0 &&
			VisibleTags.Num() == 0)
		{
			MessageTextBlock->SetText(INVTEXT("No available stamps without tags"));
		}
		else if (!SearchBox->GetText().IsEmpty())
		{
			MessageTextBlock->SetText(INVTEXT("No available stamps matching this search"));
		}
		else
		{
			MessageTextBlock->SetText(INVTEXT("No available stamps in this category"));
		}
		return;
	}

	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void SVoxelPlaceStampsTab::UpdateFavorites()
{
	TArray<FString> FavoriteStrings;
	for (const FSoftObjectPath& Data : Favorites)
	{
		FavoriteStrings.Add(Data.ToString());
	}

	GConfig->SetArray(TEXT("VoxelPlaceStamps"), TEXT("Favorites"), FavoriteStrings, GEditorPerProjectIni);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelPlaceStampsTab::OnAssetsAdded(const TArrayView<const FAssetData> AssetDatas)
{
	TFunction<TSharedPtr<FVoxelPlaceStampsItem>(const FAssetData&)> ConstructFunction = nullptr;
	if (ActiveTabName == "Graphs")
	{
		ConstructFunction = &ConstructGraph;
	}
	else if (ActiveTabName == "Meshes")
	{
		ConstructFunction = &ConstructMesh;
	}
	else if (ActiveTabName == "Heightmaps")
	{
		ConstructFunction = &ConstructHeightmap;
	}
	else if (ActiveTabName == "Sculpts")
	{
		ConstructFunction = &ConstructSculpt;
	}
	else
	{
		return;
	}

	bool bHasNewItems = false;
	for (const FAssetData& Asset : AssetDatas)
	{
		const FSoftObjectPath SoftPath = Asset.GetSoftObjectPath();

		// After renaming/moving asset, rename fixes the current item
		if (TrackedAssetToItem.Contains(SoftPath))
		{
			continue;
		}

		TSharedPtr<FVoxelPlaceStampsItem> NewItem = ConstructFunction(Asset);
		if (!NewItem)
		{
			continue;
		}

		NewItem->Setup();

		AllCategoryItems.Add(NewItem);
		TrackedAssetToItem.Add(SoftPath, NewItem);
		bHasNewItems = true;
	}

	if (!bHasNewItems)
	{
		return;
	}

	UpdateTags(false);
	UpdateFilteredItems();
}

void SVoxelPlaceStampsTab::OnAssetRenamed(const FAssetData& AssetData, const FString& OldName)
{
	const FSoftObjectPath OldPath = FSoftObjectPath(OldName);
	if (Favorites.Contains(OldPath))
	{
		Favorites.Remove(OldPath);
		Favorites.Add(AssetData.GetSoftObjectPath());
	}

	const TSharedPtr<FVoxelPlaceStampsItem> Item = TrackedAssetToItem.FindRef(OldPath);
	if (!Item)
	{
		return;
	}

	Item->Name = AssetData.AssetName.ToString();
	Item->Asset = AssetData;
	TrackedAssetToItem.Remove(OldPath);
	TrackedAssetToItem.Add(AssetData.GetSoftObjectPath(), Item);
	UpdateFilteredItems();
}

void SVoxelPlaceStampsTab::OnAssetUpdated(const FAssetData& AssetData)
{
	const TSharedPtr<FVoxelPlaceStampsItem> Item = TrackedAssetToItem.FindRef(AssetData.GetSoftObjectPath());
	if (!Item)
	{
		return;
	}

	Item->Name = AssetData.AssetName.ToString();
	Item->AssetThumbnail->RefreshThumbnail();
	Item->Asset = AssetData;

	if (Item->GetDescription.IsBound())
	{
		Item->Description = Item->GetDescription.Execute();
	}

	if (Item->GetTags.IsBound())
	{
		Item->UserTags = Item->GetTags.Execute();
		UpdateTags(false);
	}

	UpdateFilteredItems();
}

void SVoxelPlaceStampsTab::OnAssetRemoved(const FAssetData& AssetData)
{
	const FSoftObjectPath AssetPath = AssetData.GetSoftObjectPath();
	Favorites.Remove(AssetPath);

	const TSharedPtr<FVoxelPlaceStampsItem> Item = TrackedAssetToItem.FindRef(AssetPath);
	if (!Item)
	{
		return;
	}

	TrackedAssetToItem.Remove(AssetPath);
	AllCategoryItems.Remove(Item);

	UpdateTags(false);
	UpdateFilteredItems();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelPlaceStampsItem> SVoxelPlaceStampsTab::ConstructGraph(const FAssetData& AssetData)
{
	const UClass* Class = AssetData.GetClass();
	if (!Class)
	{
		return nullptr;
	}

	if (!Class->IsChildOf<UVoxelGraph>())
	{
		return nullptr;
	}

	const TSharedRef<FVoxelPlaceStampsItem> NewItem = MakeShared<FVoxelPlaceStampsItem>();
	NewItem->Name = AssetData.AssetName.ToString();
	NewItem->AssetThumbnail = MakeShared<FAssetThumbnail>(AssetData, 128.f, 128.f, FVoxelEditorUtilities::GetThumbnailPool());
	NewItem->GetDescription = MakeWeakPtrDelegate(NewItem, [WeakItem = MakeWeakPtr(NewItem)]() -> FString
	{
		const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
		if (!Item)
		{
			return {};
		}

		return Item->Asset.GetTagValueRef<FString>(GET_MEMBER_NAME_STRING_CHECKED(UVoxelGraph, Description));
	});
	NewItem->GetTags = MakeWeakPtrDelegate(NewItem, [WeakItem = MakeWeakPtr(NewItem)]() -> TSet<FName>
	{
		const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
		if (!Item)
		{
			return {};
		}

		TSet<FName> Result;

		FVoxelUtilities::PropertyFromText_Direct(
			FindFPropertyChecked(UVoxelGraph, GraphTags),
			Item->Asset.GetTagValueRef<FString>(GET_MEMBER_NAME_STRING_CHECKED(UVoxelGraph, GraphTags)),
			reinterpret_cast<void*>(&Result),
			nullptr);

		return Result;
	});

	NewItem->SystemTags = { "Graph" };
	NewItem->Asset = AssetData;

	NewItem->Type = INLINE_LAMBDA
	{
		FString TypeName = AssetData.GetClass()->GetDisplayNameText().ToString();
		TypeName.RemoveFromStart("Voxel");
		return TypeName;
	};

	return NewItem;
}

TSharedPtr<FVoxelPlaceStampsItem> SVoxelPlaceStampsTab::ConstructMesh(const FAssetData& AssetData)
{
	const UClass* Class = AssetData.GetClass();
	if (!Class)
	{
		return nullptr;
	}

	if (!Class->IsChildOf<UVoxelStaticMesh>())
	{
		return nullptr;
	}


	const TSharedRef<FVoxelPlaceStampsItem> NewItem = MakeShared<FVoxelPlaceStampsItem>();
	NewItem->Name = AssetData.AssetName.ToString();
	NewItem->AssetThumbnail = MakeShared<FAssetThumbnail>(AssetData, 128.f, 128.f, FVoxelEditorUtilities::GetThumbnailPool());

	NewItem->GetTags = MakeWeakPtrDelegate(NewItem, [WeakItem = MakeWeakPtr(NewItem)]() -> TSet<FName>
	{
		const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
		if (!Item)
		{
			return {};
		}

		TSet<FName> Result;

		FVoxelUtilities::PropertyFromText_Direct(
			FindFPropertyChecked(UVoxelStaticMesh, Tags),
			Item->Asset.GetTagValueRef<FString>(GET_MEMBER_NAME_STRING_CHECKED(UVoxelStaticMesh, Tags)),
			reinterpret_cast<void*>(&Result),
			nullptr);

		return Result;
	});

	NewItem->SystemTags = { "Mesh" };
	NewItem->Asset = AssetData;

	NewItem->Type = "Mesh";

	return NewItem;
}

TSharedPtr<FVoxelPlaceStampsItem> SVoxelPlaceStampsTab::ConstructHeightmap(const FAssetData& AssetData)
{
	const UClass* Class = AssetData.GetClass();
	if (!Class)
	{
		return nullptr;
	}

	if (!Class->IsChildOf<UVoxelHeightmap>())
	{
		return nullptr;
	}

	const TSharedRef<FVoxelPlaceStampsItem> NewItem = MakeShared<FVoxelPlaceStampsItem>();
	NewItem->Name = AssetData.AssetName.ToString();
	NewItem->AssetThumbnail = MakeShared<FAssetThumbnail>(AssetData, 128.f, 128.f, FVoxelEditorUtilities::GetThumbnailPool());

	NewItem->GetTags = MakeWeakPtrDelegate(NewItem, [WeakItem = MakeWeakPtr(NewItem)]() -> TSet<FName>
	{
		const TSharedPtr<FVoxelPlaceStampsItem> Item = WeakItem.Pin();
		if (!Item)
		{
			return {};
		}

		TSet<FName> Result;

		FVoxelUtilities::PropertyFromText_Direct(
			FindFPropertyChecked(UVoxelHeightmap, Tags),
			Item->Asset.GetTagValueRef<FString>(GET_MEMBER_NAME_STRING_CHECKED(UVoxelHeightmap, Tags)),
			reinterpret_cast<void*>(&Result),
			nullptr);

		return Result;
	});

	NewItem->SystemTags = { "Heightmap" };
	NewItem->Asset = AssetData;

	NewItem->Type = "Heightmap";

	return NewItem;
}

TSharedPtr<FVoxelPlaceStampsItem> SVoxelPlaceStampsTab::ConstructSculpt(const FAssetData& AssetData)
{
	const UClass* Class = Cast<UClass>(AssetData.GetAsset());
	if (!Class)
	{
		return nullptr;
	}

	if (!Class->IsChildOf<AVoxelSculptActorBase>())
	{
		return nullptr;
	}

	const TSharedRef<FVoxelPlaceStampsItem> NewItem = MakeShared<FVoxelPlaceStampsItem>();
	NewItem->Name = Class->GetDisplayNameText().ToString();
	NewItem->AssetThumbnail = MakeShared<FAssetThumbnail>(AssetData, 128.f, 128.f, FVoxelEditorUtilities::GetThumbnailPool());

	NewItem->Asset = AssetData;

	if (Class->IsChildOf<AVoxelSculptHeight>())
	{
		NewItem->SystemTags = { "Sculpt", "Height" };
	}
	else if (Class->IsChildOf<AVoxelSculptVolume>())
	{
		NewItem->SystemTags = { "Sculpt", "Volume" };
	}
	else
	{
		ensure(false);
	}

	NewItem->Type = "Sculpt";

	return NewItem;
}

TSharedPtr<FVoxelPlaceStampsItem> SVoxelPlaceStampsTab::ConstructShape(const FAssetData& AssetData)
{
	const UClass* Class = AssetData.GetClass();
	if (!Class)
	{
		return nullptr;
	}

	if (!Class->IsChildOf<UScriptStruct>())
	{
		return nullptr;
	}

	const UScriptStruct* Struct = Cast<UScriptStruct>(AssetData.GetAsset());
	if (!Struct->IsChildOf<FVoxelShape>())
	{
		return nullptr;
	}

	const TSharedRef<FVoxelPlaceStampsItem> NewItem = MakeShared<FVoxelPlaceStampsItem>();
	if (Struct->HasMetaData("ShortName"))
	{
		NewItem->Name = Struct->GetMetaData("ShortName");
	}
	else
	{
		NewItem->Name = Struct->GetDisplayNameText().ToString();
	}

	if (Struct->HasMetaData("Thumbnail"))
	{
		NewItem->BrushName = Struct->GetMetaData("Thumbnail");
	}
	else
	{
		NewItem->AssetThumbnail = MakeShared<FAssetThumbnail>(AssetData, 128.f, 128.f, FVoxelEditorUtilities::GetThumbnailPool());
	}
	NewItem->Asset = AssetData;

	NewItem->Type = "Shape";

	return NewItem;
}

void SVoxelPlaceStampsTab::FillFavoritesList()
{
	TArray<FString> FavoriteStrings;
	GConfig->GetArray(TEXT("VoxelPlaceStamps"), TEXT("Favorites"), FavoriteStrings, GEditorPerProjectIni);

	for (const FString& Data : FavoriteStrings)
	{
		Favorites.Add(FSoftObjectPath(Data));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelPlaceStampsTab::CollectAllFavorites(TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems)
{
	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	for (const FSoftObjectPath& Path : Favorites)
	{
		FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(Path);
		if (!AssetData.IsValid())
		{
			continue;
		}

		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructGraph(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}
		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructMesh(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}
		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructHeightmap(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}
		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructSculpt(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}
		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructShape(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}

		ensure(false);
	}
}

bool SVoxelPlaceStampsTab::UseSystemTags() const
{
	return
		ActiveTabName == STATIC_FNAME("Favorites") ||
		ActiveTabName == STATIC_FNAME("Recent");
}

void SVoxelPlaceStampsTab::CollectAllRecent(TArray<TSharedPtr<FVoxelPlaceStampsItem>>& OutItems)
{
	if (!IPlacementModeModule::IsAvailable())
	{
		return;
	}

	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	for (const FActorPlacementInfo& Data : IPlacementModeModule::Get().GetRecentlyPlaced())
	{
		FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(Data.ObjectPath);
		if (!AssetData.IsValid())
		{
			continue;
		}

		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructGraph(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}
		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructMesh(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}
		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructHeightmap(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}
		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructSculpt(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}
		if (const TSharedPtr<FVoxelPlaceStampsItem> Item = ConstructShape(AssetData))
		{
			OutItems.Add(Item);
			continue;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FLinearColor SVoxelPlaceStampsTab::GetStampTagColor(const FName StampTag)
{
	if (StampTag == STATIC_FNAME("Untagged"))
	{
		return FLinearColor::Black;
	}

	const uint32 Hash = FCrc::StrCrc32(*StampTag.ToString());
	const FLinearColor TagColor =  FLinearColor::MakeRandomSeededColor(Hash);
	return TagColor;
}