// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelHeightmapList.h"
#include "Heightmap/VoxelHeightmap_Height.h"
#include "Heightmap/VoxelHeightmap_Weight.h"
#include "Surface/VoxelSurfaceTypeInterface.h"

#include "Engine/Texture2D.h"
#include "Editor/EditorWidgets/Public/SAssetDropTarget.h"

class FVoxelHeightStampListCommands : public TVoxelCommands<FVoxelHeightStampListCommands>
{
public:
	TSharedPtr<FUICommandInfo> AddWeightmap;

	virtual void RegisterCommands() override;
};

DEFINE_VOXEL_COMMANDS(FVoxelHeightStampListCommands);

void FVoxelHeightStampListCommands::RegisterCommands()
{
	VOXEL_UI_COMMAND(AddWeightmap, "Add Weightmap", "Adds new weightmap to list", EUserInterfaceActionType::Button, FInputChord());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelHeightStampLayer::FVoxelHeightStampLayer(UObject* Object, UObject* ThumbnailAsset, const int32 Index)
	: WeakObject(Object)
	, AssetThumbnail(MakeShared<FAssetThumbnail>(ThumbnailAsset, 32, 32, FVoxelEditorUtilities::GetThumbnailPool()))
	, bHeightmap(Object->IsA<UVoxelHeightmap_Height>())
	, Index(Index)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelHeightStampLayerDragDropOp> FVoxelHeightStampLayerDragDropOp::New(const TSharedPtr<FVoxelHeightStampLayer>& Layer)
{
	TSharedRef<FVoxelHeightStampLayerDragDropOp> DragDrop = MakeShared<FVoxelHeightStampLayerDragDropOp>();
	DragDrop->Layer = Layer;
	DragDrop->SetValidTarget(false);
	DragDrop->SetupDefaults();
	DragDrop->Construct();
	return DragDrop;
}

void FVoxelHeightStampLayerDragDropOp::SetValidTarget(const bool IsValidTarget)
{
	if (IsValidTarget)
	{
		CurrentHoverText = INVTEXT("Move layer here");
		CurrentIconBrush = FAppStyle::GetBrush("Graph.ConnectorFeedback.OK");
	}
	else
	{
		CurrentHoverText = INVTEXT("Layer cannot be moved here");
		CurrentIconBrush = FAppStyle::GetBrush("Graph.ConnectorFeedback.Error");
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelHeightmapList::Construct(const FArguments& InArgs)
{
	WeakAsset = InArgs._Asset;

	{
		// New details panel, for multiple top level objects
		FDetailsViewArgs Args;
		Args.bHideSelectionTip = true;
		Args.NotifyHook = InArgs._NotifyHook;
		Args.bAllowMultipleTopLevelObjects = true;
		Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		DetailsView = PropertyModule.CreateDetailView(Args);
	}

	CommandList = MakeShared<FUICommandList>();
	BindCommands();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Vertical)
			+ SSplitter::Slot()
			.Value(.3f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						[
							SNew(SBorder)
							.BorderImage(FAppStyle::Get().GetBrush("Brushes.Panel"))
							.HAlign(HAlign_Left)
							.Padding(12.f, 6.f)
							[
								SNew(STextBlock)
								.TextStyle(FAppStyle::Get(), "ButtonText")
								.Text(INVTEXT("Layers"))
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SButton)
							.ForegroundColor(FSlateColor::UseStyle())
							.ToolTipText(INVTEXT("Add Weightmap"))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.OnClicked_Lambda([this]
							{
								AddLayer(nullptr, nullptr);
								return FReply::Handled();
							})
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.AutoWidth()
								[
									SNew(SImage)
									.Image(FAppStyle::Get().GetBrush("Icons.Plus"))
									.ColorAndOpacity(FStyleColors::AccentGreen)
								]
								+ SHorizontalBox::Slot()
								.Padding(3.f, 0.f, 0.f, 0.f)
								.VAlign(VAlign_Center)
								.AutoWidth()
								[
									SNew(STextBlock)
									.TextStyle(FAppStyle::Get(), "SmallButtonText")
									.Text(INVTEXT("Add"))
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SSeparator)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SAssetDropTarget)
						.OnAreAssetsAcceptableForDropWithReason_Lambda([](TArrayView<FAssetData> AssetDatas, FText& OutReason)
						{
							for (const FAssetData& Asset : AssetDatas)
							{
								if (Asset.GetClass() != UTexture2D::StaticClass())
								{
									return false;
								}
							}

							return true;
						})
						.OnAssetsDropped_Lambda([this](const FDragDropEvent& Event, TArrayView<FAssetData> AssetDatas)
						{
							for (const FAssetData& Asset : AssetDatas)
							{
								if (Asset.GetClass() != UTexture2D::StaticClass())
								{
									continue;
								}

								AddLayer(nullptr, Cast<UTexture2D>(Asset.GetAsset()));
							}
						})
						[
							SAssignNew(ListView, SListView<TSharedPtr<FVoxelHeightStampLayer>>)
							.SelectionMode(ESelectionMode::Multi)
							.ListItemsSource(&Layers)
							.OnGenerateRow(this, &SVoxelHeightmapList::MakeWidgetFromLayer)
							.OnSelectionChanged_Lambda([this](TSharedPtr<FVoxelHeightStampLayer> Item, ESelectInfo::Type SelectInfo)
							{
								if (!Item)
								{
									DetailsView->SetObject(WeakAsset.Resolve(), true);
									return;
								}

								TArray<UObject*> Objects;
								Objects.Add(WeakAsset.Resolve());
								Objects.Add(Item->WeakObject.Resolve());
								DetailsView->SetObjects(Objects, true);
							})
							.OnContextMenuOpening(this, &SVoxelHeightmapList::OnContextMenuOpening)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(12.f, 6.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]
						{
							const UVoxelHeightmap* Asset = WeakAsset.Resolve();
							if (!Asset)
							{
								return INVTEXT("0 weightmaps");
							}

							return FText::FromString(FText::AsNumber(Asset->Weights.Num()).ToString() + " weightmaps");
						})
					]
				]
			]
			+ SSplitter::Slot()
			.Value(.7f)
			[
				DetailsView.ToSharedRef()
			]
		]
	];

	BuildList();
	ListView->SetSelection(Layers[0]);
}

