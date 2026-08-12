// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphStackLayerSelector.h"
#include "VoxelLayer.h"
#include "VoxelLayerStack.h"
#include "SVoxelMenuAssetPicker.h"
#include "Subsystems/AssetEditorSubsystem.h"

void SVoxelGraphStackLayerSelector::Construct(const FArguments& InArgs)
{
	StackLayerAttribute = InArgs._StackLayer;
	OnUpdateStackLayer = InArgs._OnUpdateStackLayer;
	LayerType = InArgs._LayerType;

	{
		FVoxelStackLayer StackLayer = StackLayerAttribute.Get();
		StackThumbnail = MakeShared<FAssetThumbnail>(StackLayer.Stack, InArgs._ThumbnailSize.X * 0.25f, InArgs._ThumbnailSize.Y * 0.25f, InArgs._ThumbnailPool);
		LayerThumbnail = MakeShared<FAssetThumbnail>(StackLayer.Layer, InArgs._ThumbnailSize.X, InArgs._ThumbnailSize.Y, InArgs._ThumbnailPool);
	}

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(0.f, 3.f, 5.f, 0.f)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Visibility(EVisibility::SelfHitTestInvisible)
			.Padding(FMargin(0.f, 0.f, 4.f, 4.f))
			.BorderImage(FAppStyle::Get().GetBrush("PropertyEditor.AssetTileItem.DropShadow"))
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.Padding(1.f)
				[
					SAssignNew(ThumbnailBorder, SBorder)
					.Padding(0.f)
					.BorderImage(FStyleDefaults::GetNoBrush())
					.OnMouseDoubleClick_Lambda([this](const FGeometry&, const FPointerEvent&)
					{
						const FVoxelStackLayer StackLayer = StackLayerAttribute.Get();
						UObject* ObjectToOpen = StackLayer.Layer;
						if (!ObjectToOpen)
						{
							ObjectToOpen = StackLayer.Stack;
						}

						if (!ObjectToOpen)
						{
							return FReply::Handled();
						}

						UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
						if (!AssetEditorSubsystem)
						{
							return FReply::Handled();
						}

						FText ErrorMsg;
						if (AssetEditorSubsystem->CanOpenEditorForAsset(ObjectToOpen, EAssetTypeActivationOpenedMethod::Edit, &ErrorMsg))
						{
							AssetEditorSubsystem->OpenEditorForAsset(ObjectToOpen);
						}
						else if (AssetEditorSubsystem->CanOpenEditorForAsset(ObjectToOpen, EAssetTypeActivationOpenedMethod::View, &ErrorMsg))
						{
							AssetEditorSubsystem->OpenEditorForAsset(ObjectToOpen, EToolkitMode::Standalone, TSharedPtr<IToolkitHost>(), true, EAssetTypeActivationOpenedMethod::View);
						}

						return FReply::Handled();
					})
					[
						SNew(SBox)
						.WidthOverride(InArgs._ThumbnailSize.X)
						.HeightOverride(InArgs._ThumbnailSize.Y)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								LayerThumbnail->MakeThumbnailWidget()
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Bottom)
							[
								SNew(SBox)
								.WidthOverride(InArgs._ThumbnailSize.X * 0.25f)
								.HeightOverride(InArgs._ThumbnailSize.X * 0.25f)
								[
									StackThumbnail->MakeThumbnailWidget()
								]
							]
						]
					]
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image_Lambda([this]
					{
						return ThumbnailBorder->IsHovered() ? FAppStyle::Get().GetBrush(STATIC_FNAME("PropertyEditor.AssetThumbnailBorderHovered")) : FAppStyle::Get().GetBrush(STATIC_FNAME("PropertyEditor.AssetThumbnailBorder"));
					})
					.Visibility(EVisibility::SelfHitTestInvisible)
				]
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(0.f)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.f)
				[
					SAssignNew(StackComboButton, SComboButton)
					.OnGetMenuContent_Lambda([this]
					{
						const FVoxelStackLayer StackLayer = StackLayerAttribute.Get();

						return
							SNew(SVoxelMenuAssetPicker)
							.InitialObject(FAssetData(StackLayer.Stack))
							.PropertyHandle(nullptr)
							.ForceShowPluginContent(true)
							.AllowedClasses({ UVoxelLayerStack::StaticClass() })
							.OnSet_Lambda([this](const FAssetData& AssetData)
							{
								SetStack(Cast<UVoxelLayerStack>(AssetData.GetAsset()));
							})
							.OnClose(MakeWeakPtrDelegate(this, [this]
							{
								if (StackComboButton)
								{
									StackComboButton->SetIsOpen(false);
								}
							}));
					})
					.OnMenuOpenChanged_Lambda([this](const bool bOpen)
					{
						if (bOpen ||
							!StackComboButton)
						{
							return;
						}

						StackComboButton->SetMenuContent(SNullWidget::NullWidget);
					})
					.ButtonContent()
					[
						SNew(STextBlock)
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
						.Text_Lambda([this]
						{
							const FVoxelStackLayer StackLayer = StackLayerAttribute.Get();
							return GetAssetName(StackLayer.Stack);
						})
					]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(2.f, 0.f)
				[
					PropertyCustomizationHelpers::MakeUseSelectedButton(MakeLambdaDelegate([this]
					{
						FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

						if (UObject* Selection = GEditor->GetSelectedObjects()->GetTop(UVoxelLayerStack::StaticClass()))
						{
							SetStack(Cast<UVoxelLayerStack>(Selection));
						}
					}))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(2.f, 0.f)
				[
					PropertyCustomizationHelpers::MakeBrowseButton(MakeLambdaDelegate([this]
					{
						FVoxelStackLayer StackLayer = StackLayerAttribute.Get();
						if (!StackLayer.Stack)
						{
							return;
						}

						TArray<FAssetData> AssetDataList;
						AssetDataList.Add({ FAssetData(StackLayer.Stack) });

						GEditor->SyncBrowserToObjects(AssetDataList);
					}))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.f)
				[
					SAssignNew(LayerComboButton, SComboButton)
					.OnGetMenuContent_Lambda([this]
					{
						const FVoxelStackLayer StackLayer = StackLayerAttribute.Get();

						TArray<const UClass*> AllowedClasses = { UVoxelHeightLayer::StaticClass(), UVoxelVolumeLayer::StaticClass() };
						if (LayerType.IsSet())
						{
							switch (LayerType.GetValue())
							{
							case EVoxelLayerType::Height: AllowedClasses = { UVoxelHeightLayer::StaticClass() }; break;
							case EVoxelLayerType::Volume: AllowedClasses = { UVoxelVolumeLayer::StaticClass() }; break;
							}
						}

						return
							SNew(SVoxelMenuAssetPicker)
							.InitialObject(FAssetData(StackLayer.Layer))
							.PropertyHandle(nullptr)
							.ForceShowPluginContent(true)
							.AllowedClasses(AllowedClasses)
							.OnShouldFilterAsset_Lambda([this, Stack = StackLayer.Stack](const FAssetData& AssetData)
							{
								return !CanSelectLayer(Stack, AssetData);
							})
							.OnSet_Lambda([this](const FAssetData& AssetData)
							{
								SetLayer(Cast<UVoxelLayer>(AssetData.GetAsset()));
							})
							.OnClose(MakeWeakPtrDelegate(this, [this]
							{
								if (LayerComboButton)
								{
									LayerComboButton->SetIsOpen(false);
								}
							}));
					})
					.OnMenuOpenChanged_Lambda([this](const bool bOpen)
					{
						if (bOpen ||
							!LayerComboButton)
						{
							return;
						}

						LayerComboButton->SetMenuContent(SNullWidget::NullWidget);
					})
					.ButtonContent()
					[
						SNew(STextBlock)
						.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
						.Text_Lambda([this]
						{
							const FVoxelStackLayer StackLayer = StackLayerAttribute.Get();
							return GetAssetName(StackLayer.Layer);
						})
					]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(2.f, 0.f)
				[
					PropertyCustomizationHelpers::MakeUseSelectedButton(MakeLambdaDelegate([this]
					{
						const FVoxelStackLayer StackLayer = StackLayerAttribute.Get();
						if (!StackLayer.Stack)
						{
							return;
						}

						FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

						const UClass* TargetClass = UVoxelLayer::StaticClass();
						if (LayerType.IsSet())
						{
							switch (LayerType.GetValue())
							{
							case EVoxelLayerType::Height: TargetClass = UVoxelHeightLayer::StaticClass(); break;
							case EVoxelLayerType::Volume: TargetClass = UVoxelVolumeLayer::StaticClass(); break;
							}
						}

						UObject* Selection = GEditor->GetSelectedObjects()->GetTop(TargetClass);
						if (!Selection ||
							!CanSelectLayer(StackLayer.Stack, Selection))
						{
							return;
						}

						SetLayer(Cast<UVoxelLayer>(Selection));
					}))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				.Padding(2.f, 0.f)
				[
					PropertyCustomizationHelpers::MakeBrowseButton(MakeLambdaDelegate([this]
					{
						FVoxelStackLayer StackLayer = StackLayerAttribute.Get();
						if (!StackLayer.Layer)
						{
							return;
						}

						TArray<FAssetData> AssetDataList;
						AssetDataList.Add({ FAssetData(StackLayer.Layer) });

						GEditor->SyncBrowserToObjects(AssetDataList);
					}))
				]
			]
		]
	];
}

