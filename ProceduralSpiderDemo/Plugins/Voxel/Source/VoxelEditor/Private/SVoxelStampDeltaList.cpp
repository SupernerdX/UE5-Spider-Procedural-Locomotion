// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelStampDeltaList.h"
#include "VoxelLayerStack.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"
#include "Shape/VoxelShapeStamp.h"
#include "WorkspaceMenuStructure.h"
#include "Styling/SlateIconFinder.h"
#include "WorkspaceMenuStructureModule.h"

VOXEL_INITIALIZE_STYLE(VoxelStampDeltaListStyle)
{
	const FButtonStyle Button = FAppStyle::GetWidgetStyle<FButtonStyle>("Button");
	const FSlateColor SelectionColor = FAppStyle::GetSlateColor("SelectionColor");
	const FSlateColor SelectionColor_Inactive = FAppStyle::GetSlateColor("SelectionColor_Inactive");
	const FSlateColor SelectionColor_Pressed = FAppStyle::GetSlateColor("SelectionColor_Pressed");

	Set("Voxel.OpenInExternalEditor", FButtonStyle(Button)
		.SetNormal(CORE_IMAGE_BRUSH_SVG("Starship/Common/OpenInExternalEditor", CoreStyleConstants::Icon16x16))
		.SetHovered(CORE_IMAGE_BRUSH_SVG("Starship/Common/OpenInExternalEditor", CoreStyleConstants::Icon16x16, SelectionColor))
		.SetPressed(CORE_IMAGE_BRUSH_SVG("Starship/Common/OpenInExternalEditor", CoreStyleConstants::Icon16x16, SelectionColor_Pressed))
		.SetDisabled(CORE_IMAGE_BRUSH_SVG("Starship/Common/OpenInExternalEditor", CoreStyleConstants::Icon16x16, SelectionColor_Inactive)));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStampDeltaListTabManager* GVoxelStampDeltaListTabManager = new FVoxelStampDeltaListTabManager();

void FVoxelStampDeltaListTabManager::Initialize()
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();

	TabManager->RegisterNomadTabSpawner(StampDeltaListTabId, MakeLambdaDelegate([=](const FSpawnTabArgs& SpawnTabArgs)
	{
		return
			SNew(SDockTab)
			.TabRole(NomadTab)
			.Label(INVTEXT("Voxel Stamps Delta List"))
			.ToolTipText(INVTEXT("Preview Voxel Stamps list in the viewport"))
			[
				SNew(SVoxelStampDeltaList)
				.IsTab(true)
			];
	}))
	.SetDisplayName(INVTEXT("Voxel Stamps Delta List"))
	.SetIcon(FSlateIcon(FVoxelEditorStyle::GetStyleSetName(), "VoxelIcon"))
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
}

void FVoxelStampDeltaListTabManager::OpenStampDeltaList(const TVoxelArray<FVoxelStampDelta>& List) const
{
	const TSharedPtr<SDockTab> NewTab = FGlobalTabmanager::Get()->TryInvokeTab(StampDeltaListTabId);
	if (!ensure(NewTab))
	{
		return;
	}

	const TSharedRef<SVoxelStampDeltaList> StampDeltaList = StaticCastSharedRef<SVoxelStampDeltaList>(NewTab->GetContent());
	StampDeltaList->UpdateStamps(List);
}

DEFINE_PRIVATE_ACCESS(FTabSpawnerEntry, OnFindTabToReuse)
DEFINE_PRIVATE_ACCESS(FTabSpawnerEntry, SpawnedTabPtr)