void SVoxelHeightmapList::OnPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent)
{
	for (int32 Index = 0; Index < PropertyChangedEvent.GetNumObjectsBeingEdited(); Index++)
	{
		const UObject* Object = PropertyChangedEvent.GetObjectBeingEdited(Index);
		if (!Object)
		{
			continue;
		}

		if (const UVoxelHeightmap_Height* Heightmap = Cast<UVoxelHeightmap_Height>(Object))
		{
			if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_STATIC(UVoxelHeightmap_Height, Texture))
			{
				for (const TSharedPtr<FVoxelHeightStampLayer>& Layer : Layers)
				{
					if (Layer->WeakObject == Object)
					{
						Layer->AssetThumbnail->SetAsset(Heightmap->Texture.LoadSynchronous());
						break;
					}
				}
				return;
			}
			return;
		}
		if (const UVoxelHeightmap_Weight* Weightmap = Cast<UVoxelHeightmap_Weight>(Object))
		{
			if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_STATIC(UVoxelHeightmap_Weight, Texture))
			{
				for (const TSharedPtr<FVoxelHeightStampLayer>& Layer : Layers)
				{
					if (Layer->WeakObject == Object)
					{
						Layer->AssetThumbnail->SetAsset(Weightmap->Texture.LoadSynchronous());
						break;
					}
				}
				return;
			}
		}
	}
}

