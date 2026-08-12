// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelConsoleCommandsTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

FVoxelConsoleCommandsTabManager* GVoxelCommandsTabManager = new FVoxelConsoleCommandsTabManager();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VOXEL_INITIALIZE_STYLE(VoxelConsoleCommandsStyle)
{
	Set("Favorites.Icon", new CORE_IMAGE_BRUSH("Icons/Star_16x", CoreStyleConstants::Icon16x16, FLinearColor(0.2f, 0.2f, 0.2f, 1.f)));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelConsoleCommandsTabManager::Initialize()
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();

	TabManager->RegisterNomadTabSpawner(GlobalConsoleCommandsTabId, MakeLambdaDelegate([=](const FSpawnTabArgs& SpawnTabArgs)
	{
		return
			SNew(SDockTab)
			.TabRole(NomadTab)
			.Label(INVTEXT("Voxel Console Commands"))
			.ToolTipText(INVTEXT("Change, execute available voxel console commands"))
			[
				SNew(SVoxelConsoleCommandsTab)
			];
	}))
	.SetDisplayName(INVTEXT("Voxel Console Commands"))
	.SetIcon(FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "VoxelIcon"))
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
}

void FVoxelConsoleCommandsTabManager::OpenGlobalCommands() const
{
	FGlobalTabmanager::Get()->TryInvokeTab(GlobalConsoleCommandsTabId);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelConsoleCommandRow::Construct(
	const FArguments& Args,
	const TSharedRef<STableViewBase>& OwnerTableView,
	const TSharedPtr<FVoxelConsoleCommandItem>& InItem)
{
	Item = InItem;
	HighlightText = Args._HighlightText;
	IsFavorite = Args._IsFavorite;
	OnFavoriteStateChanged = Args._OnFavoriteStateChanged;

	FSuperRowType::Construct(
		FSuperRowType::FArguments()
		.Style(&FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("SceneOutliner.TableViewRow")),
		OwnerTableView);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> SVoxelConsoleCommandRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == "Name")
	{
		return
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(ExpanderArrowWidget, SExpanderArrow, SharedThis(this) )
				.StyleSet(ExpanderStyleSet)
				.ShouldDrawWires(false)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.Visibility(Item->Object ? EVisibility::Visible : EVisibility::Collapsed)
				.Style(&FVoxelEditorStyle::GetWidgetStyle<FCheckBoxStyle>("PlaceStampFavoriteCheckbox"))
				.Cursor(EMouseCursor::Hand)
				.IsChecked_Lambda([this]
				{
					return IsFavorite.Get() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([this](const ECheckBoxState NewState)
				{
					OnFavoriteStateChanged.ExecuteIfBound();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Visibility(
					!Item->Object &&
					Item->DisplayName.ToString() == "Favorites"
					? EVisibility::Visible
					: EVisibility::Collapsed)
				.Image(FVoxelEditorStyle::GetBrush("Favorites.Icon"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SNew(SBox)
				.MinDesiredHeight(24.f)
				.VAlign(VAlign_Center)
				.Padding(4.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Item->DisplayName)
					.Font(Item->Object ? FAppStyle::GetFontStyle("PropertyWindow.NormalFont") : FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
					.ColorAndOpacity(FSlateColor::UseForeground())
					.HighlightText(HighlightText)
				]
			];
	}

	if (!Item->Object)
	{
		return SNullWidget::NullWidget;
	}

	if (ColumnName != "Value")
	{
		ensure(false);
		return SNullWidget::NullWidget;
	}

	if (IConsoleCommand* Command = Item->Object->AsCommand())
	{
		return
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(2.f, 0.f)
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SAssignNew(EditableTextBox, SEditableTextBox)
				.HintText(INVTEXT("Enter arguments..."))
				.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
			]
			+ SHorizontalBox::Slot()
			.Padding(2.f, 0.f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(INVTEXT("Execute"))
				.OnClicked_Lambda([this, Command]
				{
					TArray<FString> Args;
					EditableTextBox->GetText().ToString().ParseIntoArrayWS(Args);
					Command->Execute(Args, nullptr, *GLog);
					return FReply::Handled();
				})
			];
	}

	IConsoleVariable* VariableObject = Item->Object->AsVariable();
	if (!ensure(VariableObject))
	{
		return SNullWidget::NullWidget;
	}

	if (VariableObject->IsVariableBool())
	{
		return
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(2.f, 0.f)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([VariableObject]
				{
					return
						VariableObject->GetBool()
						? ECheckBoxState::Checked
						: ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([VariableObject](const ECheckBoxState NewState)
				{
					VariableObject->Set(NewState == ECheckBoxState::Checked, ECVF_SetByConsole);
				})
			];
	}

	if (VariableObject->IsVariableInt())
	{
		return
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			.Padding(2.f, 0.f)
			[
				SNew(SNumericEntryBox<int32>)
				.AllowSpin(true)
				.MinValue({})
				.MaxValue({})
				.MinSliderValue({})
				.MaxSliderValue({})
				.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.OnValueChanged_Lambda([VariableObject](const int32 NewValue)
				{
					VariableObject->Set(NewValue, ECVF_SetByConsole);
				})
				.Value_Lambda([VariableObject]
				{
					return VariableObject->GetInt();
				})
			];
	}

	if (VariableObject->IsVariableFloat())
	{
		return
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			.Padding(2.f, 0.f)
			[
				SNew(SNumericEntryBox<float>)
				.AllowSpin(true)
				.MinValue({})
				.MaxValue({})
				.MinSliderValue({})
				.MaxSliderValue({})
				.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.OnValueChanged_Lambda([VariableObject](const float NewValue)
				{
					VariableObject->Set(NewValue, ECVF_SetByConsole);
				})
				.Value_Lambda([VariableObject]
				{
					return VariableObject->GetFloat();
				})
			];
	}

	if (VariableObject->IsVariableFloat())
	{
		return
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(2.f, 0.f)
			[
				SNew(SEditableTextBox)
				.Font(FAppStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.OnTextChanged_Lambda([VariableObject](const FText& NewValue)
				{
					VariableObject->Set(*NewValue.ToString(), ECVF_SetByConsole);
				})
				.Text_Lambda([VariableObject]
				{
					return FText::FromString(VariableObject->GetString());
				})
			];
	}

	return SNullWidget::NullWidget;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelConsoleCommandsTab::Construct(const FArguments& InArgs)
{
	Filter = MakeShared<FConsoleCommandTextFilter>(
		FConsoleCommandTextFilter::FItemToStringArray::CreateLambda([](const TSharedPtr<FVoxelConsoleCommandItem> Item, TArray<FString>& Array)
		{
			Array.Add(Item->DisplayName.ToString());
		})
	);

	{
		TArray<FString> FavoriteStrings;
		GConfig->GetArray(TEXT("VoxelConsoleCommands"), TEXT("Favorites"), FavoriteStrings, GEditorPerProjectIni);

		for (const FString& Data : FavoriteStrings)
		{
			FavoriteCommands.Add(FName(Data));
		}
	}

	CollectCommands();
	UpdateFilteredCommands();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.f, 5.f)
			[
				SAssignNew(SearchTextField, SSearchBox)
				.HintText(INVTEXT("Enter command name..."))
				.OnTextChanged_Lambda([this](const FText& Text)
				{
					HighlightText = Text;
					Filter->SetRawFilterText(Text);
					UpdateFilteredCommands();
				})
				.DelayChangeNotificationsWhileTyping(false)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("Brushes.Recessed"))
				.Padding(0.f)
				[
					SAssignNew(TreeView, STreeView<TSharedPtr<FVoxelConsoleCommandItem>>)
					.TreeViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("PropertyTable.InViewport.ListView"))
					.TreeItemsSource(&RootItems)
					.OnGenerateRow_Lambda([this](const TSharedPtr<FVoxelConsoleCommandItem>& Item, const TSharedRef<STableViewBase>& OwnerTable)
					{
						return
							SNew(SVoxelConsoleCommandRow, OwnerTable, Item)
							.ToolTipText(Item->ToolTip)
							.HighlightText_Lambda([this]
							{
								return HighlightText;
							})
							.IsFavorite_Lambda([this, Item]
							{
								return FavoriteCommands.Contains(Item->FullCommand);
							})
							.OnFavoriteStateChanged_Lambda([this, Item]
							{
								if (!FavoriteCommands.Remove(Item->FullCommand))
								{
									FavoriteCommands.Add(Item->FullCommand);
								}

								UpdateFavorites();
							});
					})
					.OnGetChildren_Lambda([=](const TSharedPtr<FVoxelConsoleCommandItem> Item, TArray<TSharedPtr<FVoxelConsoleCommandItem>>& OutChildren)
					{
						if (!ensure(Item))
						{
							return;
						}

						OutChildren.Append(Item->Children);
					})
					.SelectionMode(ESelectionMode::None)
					.HeaderRow(
						SNew(SHeaderRow)
						+ SHeaderRow::Column("Name")
						.HAlignCell(HAlign_Left)
						.VAlignCell(VAlign_Center)
						.VAlignHeader(VAlign_Center)
						.FillWidth(0.7f)
						[
							SNew(SBox)
							.MinDesiredHeight(18.f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SVoxelDetailText)
								.Text(INVTEXT("Name"))
							]
						]
						+ SHeaderRow::Column("Value")
						.HAlignCell(HAlign_Fill)
						.VAlignCell(VAlign_Center)
						.VAlignHeader(VAlign_Center)
						.FillWidth(0.3f)
						[
							SNew(SBox)
							.MinDesiredHeight(18.f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SVoxelDetailText)
								.Text(INVTEXT("Value"))
							]
						]
					)
				]
			]
		]
	];

	for (const TSharedPtr<FVoxelConsoleCommandItem>& Item : AllFlattenedItems)
	{
		TreeView->SetItemExpansion(Item, true);
	}

	TreeView->SetItemExpansion(FavoriteCommandsCategory, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelConsoleCommandsTab::CollectCommands()
{
	TMap<FName, TSharedPtr<FVoxelConsoleCommandItem>> CategoryToEntry;

	FavoriteCommandsCategory = MakeShared<FVoxelConsoleCommandItem>();
	FavoriteCommandsCategory->DisplayName = INVTEXT("Favorites");

	const TSharedPtr<FVoxelConsoleCommandItem> DefaultCategory = MakeShared<FVoxelConsoleCommandItem>();
	DefaultCategory->DisplayName = INVTEXT("Default");
	AllRootItems.Add(DefaultCategory);
	AllFlattenedItems.Add(DefaultCategory);

	IConsoleManager::Get().ForEachConsoleObjectThatStartsWith(
		MakeLambdaDelegate([&](const TCHAR* Variable, IConsoleObject* Object)
		{
			FString Name = Variable;
			Name.RemoveFromStart("voxel.");

			TArray<FString> Sections;
			Name.ParseIntoArray(Sections, TEXT("."));

			TArray<TSharedPtr<FVoxelConsoleCommandItem>>* ParentList = &AllRootItems;

			ON_SCOPE_EXIT
			{
				const TSharedRef<FVoxelConsoleCommandItem> Item = MakeShared<FVoxelConsoleCommandItem>();
				Item->DisplayName = FText::FromString(FName::NameToDisplayString(Sections.Last(), false));
				Item->ToolTip = FText::FromString(Object->GetDetailedHelp().ToString() + "\n\n" + Variable);
				Item->Object = Object;
				Item->FullCommand = FName(Variable);

				if (!ensure(ParentList))
				{
					return;
				}

				ParentList->Add(Item);
				AllFlattenedItems.Add(Item);
			};

			if (Sections.Num() == 1)
			{
				ParentList = &DefaultCategory->Children;
				return;
			}

			FString SectionName;
			for (int32 Index = 0; Index < Sections.Num() - 1; Index++)
			{
				SectionName = (SectionName.IsEmpty() ? "" : ".") + Sections[Index];
				const FName SectionPath = FName(SectionName);

				TSharedPtr<FVoxelConsoleCommandItem> CategoryItem = CategoryToEntry.FindRef(SectionPath);
				if (!CategoryItem)
				{
					CategoryItem = MakeShared<FVoxelConsoleCommandItem>();
					CategoryItem->DisplayName = FText::FromString(FName::NameToDisplayString(Sections[Index], false));
					ParentList->Add(CategoryItem);

					CategoryToEntry.Add(SectionPath, CategoryItem);
					AllFlattenedItems.Add(CategoryItem);
				}

				ParentList = &CategoryItem->Children;
			}
		}),
		TEXT("voxel."));

	AllRootItems.Sort([](const TSharedPtr<FVoxelConsoleCommandItem>& A, const TSharedPtr<FVoxelConsoleCommandItem>& B)
	{
		return A->DisplayName.ToString() < B->DisplayName.ToString();
	});

	for (const TSharedPtr<FVoxelConsoleCommandItem>& Item : AllFlattenedItems)
	{
		Item->Children.Sort([](const TSharedPtr<FVoxelConsoleCommandItem>& A, const TSharedPtr<FVoxelConsoleCommandItem>& B)
		{
			return A->DisplayName.ToString() < B->DisplayName.ToString();
		});
	}

	UpdateFavoritesCategory();
}

void SVoxelConsoleCommandsTab::UpdateFilteredCommands()
{
	RootItems.Reset();

	const auto LookThroughList = [&](const TArray<TSharedPtr<FVoxelConsoleCommandItem>>& UnfilteredList, TArray<TSharedPtr<FVoxelConsoleCommandItem>>& OutFilteredList, auto& Lambda) -> bool
	{
		bool bReturnVal = false;
		for (const TSharedPtr<FVoxelConsoleCommandItem>& Item : UnfilteredList)
		{
			const bool bMatchesFilter = Filter->PassesFilter(Item);
			if (Item->Children.Num() == 0 ||
				bMatchesFilter)
			{
				if (bMatchesFilter)
				{
					OutFilteredList.Add(Item);

					if (Item->Children.Num() > 0 &&
						TreeView.IsValid())
					{
						TreeView->SetItemExpansion(Item, true);
					}

					bReturnVal = true;
				}
				continue;
			}

			TArray<TSharedPtr<FVoxelConsoleCommandItem>> ValidChildren;
			if (Lambda(Item->Children, ValidChildren, Lambda))
			{
				TSharedRef<FVoxelConsoleCommandItem> NewCategory = MakeShared<FVoxelConsoleCommandItem>();
				NewCategory->DisplayName = Item->DisplayName;
				NewCategory->Children = ValidChildren;
				OutFilteredList.Add(NewCategory);

				if (TreeView.IsValid())
				{
					TreeView->SetItemExpansion(NewCategory, true);
				}

				bReturnVal = true;
			}
		}

		return bReturnVal;
	};

	LookThroughList(AllRootItems, RootItems, LookThroughList);

	if (TreeView)
	{
		for (const TSharedPtr<FVoxelConsoleCommandItem>& Item : AllFlattenedItems)
		{
			TreeView->SetItemExpansion(Item, true);
		}

		TreeView->RequestTreeRefresh();
	}
}

void SVoxelConsoleCommandsTab::UpdateFavorites()
{
	TArray<FString> FavoriteStrings;
	for (const FName Command : FavoriteCommands)
	{
		FavoriteStrings.Add(Command.ToString());
	}

	GConfig->SetArray(TEXT("VoxelConsoleCommands"), TEXT("Favorites"), FavoriteStrings, GEditorPerProjectIni);

	UpdateFavoritesCategory();
	UpdateFilteredCommands();
}

void SVoxelConsoleCommandsTab::UpdateFavoritesCategory()
{
	AllRootItems.Remove(FavoriteCommandsCategory);

	FavoriteCommandsCategory->Children.Reset();
	bool bAddFavoritesCategory = false;
	for (const TSharedPtr<FVoxelConsoleCommandItem>& Item : AllFlattenedItems)
	{
		if (!Item->Object ||
			!FavoriteCommands.Contains(Item->FullCommand))
		{
			continue;
		}

		TSharedPtr<FVoxelConsoleCommandItem> NewItem = MakeShared<FVoxelConsoleCommandItem>(*Item);
		FavoriteCommandsCategory->Children.Add(NewItem);
		bAddFavoritesCategory = true;
	}

	if (!bAddFavoritesCategory)
	{
		return;
	}

	FavoriteCommandsCategory->Children.Sort([](const TSharedPtr<FVoxelConsoleCommandItem>& A, const TSharedPtr<FVoxelConsoleCommandItem>& B)
	{
		return A->DisplayName.ToString() < B->DisplayName.ToString();
	});

	AllRootItems.Insert(FavoriteCommandsCategory, 0);
}