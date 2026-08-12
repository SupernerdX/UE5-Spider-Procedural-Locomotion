// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelBaseDiff.h"

#include "DiffUtils.h"
#include "VoxelDiffMode.h"
#include "Subsystems/AssetEditorSubsystem.h"

void SVoxelBaseDiff_Base::Construct(const FArguments& InArgs)
{
	OldAsset = InArgs._OldAsset;
	NewAsset = InArgs._NewAsset;
	OldRevision = InArgs._OldRevision;
	NewRevision = InArgs._NewRevision;

	InitializeModes(Modes);
	if (!ensure(Modes.Num() > 0))
	{
		return;
	}

	FToolBarBuilder NavToolBarBuilder(nullptr, FMultiBoxCustomization::None);
	NavToolBarBuilder.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateSP(this, &SVoxelBaseDiff_Base::JumpToPrev),
			FCanExecuteAction::CreateSP(this, &SVoxelBaseDiff_Base::HasPrev)
		),
		{},
		INVTEXT("Prev"),
		INVTEXT("Go to previous difference"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "BlueprintDif.PrevDiff")
	);
	NavToolBarBuilder.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateSP(this, &SVoxelBaseDiff_Base::JumpToNext),
			FCanExecuteAction::CreateSP(this, &SVoxelBaseDiff_Base::HasNext)
		),
		{},
		INVTEXT("Next"),
		INVTEXT("Go to next difference"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "BlueprintDif.NextDiff")
	);

	const auto TextBlock = [](const FText& Text) -> TSharedRef<SWidget>
	{
		return
			SNew(SBox)
			.Padding(4.f, 10.f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
				.Text(Text)
			];
	};

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Docking.Tab", ".ContentAreaBrush"))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.VAlign(VAlign_Top)
			[
				SNew(SSplitter)
				.Visibility(EVisibility::HitTestInvisible)
				+ SSplitter::Slot()
				.Value(0.2f)
				[
					SNew(SBox)
				]
				+ SSplitter::Slot()
				.Value(0.8f)
				[
					SNew(SSplitter)
					.PhysicalSplitterHandleSize(10.f)
					+ SSplitter::Slot()
					.Value(0.5f)
					[
						TextBlock(DiffViewUtils::GetPanelLabel(OldAsset, OldRevision, {}))
					]
					+ SSplitter::Slot()
					.Value(0.5f)
					[
						TextBlock(DiffViewUtils::GetPanelLabel(NewAsset, NewRevision, {}))
					]
				]
			]
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 2.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(4.f)
					.AutoWidth()
					[
						NavToolBarBuilder.MakeWidget()
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SSpacer)
					]
				]
				+ SVerticalBox::Slot()
				[
					SNew(SSplitter)
					+ SSplitter::Slot()
					.Value(0.2f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SAssignNew(DiffTreeView, STreeView<TSharedPtr<FVoxelDiffEntry>>)
							.OnGenerateRow_Lambda([](TSharedPtr<FVoxelDiffEntry> Entry, const TSharedRef<STableViewBase>& Owner)
							{
								return
									SNew(STableRow<TSharedPtr<FVoxelDiffEntry>>, Owner)
									[
										Entry->GenerateWidget()
									];
							})
							.OnGetChildren_Lambda([](TSharedPtr<FVoxelDiffEntry> Entry, TArray<TSharedPtr<FVoxelDiffEntry>>& OutChildren)
							{
								OutChildren = Entry->Children;
							})
							.OnSelectionChanged_Lambda([this](TSharedPtr<FVoxelDiffEntry> Entry, ESelectInfo::Type Type)
							{
								OnEntrySelected(Entry);
							})
							.TreeItemsSource(&Entries)
						]
					]
					+ SSplitter::Slot()
					.Value(0.8f)
					[
						SAssignNew(ModeContents, SBox)
					]
				]
			]
		]
	];

	GenerateDifferencesList();
	if (Modes.Num() > 1)
	{
		DiffTreeView->SetSelection(Entries[0]);
	}
	else
	{
		const TSharedPtr<FVoxelDiffMode> EntryMode = Entries[0]->WeakMode.Pin();
		if (ensure(EntryMode))
		{
			SetMode(EntryMode);
		}
	}
}

void SVoxelBaseDiff_Base::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	for (const TSharedPtr<FVoxelDiffMode>& Mode : Modes)
	{
		Mode->Tick();
	}

	if (bRefresh)
	{
		bRefresh = false;
		GenerateDifferencesList();
	}
}