void SVoxelGraphStackLayerSelector::SetStack(UVoxelLayerStack* NewStack)
{
	FVoxelStackLayer StackLayer = StackLayerAttribute.Get();
	StackLayer.Stack = NewStack;

	if (NewStack)
	{
		if (!LayerType.IsSet())
		{
			if (!NewStack->VolumeLayers.Contains(StackLayer.Layer) &&
				!NewStack->HeightLayers.Contains(StackLayer.Layer))
			{
				if (NewStack->VolumeLayers.Num() > 0)
				{
					StackLayer.Layer = NewStack->VolumeLayers[0];
				}
				else if (NewStack->HeightLayers.Num() > 0)
				{
					StackLayer.Layer = NewStack->HeightLayers[0];
				}
				else
				{
					StackLayer.Layer = nullptr;
				}
			}
		}
		else if (LayerType == EVoxelLayerType::Height)
		{
			if (!NewStack->HeightLayers.Contains(StackLayer.Layer))
			{
				if (NewStack->HeightLayers.Num() > 0)
				{
					StackLayer.Layer = NewStack->HeightLayers[0];
				}
				else
				{
					StackLayer.Layer = nullptr;
				}
			}
		}
		else if (ensure(LayerType == EVoxelLayerType::Volume))
		{
			if (!NewStack->VolumeLayers.Contains(StackLayer.Layer))
			{
				if (NewStack->VolumeLayers.Num() > 0)
				{
					StackLayer.Layer = NewStack->VolumeLayers[0];
				}
				else
				{
					StackLayer.Layer = nullptr;
				}
			}
		}
	}
	else
	{
		StackLayer.Layer = nullptr;
	}

	if (StackThumbnail &&
		StackThumbnail->GetAsset() != NewStack)
	{
		StackThumbnail->SetAsset(NewStack);
		StackThumbnail->RefreshThumbnail();
	}

	if (LayerThumbnail &&
		LayerThumbnail->GetAsset() != StackLayer.Layer)
	{
		LayerThumbnail->SetAsset(StackLayer.Layer);
		LayerThumbnail->RefreshThumbnail();
	}

	OnUpdateStackLayer.ExecuteIfBound(StackLayer);
}

void SVoxelGraphStackLayerSelector::SetLayer(UVoxelLayer* NewLayer)
{
	FVoxelStackLayer StackLayer = StackLayerAttribute.Get();
	StackLayer.Layer = NewLayer;

	if (LayerThumbnail &&
		LayerThumbnail->GetAsset() != NewLayer)
	{
		LayerThumbnail->SetAsset(NewLayer);
		LayerThumbnail->RefreshThumbnail();
	}

	OnUpdateStackLayer.ExecuteIfBound(StackLayer);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FText SVoxelGraphStackLayerSelector::GetAssetName(const UObject* Asset) const
{
	FText Name = INVTEXT("None");
	if (!Asset)
	{
		return Name;
	}

	return FText::AsCultureInvariant(Asset->GetName());
}

bool SVoxelGraphStackLayerSelector::CanSelectLayer(const UVoxelLayerStack* Stack, const FAssetData& AssetData) const
{
	if (!Stack)
	{
		return false;
	}

	return
		Stack->HeightLayers.Contains(AssetData.GetAsset()) ||
		Stack->VolumeLayers.Contains(AssetData.GetAsset());
}