void SVoxelHeightmapList::PostUndo()
{
	const TSharedPtr<FVoxelHeightStampLayer> CurrentSelection = ListView->GetNumItemsSelected() > 0 ? ListView->GetSelectedItems()[0] : nullptr;
	BuildList();

	if (CurrentSelection)
	{
		for (const TSharedPtr<FVoxelHeightStampLayer>& Layer : Layers)
		{
			if (Layer->WeakObject == CurrentSelection->WeakObject)
			{
				ListView->SetSelection(Layer);
				ListView->RequestScrollIntoView(Layer);
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FReply SVoxelHeightmapList::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelHeightmapList::BuildList()
{
	ON_SCOPE_EXIT
	{
		ListView->RequestListRefresh();
	};

	Layers = {};

	UVoxelHeightmap* Asset = WeakAsset.Resolve();
	if (!ensure(Asset))
	{
		return;
	}

	Layers.Add(MakeShared<FVoxelHeightStampLayer>(
		Asset->Height,
		Asset->Height->Texture.LoadSynchronous(),
		-1));

	for (int32 Index = 0; Index < Asset->Weights.Num(); Index++)
	{
		UVoxelHeightmap_Weight* Weight = Asset->Weights[Index];
		if (!ensure(Weight))
		{
			continue;
		}

		Layers.Add(MakeShared<FVoxelHeightStampLayer>(
			Weight,
			Weight->Texture.LoadSynchronous(),
			Index));
	}
}

void SVoxelHeightmapList::BindCommands()
{
	CommandList->MapAction(
		FVoxelHeightStampListCommands::Get().AddWeightmap,
		FExecuteAction::CreateLambda([this]
		{
			AddLayer(nullptr, nullptr);
		}),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([this]
		{
			return ListView->GetNumItemsSelected() == 0;
		}));

	CommandList->MapAction(
		FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateLambda([this]
		{
			if (ListView->GetNumItemsSelected() == 0)
			{
				return;
			}

			const TSharedPtr<FVoxelHeightStampLayer> Layer = ListView->GetSelectedItems()[0];
			if (Layer->bHeightmap)
			{
				return;
			}

			if (const UVoxelHeightmap_Weight* Weightmap = Cast<UVoxelHeightmap_Weight>(Layer->WeakObject.Resolve()))
			{
				AddLayer(Weightmap, nullptr);
			}
		}),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([this]
		{
			if (ListView->GetNumItemsSelected() != 1)
			{
				return false;
			}

			return !ListView->GetSelectedItems()[0]->bHeightmap;
		}));

	CommandList->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateLambda([this]
		{
			if (ListView->GetNumItemsSelected() == 0)
			{
				return;
			}

			const TSharedPtr<FVoxelHeightStampLayer> Layer = ListView->GetSelectedItems()[0];
			if (Layer->bHeightmap)
			{
				return;
			}

			DeleteLayer(Layer);
		}),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([this]
		{
			if (ListView->GetNumItemsSelected() != 1)
			{
				return false;
			}

			return !ListView->GetSelectedItems()[0]->bHeightmap;
		}));
}

TSharedRef<ITableRow> SVoxelHeightmapList::MakeWidgetFromLayer(TSharedPtr<FVoxelHeightStampLayer> Layer, const TSharedRef<STableViewBase>& OwnerTable)
{
	const TSharedRef<SWidget> Widget =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(15.f, 3.f, 10.f, 3.f)
		[
			SNew(SBox)
			.WidthOverride(32.f)
			.HeightOverride(32.f)
			[
				Layer->AssetThumbnail->MakeThumbnailWidget()
			]
		]
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SVoxelDetailText)
				.Text_Lambda([Layer]
				{
					const UObject* Object = Layer->WeakObject.Resolve();
					if (!Object)
					{
						return INVTEXT("None");
					}

					if (Layer->bHeightmap)
					{
						return INVTEXT("Heightmap");
					}

					return FText::FromString("Weightmap " + FText::AsNumber(Layer->Index + 1).ToString());
				})
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Font_Lambda([Layer]
				{
					const UObject* Object = Layer->WeakObject.Resolve();
					if (!Object)
					{
						return FAppStyle::GetFontStyle("PropertyWindow.ItalicFont");
					}

					if (Layer->bHeightmap)
					{
						return FAppStyle::GetFontStyle("PropertyWindow.BoldFont");
					}

					return FAppStyle::GetFontStyle("PropertyWindow.NormalFont");
				})
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SVoxelDetailText)
				.Text_Lambda([Layer]() -> FText
				{
					UObject* Object = Layer->WeakObject.Resolve();

					if (const UVoxelHeightmap_Height* Heightmap = Cast<UVoxelHeightmap_Height>(Object))
					{
						const TSharedPtr<const FVoxelHeightmap_HeightData> Data = Heightmap->GetData();
						if (!Data)
						{
							return {};
						}

						return FText::FromString(
							LexToString(Data->SizeX) + "x" +
							LexToString(Data->SizeY) + " " +
							(Data->bIsUINT16 ? " 16 bit " : " float ") +
							FVoxelUtilities::BytesToString(Data->GetAllocatedSize()));
					}

					if (const UVoxelHeightmap_Weight* Weightmap = Cast<UVoxelHeightmap_Weight>(Object))
					{
						const TSharedPtr<const FVoxelHeightmap_WeightData> Data = Weightmap->GetData();
						if (!Data)
						{
							return {};
						}

						return FText::FromString(
							LexToString(Data->SizeX) + "x" +
							LexToString(Data->SizeY) + " " +
							"8 bits " +
							FVoxelUtilities::BytesToString(Data->GetAllocatedSize()) + "\n" +
							(Weightmap->Type == EVoxelHeightmapWeightType::AlphaBlended ? "Alpha blended" : "Weight blended") + "\t" +
							MakeVoxelObjectPtr(Weightmap->SurfaceType).GetReadableName());
					}

					ensure(false);
					return {};
				})
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		];

	return
		SNew(STableRow<TSharedPtr<FVoxelHeightStampLayer>>, OwnerTable)
		.Padding(4.f)
		.OnDragLeave_Lambda([](const FDragDropEvent& DragDropEvent)
		{
			const TSharedPtr<FVoxelHeightStampLayerDragDropOp> LayerDropOperation = DragDropEvent.GetOperationAs<FVoxelHeightStampLayerDragDropOp>();
			if (!LayerDropOperation)
			{
				return;
			}

			LayerDropOperation->SetValidTarget(false);
		})
		.OnCanAcceptDrop_Lambda([WeakWidget = MakeWeakPtr(Widget)](const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, TSharedPtr<FVoxelHeightStampLayer> TargetLayer) -> TOptional<EItemDropZone>
		{
			const TSharedPtr<FVoxelHeightStampLayerDragDropOp> LayerDropOperation = DragDropEvent.GetOperationAs<FVoxelHeightStampLayerDragDropOp>();
			const TSharedPtr<SWidget> WidgetToMoveOver = WeakWidget.Pin();
			if (!LayerDropOperation ||
				!WidgetToMoveOver)
			{
				return TOptional<EItemDropZone>();
			}

			const TSharedPtr<FVoxelHeightStampLayer> LayerToMove = LayerDropOperation->Layer.Pin();
			if (!LayerToMove)
			{
				return TOptional<EItemDropZone>();
			}

			const FGeometry& Geometry = WidgetToMoveOver->GetTickSpaceGeometry();
			const float LocalPointerY = Geometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition()).Y;
			const EItemDropZone OverrideDropZone = LocalPointerY < Geometry.GetLocalSize().Y * 0.5f ? EItemDropZone::AboveItem : EItemDropZone::BelowItem;

			const bool bIsValidDrop = !TargetLayer->bHeightmap || OverrideDropZone == EItemDropZone::BelowItem;

			LayerDropOperation->SetValidTarget(bIsValidDrop);

			if (!bIsValidDrop)
			{
				return TOptional<EItemDropZone>();
			}

			return OverrideDropZone;
		})
		.OnAcceptDrop_Lambda([this](const FDragDropEvent& DragDropEvent, const EItemDropZone DropZone, TSharedPtr<FVoxelHeightStampLayer> TargetLayer) -> FReply
		{
			UVoxelHeightmap* Asset = WeakAsset.Resolve();
			const TSharedPtr<FVoxelHeightStampLayerDragDropOp> LayerDropOperation = DragDropEvent.GetOperationAs<FVoxelHeightStampLayerDragDropOp>();
			if (!LayerDropOperation ||
				!ensure(Asset) ||
				!ensure(TargetLayer))
			{
				return FReply::Unhandled();
			}

			const TSharedPtr<FVoxelHeightStampLayer> LayerToMove = LayerDropOperation->Layer.Pin();
			const bool bIsValidDrop = !TargetLayer->bHeightmap || DropZone == EItemDropZone::BelowItem;
			if (!LayerToMove ||
				!bIsValidDrop)
			{
				return FReply::Unhandled();
			}

			if (TargetLayer == LayerToMove)
			{
				return FReply::Unhandled();
			}

			const int32 TargetIndex = FMath::Clamp(TargetLayer->Index + (DropZone == EItemDropZone::AboveItem ? 0 : 1), 0, Asset->Weights.Num());
			if (TargetIndex == LayerToMove->Index)
			{
				return FReply::Unhandled();
			}

			int32 NewSelectionIndex;
			{
				FVoxelTransaction Transaction(Asset, "Reorder Weightmaps");

				if (TargetIndex < Asset->Weights.Num())
				{
					int32 OriginalIndex = LayerToMove->Index;
					if (TargetIndex > OriginalIndex)
					{
						NewSelectionIndex = 0;
					}
					else
					{
						NewSelectionIndex = 1;
						OriginalIndex++;
					}

					NewSelectionIndex += Asset->Weights.Insert(Cast<UVoxelHeightmap_Weight>(LayerToMove->WeakObject.Resolve()), TargetIndex);
					Asset->Weights.RemoveAt(OriginalIndex);
				}
				else
				{
					Asset->Weights.RemoveAt(LayerToMove->Index);
					NewSelectionIndex = Asset->Weights.Add(Cast<UVoxelHeightmap_Weight>(LayerToMove->WeakObject.Resolve())) + 1;
				}
			}

			BuildList();

			if (Layers.IsValidIndex(NewSelectionIndex))
			{
				ListView->SetSelection(Layers[NewSelectionIndex]);
				ListView->RequestScrollIntoView(Layers[NewSelectionIndex]);
			}

			return FReply::Handled();
		})
		.OnDragDetected_Lambda([Layer](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
		{
			if (!MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) ||
				Layer->bHeightmap)
			{
				return FReply::Unhandled();
			}

			return FReply::Handled().BeginDragDrop(FVoxelHeightStampLayerDragDropOp::New(Layer));
		})
		[
			Widget
		];
}

