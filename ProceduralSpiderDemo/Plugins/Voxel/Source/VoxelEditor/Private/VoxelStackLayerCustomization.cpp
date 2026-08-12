// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelLayer.h"
#include "VoxelLayerStack.h"
#include "VoxelStackLayer.h"
#include "Subsystems/AssetEditorSubsystem.h"

template<typename T>
class TVoxelStackLayerCustomization
	: public FVoxelPropertyTypeCustomizationBase
	, public FVoxelTicker
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		BaseHandle = PropertyHandle;

		if constexpr (std::is_same_v<T, FVoxelStackLayer>)
		{
			StackHandle = PropertyHandle->GetChildHandleStatic(FVoxelStackLayer, Stack);
			LayerHandle = PropertyHandle->GetChildHandleStatic(FVoxelStackLayer, Layer);
		}
		else if constexpr (std::is_same_v<T, FVoxelStackHeightLayer>)
		{
			StackHandle = PropertyHandle->GetChildHandleStatic(FVoxelStackHeightLayer, Stack);
			LayerHandle = PropertyHandle->GetChildHandleStatic(FVoxelStackHeightLayer, Layer);
		}
		else if constexpr (std::is_same_v<T, FVoxelStackVolumeLayer>)
		{
			StackHandle = PropertyHandle->GetChildHandleStatic(FVoxelStackVolumeLayer, Stack);
			LayerHandle = PropertyHandle->GetChildHandleStatic(FVoxelStackVolumeLayer, Layer);
		}

		{
			UObject* Object = nullptr;
			StackHandle->GetValue(Object);
			StackThumbnail = MakeShared<FAssetThumbnail>(Object, 16.f, 16.f, CustomizationUtils.GetThumbnailPool());

			Object = nullptr;
			LayerHandle->GetValue(Object);
			LayerThumbnail = MakeShared<FAssetThumbnail>(Object, 48.f, 48.f, CustomizationUtils.GetThumbnailPool());
		}

		HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(250.f)
		.MaxDesiredWidth(250.f)
		[
			SNew(SBox)
			.MinDesiredWidth(250.f)
			.MaxDesiredWidth(250.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0.f,3.f,5.f,0.f)
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.Visibility(EVisibility::SelfHitTestInvisible)
					.Padding(0.f, 0.f, 4.f, 4.f)
					.BorderImage(FAppStyle::Get().GetBrush("PropertyEditor.AssetTileItem.DropShadow"))
					[
						SNew(SOverlay)
						+SOverlay::Slot()
						.Padding(1.f)
						[
							SAssignNew(ThumbnailBorder, SBorder)
							.Padding(0.f)
							.BorderImage(FStyleDefaults::GetNoBrush())
							.OnMouseDoubleClick_Lambda([this](const FGeometry&, const FPointerEvent&)
							{
								if (!LayerHandle ||
									!LayerHandle->IsValidHandle() ||
									!StackHandle ||
									!StackHandle->IsValidHandle())
								{
									return FReply::Handled();
								}

								UObject* Object = nullptr;
								LayerHandle->GetValue(Object);

								if (!Object)
								{
									StackHandle->GetValue(Object);
								}

								if (!Object)
								{
									return FReply::Handled();
								}

								UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
								if (!AssetEditorSubsystem)
								{
									return FReply::Handled();
								}

								FText ErrorMsg;
								if (AssetEditorSubsystem->CanOpenEditorForAsset(Object, EAssetTypeActivationOpenedMethod::Edit, &ErrorMsg))
								{
									AssetEditorSubsystem->OpenEditorForAsset(Object);
								}
								else if (AssetEditorSubsystem->CanOpenEditorForAsset(Object, EAssetTypeActivationOpenedMethod::View, &ErrorMsg))
								{
									AssetEditorSubsystem->OpenEditorForAsset(Object, EToolkitMode::Standalone, TSharedPtr<IToolkitHost>(), true, EAssetTypeActivationOpenedMethod::View);
								}

								return FReply::Handled();
							})
							[
								SNew(SBox)
								.ToolTipText(PropertyHandle->GetToolTipText())
								.WidthOverride(48.f)
								.HeightOverride(48.f)
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
									    .WidthOverride(16.f)
									    .HeightOverride(16.f)
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
								static const FName HoveredBorderName("PropertyEditor.AssetThumbnailBorderHovered");
								static const FName RegularBorderName("PropertyEditor.AssetThumbnailBorder");

								return ThumbnailBorder->IsHovered() ? FAppStyle::Get().GetBrush(HoveredBorderName) : FAppStyle::Get().GetBrush(RegularBorderName);
							})
							.Visibility(EVisibility::SelfHitTestInvisible)
						]
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(0.f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						CreateStackPicker()
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						CreateLayerPicker()
					]
				]
			]
		];
	}

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
	}