bool FVoxelStampDeltaListTabManager::IsTabOpened() const
{
	const TSharedPtr<FTabSpawnerEntry> Spawner = FGlobalTabmanager::Get()->FindTabSpawnerFor(StampDeltaListTabId);
	if (!Spawner)
	{
		return false;
	}

	const FOnFindTabToReuse& OnFindTabToReuse = PrivateAccess::OnFindTabToReuse(*Spawner);
	const TWeakPtr<SDockTab> SpawnedTabPtr = PrivateAccess::SpawnedTabPtr(*Spawner);

	const TSharedPtr<SDockTab> ExistingTab =
		OnFindTabToReuse.IsBound()
		? OnFindTabToReuse.Execute(StampDeltaListTabId)
		: SpawnedTabPtr.Pin();
	return ExistingTab != nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelStampDeltaListEntry::Construct(
	const FArguments& Args,
	const TSharedRef<STableViewBase>& OwnerTableView,
	const FVoxelStampDelta& NewStampDelta)
{
	StampDelta = NewStampDelta;

	FSuperRowType::Construct(FSuperRowType::FArguments(), OwnerTableView);
}

TSharedRef<SWidget> SVoxelStampDeltaListEntry::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (!ensure(StampDelta.Stamp))
	{
		return SNullWidget::NullWidget;
	}

	const FVoxelStampRuntime& Stamp = *StampDelta.Stamp;

	if (ColumnName == "StampName")
	{
		const AActor* Actor = Stamp.GetActor();
		const int32 InstanceIndex = Stamp.GetInstanceIndex();

		return
			SNew(SBox)
			.MinDesiredHeight(24.f)
			.Padding(4.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.f, 0.f)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(FSlateIconFinder::FindIconBrushForClass(Actor ? Actor->GetClass() : nullptr))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 1.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(FText::FromString(Stamp.GetComponent().GetReadableName()))
							.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(FText::FromString(" [" + FText::AsNumber(InstanceIndex).ToString() + "]"))
							.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.Visibility(InstanceIndex == -1 ? EVisibility::Collapsed : EVisibility::Visible)
						]
					]
				]
			];
	}

	if (ColumnName == "Layer")
	{
		return
			SNew(SBox)
			.Padding(4.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(StampDelta.Layer.Layer.GetReadableName()))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			];
	}

	const TAttribute<FText> GetText = INLINE_LAMBDA -> TAttribute<FText>
	{
		if (ColumnName == "BlendMode")
		{
			if (const FVoxelHeightStamp* HeightStamp = Stamp.GetStamp().As<FVoxelHeightStamp>())
			{
				return GetEnumDisplayName(HeightStamp->BlendMode);
			}

			return GetEnumDisplayName(Stamp.GetStamp().AsChecked<FVoxelVolumeStamp>().BlendMode);
		}
		if (ColumnName == "Type")
		{
			return Stamp.IsA<FVoxelHeightStampRuntime>() ? INVTEXT("Height") : INVTEXT("Volume");
		}
		if (ColumnName == "Priority")
		{
			return FText::AsNumber(Stamp.GetStamp().Priority);
		}
		if (ColumnName == "Value")
		{
			return FText::AsNumber(StampDelta.DistanceAfter);
		}
		if (ColumnName == "Smoothness")
		{
			return FText::AsNumber(Stamp.GetStamp().Smoothness);
		}
		if (ColumnName == "Behavior")
		{
			return StaticEnumFast<EVoxelStampBehavior>()->GetDisplayNameTextByValue(uint8(Stamp.GetStamp().Behavior));
		}
		if (ColumnName == "Object")
		{
			if (const FVoxelShapeStamp* ShapeStamp = Stamp.GetStamp().As<FVoxelShapeStamp>())
			{
				if (ShapeStamp->Shape.IsValid())
				{
					return FText::FromString(ShapeStamp->Shape.GetScriptStruct()->GetName());
				}
				return INVTEXT("None");
			}

			return FText::FromString(FVoxelUtilities::GetReadableName(Stamp.GetStamp().GetAsset()));
		}

		return {};
	};

	return
		SNew(SBox)
		.Padding(4.f, 0.f)
		[
			SNew(STextBlock)
			.Text(GetText)
			.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.ColorAndOpacity(FSlateColor::UseForeground())
		];
}