TSharedPtr<SWidget> SVoxelHeightmapList::OnContextMenuOpening()
{
	if (ListView->GetNumItemsSelected() > 0 &&
		ListView->GetSelectedItems()[0]->bHeightmap)
	{
		return SNullWidget::NullWidget;
	}

	FMenuBuilder MenuBuilder(true, CommandList);

	MenuBuilder.BeginSection("BasicOperations");
	{
		MenuBuilder.AddMenuEntry(FVoxelHeightStampListCommands::Get().AddWeightmap, {}, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FAppStyle::GetAppStyleSetName(), "DataTableEditor.Add.Small"));
		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate);
		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Delete);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelHeightmapList::AddLayer(const UVoxelHeightmap_Weight* WeightToCopy, UTexture2D* WeightmapTexture)
{
	UVoxelHeightmap* Asset = WeakAsset.Resolve();
	if (!ensure(Asset))
	{
		return;
	}

	UVoxelHeightmap_Weight* Weight;
	{
		FVoxelTransaction Transaction(Asset, "Add Weightmap");
		if (WeightToCopy)
		{
			Weight = DuplicateObject(WeightToCopy, Asset);
		}
		else
		{
			Weight = NewObject<UVoxelHeightmap_Weight>(Asset, NAME_None, RF_Transactional);
		}

		if (WeightmapTexture)
		{
			Weight->Texture = WeightmapTexture;
		}

		Asset->Weights.Add(Weight);
		(void)Asset->MarkPackageDirty();
	}

	const TSharedRef<FVoxelHeightStampLayer> NewWeightmap = MakeShared<FVoxelHeightStampLayer>(Weight, Weight->Texture.LoadSynchronous(), Asset->Weights.Num() - 1);
	Layers.Add(NewWeightmap);

	ListView->RequestListRefresh();

	ListView->SetSelection(NewWeightmap);
	ListView->RequestScrollIntoView(NewWeightmap);
}

void SVoxelHeightmapList::DeleteLayer(const TSharedPtr<FVoxelHeightStampLayer>& Layer)
{
	UVoxelHeightmap* Asset = WeakAsset.Resolve();
	if (!ensure(Asset))
	{
		return;
	}

	if (!Asset->Weights.IsValidIndex(Layer->Index))
	{
		return;
	}

	const int32 Index = Layers.IndexOfByKey(Layer);

	{
		FVoxelTransaction Transaction(Asset, "Remove Weightmap");
		Asset->Weights.RemoveAt(Layer->Index);
		(void)Asset->MarkPackageDirty();
	}

	BuildList();

	const int32 IndexToSelect = FMath::Clamp(Index, 0, Layers.Num() - 1);
	if (!Layers.IsValidIndex(IndexToSelect))
	{
		return;
	}

	const TSharedPtr<FVoxelHeightStampLayer> NewSelection = Layers[IndexToSelect];
	ListView->SetSelection(NewSelection);
	ListView->RequestScrollIntoView(NewSelection);
}