private:
	void UpdateLayer(const UVoxelLayerStack* Stack, UObject* Layer) const
	{
		if constexpr (std::is_same_v<T, FVoxelStackLayer>)
		{
			if (Layer)
			{
				if (Stack->HeightLayers.Contains(Layer) ||
					Stack->VolumeLayers.Contains(Layer))
				{
					return;
				}
			}

			UVoxelLayer* NewLayer = nullptr;
			if (Stack->VolumeLayers.Num() > 0)
			{
				NewLayer = Stack->VolumeLayers[0];
			}
			else if (Stack->HeightLayers.Num() > 0)
			{
				NewLayer = Stack->HeightLayers[0];
			}

			LayerHandle->SetValue(NewLayer);
		}
		else if constexpr (std::is_same_v<T, FVoxelStackHeightLayer>)
		{
			if (Layer)
			{
				if (Stack->HeightLayers.Contains(Layer))
				{
					return;
				}
			}

			UVoxelHeightLayer* NewLayer = nullptr;
			if (Stack->HeightLayers.Num() > 0)
			{
				NewLayer = Stack->HeightLayers[0];
			}

			LayerHandle->SetValue(NewLayer);
		}
		else if constexpr (std::is_same_v<T, FVoxelStackVolumeLayer>)
		{
			if (Layer)
			{
				if (Stack->VolumeLayers.Contains(Layer))
				{
					return;
				}
			}

			UVoxelVolumeLayer* NewLayer = nullptr;
			if (Stack->VolumeLayers.Num() > 0)
			{
				NewLayer = Stack->VolumeLayers[0];
			}

			LayerHandle->SetValue(NewLayer);
		}
	}

	TSharedRef<SWidget> CreateStackPicker()
	{
		return
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1.f)
			[
				SAssignNew(StackComboButton, SComboButton)
				.OnGetMenuContent_Lambda([this]() -> TSharedRef<SWidget>
				{
					if (!StackHandle ||
						!StackHandle->IsValidHandle())
					{
						return SNullWidget::NullWidget;
					}

					UObject* Object;
					StackHandle->GetValue(Object);

					return PropertyCustomizationHelpers::MakeAssetPickerWithMenu(
						Object,
						true,
						TArray<const UClass*>{ UVoxelLayerStack::StaticClass() },
						{},
						{},
						{},
						MakeLambdaDelegate([this](const FAssetData& AssetData)
						{
							if (!StackHandle ||
								!StackHandle->IsValidHandle())
							{
								return;
							}

							StackHandle->SetValue(AssetData);

							const UVoxelLayerStack* Stack = Cast<UVoxelLayerStack>(AssetData.GetAsset());

							if (!LayerHandle ||
								!LayerHandle->IsValidHandle())
							{
								return;
							}

							UObject* Layer = nullptr;
							if (!Stack)
							{
								LayerHandle->SetValue(Layer);
								return;
							}

							LayerHandle->GetValue(Layer);

							UpdateLayer(Stack, Layer);
						}),
						MakeLambdaDelegate([this]
						{
							if (StackComboButton)
							{
								StackComboButton->SetIsOpen(false);
							}
						}),
						StackHandle,
						{});
				})
				.OnMenuOpenChanged_Lambda([this](const bool bOpen)
				{
					if (!bOpen)
					{
						if (StackComboButton)
						{
							StackComboButton->SetMenuContent(SNullWidget::NullWidget);
						}
					}
				})
				.IsEnabled(!BaseHandle->IsEditConst())
				.HAlign(HAlign_Fill)
				.ButtonContent()
				[
					SNew(STextBlock)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.Text_Lambda([this]
					{
						if (!StackHandle ||
							!StackHandle->IsValidHandle())
						{
							return INVTEXT("None");
						}

						UObject* Object = nullptr;
						const FPropertyAccess::Result Result = StackHandle->GetValue(Object);
						if (Result == FPropertyAccess::MultipleValues)
						{
							return INVTEXT("Multiple Values");
						}

						if (!Object ||
							Result == FPropertyAccess::Fail)
						{
							return INVTEXT("None");
						}

						return FText::AsCultureInvariant(Object->GetName());
					})
				]
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.f, 0.f)
				[
					PropertyCustomizationHelpers::MakeUseSelectedButton(
						MakeLambdaDelegate([this]
						{
							FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

							const UVoxelLayerStack* NewStack = Cast<UVoxelLayerStack>(GEditor->GetSelectedObjects()->GetTop(UVoxelLayerStack::StaticClass()));
							if (!NewStack)
							{
								return;
							}

							if (!StackHandle ||
								!StackHandle->IsValidHandle())
							{
								return;
							}

							StackHandle->SetValue(NewStack);

							if (!LayerHandle ||
								!LayerHandle->IsValidHandle())
							{
								return;
							}

							UObject* Layer = nullptr;
							LayerHandle->GetValue(Layer);

							UpdateLayer(NewStack, Layer);
						}),
						FText(),
						!BaseHandle->IsEditConst(),
						false)
				]
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.f, 0.f)
				[
				PropertyCustomizationHelpers::MakeBrowseButton(
					MakeLambdaDelegate([this]
					{
						if (!StackHandle ||
							!StackHandle->IsValidHandle())
						{
							return;
						}

						FAssetData AssetData;
						StackHandle->GetValue(AssetData);
						if (!AssetData.IsValid())
						{
							return;
						}

						TArray<FAssetData> AssetDataList;
						AssetDataList.Add(AssetData);
						GEditor->SyncBrowserToObjects(AssetDataList);
					}),
					{},
					!BaseHandle->IsEditConst(),
					false)
				]
			];
	}

	TSharedRef<SWidget> CreateLayerPicker()
	{
		TDelegate<bool(const UVoxelLayerStack*, const FAssetData&)> ShouldFilterAsset = MakeLambdaDelegate([this](const UVoxelLayerStack* Stack, const FAssetData& AssetData)
		{
			if (!Stack)
			{
				return true;
			}

			if constexpr (std::is_same_v<T, FVoxelStackLayer>)
			{
				return
					!Stack->HeightLayers.Contains(AssetData.GetAsset()) &&
					!Stack->VolumeLayers.Contains(AssetData.GetAsset());
			}
			else if constexpr (std::is_same_v<T, FVoxelStackHeightLayer>)
			{
				return !Stack->HeightLayers.Contains(AssetData.GetAsset());
			}
			else if constexpr (std::is_same_v<T, FVoxelStackVolumeLayer>)
			{
				return !Stack->VolumeLayers.Contains(AssetData.GetAsset());
			}
			else
			{
				return false;
			}
		});

		const TAttribute<bool> IsEnabled = MakeAttributeLambda([this]
		{
			if (!BaseHandle ||
				!BaseHandle->IsValidHandle() ||
				!StackHandle ||
				!StackHandle->IsValidHandle())
			{
				return false;
			}

			UObject* Stack = nullptr;
			StackHandle->GetValue(Stack);
			return
				BaseHandle->IsValidHandle() &&
				Stack != nullptr;
		});

		return
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1.f)
			[
				SAssignNew(LayerComboButton, SComboButton)
				.OnGetMenuContent_Lambda([this, ShouldFilterAsset]() -> TSharedRef<SWidget>
				{
					if (!LayerHandle ||
						!LayerHandle->IsValidHandle() ||
						!StackHandle ||
						!StackHandle->IsValidHandle())
					{
						return SNullWidget::NullWidget;
					}

					UObject* StackObject = nullptr;
					StackHandle->GetValue(StackObject);

					UObject* Object = nullptr;
					LayerHandle->GetValue(Object);

					TArray<const UClass*> AllowedClasses;
					if constexpr (std::is_same_v<T, FVoxelStackLayer>)
					{
						AllowedClasses = { UVoxelHeightLayer::StaticClass(), UVoxelVolumeLayer::StaticClass() };
					}
					else if constexpr (std::is_same_v<T, FVoxelStackHeightLayer>)
					{
						AllowedClasses = { UVoxelHeightLayer::StaticClass() };
					}
					else if constexpr (std::is_same_v<T, FVoxelStackVolumeLayer>)
					{
						AllowedClasses = { UVoxelVolumeLayer::StaticClass() };
					}

					return PropertyCustomizationHelpers::MakeAssetPickerWithMenu(
						Object,
						true,
						AllowedClasses,
						{},
						{},
						MakeLambdaDelegate([ShouldFilterAsset, StackObject](const FAssetData& AssetData)
						{
							return ShouldFilterAsset.Execute(Cast<UVoxelLayerStack>(StackObject), AssetData);
						}),
						MakeLambdaDelegate([this](const FAssetData& AssetData)
						{
							if (!LayerHandle ||
								!LayerHandle->IsValidHandle())
							{
								return;
							}

							LayerHandle->SetValue(AssetData);
						}),
						MakeLambdaDelegate([this]
						{
							if (LayerComboButton)
							{
								LayerComboButton->SetIsOpen(false);
							}
						}),
						LayerHandle,
						{});
				})
				.OnMenuOpenChanged_Lambda([this](const bool bOpen)
				{
					if (!bOpen)
					{
						if (LayerComboButton)
						{
							LayerComboButton->SetMenuContent(SNullWidget::NullWidget);
						}
					}
				})
				.IsEnabled(IsEnabled)
				.HAlign(HAlign_Fill)
				.ButtonContent()
				[
					SNew(STextBlock)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.Text_Lambda([this]
					{
						if (!LayerHandle ||
							!LayerHandle->IsValidHandle())
						{
							return INVTEXT("None");
						}

						UObject* Object = nullptr;
						const FPropertyAccess::Result Result = LayerHandle->GetValue(Object);
						if (Result == FPropertyAccess::MultipleValues)
						{
							return INVTEXT("Multiple Values");
						}

						if (!Object ||
							Result == FPropertyAccess::Fail)
						{
							return INVTEXT("None");
						}

						return FText::AsCultureInvariant(Object->GetName());
					})
				]
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.f, 0.f)
				[
					PropertyCustomizationHelpers::MakeUseSelectedButton(
						MakeLambdaDelegate([this, ShouldFilterAsset]
						{
							if (!LayerHandle ||
								!LayerHandle->IsValidHandle() ||
								!StackHandle ||
								!StackHandle->IsValidHandle())
							{
								return;
							}

							UObject* StackObject = nullptr;
							StackHandle->GetValue(StackObject);

							FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

							const UObject* Selection = GEditor->GetSelectedObjects()->GetTop(UVoxelLayer::StaticClass());
							if (Selection &&
								ShouldFilterAsset.Execute(Cast<UVoxelLayerStack>(StackObject), FAssetData(Selection)))
							{
								VOXEL_MESSAGE(Warning, "{0} is not present in current stack", Selection->GetName());
								Selection = nullptr;
							}

							if (!Selection)
							{
								return;
							}

							LayerHandle->SetValue(Selection);
						}),
						FText(),
						IsEnabled,
						false)
				]
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(2.f, 0.f)
				[
					PropertyCustomizationHelpers::MakeBrowseButton(
						MakeLambdaDelegate([this]
						{
							if (!LayerHandle ||
								!LayerHandle->IsValidHandle())
							{
								return;
							}

							FAssetData AssetData;
							LayerHandle->GetValue(AssetData);
							if (!AssetData.IsValid())
							{
								return;
							}

							TArray<FAssetData> AssetDataList;
							AssetDataList.Add(AssetData);
							GEditor->SyncBrowserToObjects(AssetDataList);
						}),
						{},
						IsEnabled,
						false
						)
				]
			];
	}