void SVoxelBaseDiff_Base::AssignWindow(const TSharedPtr<SWindow>& Window)
{
	if (!Window)
	{
		return;
	}

	WeakWindow = Window;
	AssetEditorCloseDelegate = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorRequestClose().AddLambda(MakeWeakPtrLambda(this, [this](UObject* Asset, const EAssetEditorCloseReason CloseReason)
	{
		if (OldAsset != Asset &&
			NewAsset != Asset &&
			CloseReason != EAssetEditorCloseReason::CloseAllAssetEditors)
		{
			return;
		}

		SetVisibility(EVisibility::Collapsed);

		if (AssetEditorCloseDelegate.IsValid())
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorRequestClose().Remove(AssetEditorCloseDelegate);
		}

		if (const TSharedPtr<SWindow> PinnedWindow = WeakWindow.Pin())
		{
			PinnedWindow->RequestDestroyWindow();
		}
	}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelBaseDiff_Base::OnEntrySelected(const TSharedPtr<FVoxelDiffEntry>& Entry)
{
	if (!Entry)
	{
		return;
	}

	const TSharedPtr<FVoxelDiffMode> EntryMode = Entry->WeakMode.Pin();
	if (ensure(EntryMode))
	{
		SetMode(EntryMode);
	}

	if (!Entry->IsA<FVoxelNoDiffEntry>() &&
		!Entry->IsA<FVoxelModeDiffEntry>())
	{
		EntryMode->OnEntrySelected(Entry);
	}
}

void SVoxelBaseDiff_Base::GenerateDifferencesList()
{
	if (!ensure(Modes.Num() > 0))
	{
		return;
	}

	if (Modes.Num() == 1)
	{
		Modes[0]->GenerateDifferencesList(Entries);
		if (Entries.Num() == 0)
		{
			Entries.Add(MakeShared<FVoxelNoDiffEntry>(Modes[0]));
		}
		else
		{
			LinearEntries = Entries;
		}
		return;
	}

	Entries = {};
	LinearEntries = {};
	for (const TSharedPtr<FVoxelDiffMode>& Mode : Modes)
	{
		TSharedPtr<FVoxelDiffEntry> ModeEntry = Mode->GenerateTreeEntry();
		Entries.Add(ModeEntry);
		if (ModeEntry->Children.Num() > 0)
		{
			LinearEntries.Append(ModeEntry->Children);
		}
		else
		{
			ModeEntry->Children.Add(MakeShared<FVoxelNoDiffEntry>(Mode));
		}
	}

	DiffTreeView->RequestTreeRefresh();
}

bool SVoxelBaseDiff_Base::NewAssetIsLocal() const
{
	return NewRevision.Revision.IsEmpty();
}

void SVoxelBaseDiff_Base::SetMode(const TSharedPtr<FVoxelDiffMode>& NewMode)
{
	if (NewMode == CurrentMode)
	{
		return;
	}

	CurrentMode = NewMode;
	ModeContents->SetContent(CurrentMode->GetWidget());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool SVoxelBaseDiff_Base::HasPrev() const
{
	return LinearEntries.IsValidIndex(GetSelectedEntryIndex() - 1);
}

void SVoxelBaseDiff_Base::JumpToPrev()
{
	const int32 Index = GetSelectedEntryIndex();
	if (!LinearEntries.IsValidIndex(Index - 1))
	{
		return;
	}

	const TSharedPtr<FVoxelDiffEntry> PrevEntry = LinearEntries[Index - 1];
	DiffTreeView->SetSelection(PrevEntry);
	DiffTreeView->RequestScrollIntoView(PrevEntry);
}

bool SVoxelBaseDiff_Base::HasNext() const
{
	return LinearEntries.IsValidIndex(GetSelectedEntryIndex() + 1);
}

void SVoxelBaseDiff_Base::JumpToNext()
{
	const int32 Index = GetSelectedEntryIndex();
	if (!LinearEntries.IsValidIndex(Index + 1))
	{
		return;
	}

	const TSharedPtr<FVoxelDiffEntry> NextEntry = LinearEntries[Index + 1];
	DiffTreeView->SetSelection(NextEntry);
	DiffTreeView->RequestScrollIntoView(NextEntry);
}

int32 SVoxelBaseDiff_Base::GetSelectedEntryIndex() const
{
	if (DiffTreeView->GetNumItemsSelected() == 0)
	{
		return -1;
	}

	const TSharedPtr<FVoxelDiffEntry> SelectedItem = DiffTreeView->GetSelectedItems()[0];
	return LinearEntries.Find(SelectedItem);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelBaseDiff_Base::CreateDiffWindow(const TSharedRef<SVoxelBaseDiff_Base>& Widget)
{
	const TSharedPtr<SWindow> Window =
		SNew(SWindow)
		.Title(Widget->GetWindowName())
		.ClientSize(FVector2D(1000.f, 800.f))
		[
			Widget
		];

	Widget->AssignWindow(Window);

	if (const TSharedPtr<SWindow> ActiveModal = FSlateApplication::Get().GetActiveTopLevelWindow())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(Window.ToSharedRef(), ActiveModal.ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(Window.ToSharedRef());
	}

	Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda(MakeWeakPtrLambda(Widget.Get(), [WeakWidget = MakeWeakPtr(Widget)](const TSharedRef<SWindow>&)
	{
		if (const TSharedPtr<SVoxelBaseDiff_Base> PinnedWidget = WeakWidget.Pin())
		{
			PinnedWidget->OnWindowClosed.Broadcast(PinnedWidget);
		}
	})));
}