void SVoxelStampDeltaListEntry::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FSuperRowType::OnMouseEnter(MyGeometry, MouseEvent);

	if (StampDelta.Stamp)
	{
		StampDelta.Stamp->SelectComponent_EditorOnly();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelStampDeltaList::Construct(const FArguments& Args)
{
	TArray<FName> HiddenColumnsList;
	{
		TArray<FString> HiddenColumnStrings;
		INLINE_LAMBDA
		{
			// Set defaults
			if (!GConfig->GetSection(TEXT("VoxelStampDeltaList"), false, GEditorPerProjectIni))
			{
				HiddenColumnsList.Add("Behavior");
				HiddenColumnsList.Add("Object");
				HiddenColumnsList.Add("Smoothness");
				return;
			}

			GConfig->GetArray(TEXT("VoxelStampDeltaList"), TEXT("HiddenColumns"), HiddenColumnStrings, GEditorPerProjectIni);
			for (const FString& ColumnId : HiddenColumnStrings)
			{
				HiddenColumnsList.Add(FName(ColumnId));
			}
		};
	}

	ListView =
		SNew(SListView<TSharedPtr<FVoxelStampDelta>>)
		.ListViewStyle(&FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("PropertyTable.InViewport.ListView"))
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&StampDeltaArray)
		.HeaderRow(
			SAssignNew(HeaderRow, SHeaderRow)
			.CanSelectGeneratedColumn(true)
			.HiddenColumnsList(HiddenColumnsList)
			.OnHiddenColumnsListChanged_Lambda([this]
			{
				TArray<FString> HiddenColumnStrings;
				for (const FName ColumnId : HeaderRow->GetHiddenColumnIds())
				{
					HiddenColumnStrings.Add(ColumnId.ToString());
				}

				GConfig->SetArray(TEXT("VoxelStampDeltaList"), TEXT("HiddenColumns"), HiddenColumnStrings, GEditorPerProjectIni);
			})

			+ SHeaderRow::Column("StampName")
			.HAlignCell(HAlign_Left)
			.VAlignCell(VAlign_Center)
			.VAlignHeader(VAlign_Center)
			.FillWidth(0.5f)
			.ShouldGenerateWidget(true)
			.DefaultLabel(INVTEXT("Stamp Name"))

			+ SHeaderRow::Column("Object")
			.HAlignCell(HAlign_Center)
			.VAlignCell(VAlign_Center)
			.VAlignHeader(VAlign_Center)
			.FillWidth(0.1f)
			.DefaultLabel(INVTEXT("Object"))

			+ SHeaderRow::Column("BlendMode")
			.HAlignCell(HAlign_Center)
			.VAlignCell(VAlign_Center)
			.VAlignHeader(VAlign_Center)
			.FillWidth(0.1f)
			.DefaultLabel(INVTEXT("Blend Mode"))

			+ SHeaderRow::Column("Smoothness")
			.HAlignCell(HAlign_Center)
			.VAlignCell(VAlign_Center)
			.VAlignHeader(VAlign_Center)
			.FillWidth(0.1f)
			.DefaultLabel(INVTEXT("Smoothness"))

			+ SHeaderRow::Column("Behavior")
			.HAlignCell(HAlign_Center)
			.VAlignCell(VAlign_Center)
			.VAlignHeader(VAlign_Center)
			.FillWidth(0.1f)
			.DefaultLabel(INVTEXT("Behavior"))

			+ SHeaderRow::Column("Value")
			.HAlignCell(HAlign_Center)
			.VAlignCell(VAlign_Center)
			.VAlignHeader(VAlign_Center)
			.FillWidth(0.1f)
			.DefaultLabel(INVTEXT("Value"))

			+ SHeaderRow::Column("Type")
			.HAlignCell(HAlign_Center)
			.VAlignCell(VAlign_Center)
			.VAlignHeader(VAlign_Center)
			.FillWidth(0.1f)
			.DefaultLabel(INVTEXT("Type"))

			+ SHeaderRow::Column("Priority")
			.HAlignCell(HAlign_Center)
			.VAlignCell(VAlign_Center)
			.VAlignHeader(VAlign_Center)
			.FillWidth(0.1f)
			.DefaultLabel(INVTEXT("Priority"))

			+ SHeaderRow::Column("Layer")
			.HAlignCell(HAlign_Center)
			.VAlignCell(VAlign_Center)
			.VAlignHeader(VAlign_Center)
			.FillWidth(0.2f)
			.DefaultLabel(INVTEXT("Layer"))
		)
		.OnGenerateRow_Lambda([this](const TSharedPtr<FVoxelStampDelta>& StampDelta, const TSharedRef<STableViewBase>& OwnerTable)
		{
			return SNew(SVoxelStampDeltaListEntry, OwnerTable, *StampDelta);
		})
		.OnSelectionChanged_Lambda([this](const TSharedPtr<FVoxelStampDelta> StampDelta, ESelectInfo::Type SelectInfo)
		{
			if (!StampDelta)
			{
				return;
			}

			FSlateApplication::Get().DismissAllMenus();

			const FScopedTransaction Transaction(INVTEXT("Clicking on Components"));
			StampDelta->Stamp->SelectComponent_EditorOnly();
		});

	if (Args._IsTab)
	{
		ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					ListView.ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.Padding(4.f)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(INVTEXT("Num stamps: "))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]
						{
							return FText::FromString(LexToString(StampDeltaArray.Num()));
						})
					]
				]
			]
		];

		return;
	}
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					ListView.ToSharedRef()
				]
			]
		]
		+ SVerticalBox::Slot()
		.Padding(4.f, 8.f, 4.f, 0.f)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(INVTEXT("Num stamps: "))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text_Lambda([this]
				{
					return FText::FromString(LexToString(StampDeltaArray.Num()));
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.ButtonStyle(FVoxelEditorStyle::Get(), "Voxel.OpenInExternalEditor")
				.ToolTipText(INVTEXT("Open in external tab"))
				.OnClicked_Lambda([this]
				{
					TVoxelArray<FVoxelStampDelta> List;
					List.Reserve(StampDeltaArray.Num());
					for (const TSharedPtr<FVoxelStampDelta>& StampDelta : StampDeltaArray)
					{
						List.Add(*StampDelta);
					}

					GVoxelStampDeltaListTabManager->OpenStampDeltaList(List);
					FSlateApplication::Get().DismissAllMenus();
					return FReply::Handled();
				})
			]
		]
	];
}

void SVoxelStampDeltaList::UpdateStamps(const TVoxelArray<FVoxelStampDelta>& List)
{
	StampDeltaArray.Reset();

	for (const FVoxelStampDelta& StampDelta : List)
	{
		StampDeltaArray.Add(MakeSharedCopy(StampDelta));
	}

	if (ListView)
	{
		ListView->RequestListRefresh();
	}
}