public:
	virtual void Tick() override
	{
		if (GIsSavingPackage ||
			IsGarbageCollecting() ||
			!BaseHandle ||
			!BaseHandle->IsValidHandle())
		{
			return;
		}

		if (StackThumbnail &&
			StackHandle &&
			StackHandle->IsValidHandle())
		{
			UObject* Object = nullptr;
			StackHandle->GetValue(Object);

			if (Object != StackThumbnail->GetAsset())
			{
				StackThumbnail->SetAsset(Object);
				StackThumbnail->RefreshThumbnail();
			}
		}

		if (LayerThumbnail &&
			LayerHandle &&
			LayerHandle->IsValidHandle())
		{
			UObject* Object = nullptr;
			LayerHandle->GetValue(Object);

			if (Object != LayerThumbnail->GetAsset())
			{
				LayerThumbnail->SetAsset(Object);
				LayerThumbnail->RefreshThumbnail();
			}
		}
	}

private:
	TSharedPtr<IPropertyHandle> BaseHandle;
	TSharedPtr<IPropertyHandle> StackHandle;
	TSharedPtr<IPropertyHandle> LayerHandle;

	TSharedPtr<SBorder> ThumbnailBorder;
	TSharedPtr<SComboButton> StackComboButton;
	TSharedPtr<SComboButton> LayerComboButton;

	TSharedPtr<FAssetThumbnail> StackThumbnail;
	TSharedPtr<FAssetThumbnail> LayerThumbnail;
};

DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelStackLayer, TVoxelStackLayerCustomization<FVoxelStackLayer>);
DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelStackHeightLayer, TVoxelStackLayerCustomization<FVoxelStackHeightLayer>);
DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelStackVolumeLayer, TVoxelStackLayerCustomization<FVoxelStackVolumeLayer>);