// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelHeightmapAutoLevel.h"
#include "AssetToolsModule.h"
#include "Engine/Texture2D.h"
#include "VoxelEditorSettings.h"
#include "ContentBrowserModule.h"
#include "IStructureDetailsView.h"

class FVoxelHeightmapAutoLevelSettingsCustomization : public IDetailCustomization
{
	//~ Begin IDetailCustomization Interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
	{
#define GET_HANDLE(Name) DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(FVoxelHeightmapAutoLevelSettings, Name), FVoxelHeightmapAutoLevelSettings::StaticStruct())

		const TSharedRef<IPropertyHandle> ChannelSelectorHandle = GET_HANDLE(bShowChannelSelector);
		ChannelSelectorHandle->MarkHiddenByCustomization();

		bool bShowChannelSelector = true;
		ensure(ChannelSelectorHandle->GetValue(bShowChannelSelector) == FPropertyAccess::Success);

		if (!bShowChannelSelector)
		{
			GET_HANDLE(Channel)->MarkHiddenByCustomization();
		}

#undef GET_HANDLE
	}
	//~ End IDetailCustomization Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	CBMenuExtenderDelegates.Add(MakeLambdaDelegate([](const TArray<FAssetData>& Assets)
	{
		TSharedRef<FExtender> Extender = MakeShared<FExtender>();

		for (const FAssetData& Asset : Assets)
		{
			const UClass* Class = Asset.GetClass();
			if (!ensureVoxelSlow(Class))
			{
				continue;
			}

			if (!Class->IsChildOf<UTexture2D>())
			{
				return Extender;
			}
		}

		Extender->AddMenuExtension(
			"GetAssetActions",
			EExtensionHook::After,
			nullptr,
			MakeLambdaDelegate([Assets](FMenuBuilder& MenuBuilder)
			{
				if (!GetDefault<UVoxelEditorSettings>()->bEnableContentBrowserActions)
				{
					return;
				}

				MenuBuilder.AddMenuEntry(
					INVTEXT("Auto-Level Heightmap Channel"),
					INVTEXT("Auto-levels heightmap values from channel and creates new texture"),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCurveEditor.SetViewModeNormalized"),
					FUIAction(FExecuteAction::CreateLambda([=]
					{
						TArray<UTexture2D*> Textures;
						for (const FAssetData& Asset : Assets)
						{
							UTexture2D* Texture = Cast<UTexture2D>(Asset.GetAsset());
							if (!Texture ||
								!Texture->Source.IsValid() ||
								Texture->Source.GetFormat() == TSF_Invalid)
							{
								continue;
							}

							Textures.Add(Texture);
						}

						if (Textures.Num() == 0)
						{
							return;
						}

						bool bShowChannelSelector = true;
						if (Textures.Num() == 1)
						{
							bShowChannelSelector = GTextureSourceFormats[Textures[0]->Source.GetFormat()].NumComponents > 1;
						}

						FVoxelHeightmapAutoLevelSettings Settings;
						if (!SVoxelHeightmapAutoLevelSettingsDialog::OpenSettings(bShowChannelSelector, Settings))
						{
							return;
						}

						TArray<UObject*> ObjectsToSync;
						for (const UTexture2D* Texture : Textures)
						{
							UTexture2D* NewTexture = FVoxelHeightmapAutoLevel::ConstructNewHeightmap(Texture, Settings);
							if (!NewTexture)
							{
								continue;
							}

							ObjectsToSync.Add(NewTexture);
						}

						GEditor->SyncBrowserToObjects(ObjectsToSync);
					})));
			})
		);

		return Extender;
	}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UTexture2D* FVoxelHeightmapAutoLevel::ConstructNewHeightmap(const UTexture2D* Texture, const FVoxelHeightmapAutoLevelSettings& Settings)
{
	const FTextureSource& Source = Texture->Source;

	const FTextureSourceFormatInfo& TextureFormatInfo = GTextureSourceFormats[Source.GetFormat()];
	const ETextureSourceFormat NewTextureFormat = INLINE_LAMBDA
	{
		switch (TextureFormatInfo.BytesPerPixel / TextureFormatInfo.NumComponents)
		{
		case 1: return TSF_G8;
		case 2: return TSF_R16F;
		case 4: return TSF_R32F;
		default: return TSF_Invalid;
		}
	};

	if (!ensure(NewTextureFormat != TSF_Invalid))
	{
		return nullptr;
	}

	TVoxelArray<uint8> Values;
	PrepareHeightmap(Texture, Settings, NewTextureFormat, Values);

	const FString DefaultPath = FPackageName::GetLongPackagePath(Texture->GetPathName());
	const FString DefaultName = FPackageName::GetShortName(Texture->GetName() + Settings.Suffix);

	const FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	UTexture2D* NewTexture = Cast<UTexture2D>(AssetToolsModule.Get().CreateAsset(DefaultName, DefaultPath, UTexture2D::StaticClass(), nullptr));
	if (!NewTexture)
	{
		return nullptr;
	}

	NewTexture->Source.Init2DWithMipChain(Texture->Source.GetSizeX(), Texture->Source.GetSizeY(), NewTextureFormat);

	if (!NewTexture->Source.IsValid())
	{
		return nullptr;
	}

	{
		const FTextureSource::FMipLock MipLock(FTextureSource::ELockState::ReadWrite, &NewTexture->Source, 0);
		if (!MipLock.IsValid() ||
			!ensure(Values.Num() == MipLock.Image.GetImageSizeBytes()))
		{
			return nullptr;
		}

		FMemory::Memcpy(MipLock.Image.RawData, Values.GetData(), MipLock.Image.GetImageSizeBytes());
	}

	NewTexture->SRGB = false;
	NewTexture->CompressionSettings = TC_Grayscale;
	NewTexture->NeverStream = true;

	NewTexture->PostEditChange();
	NewTexture->UpdateResource();

	return NewTexture;
}

void FVoxelHeightmapAutoLevel::PrepareHeightmap(
	const UTexture2D* Texture,
	const FVoxelHeightmapAutoLevelSettings& Settings,
	const ETextureSourceFormat NewTextureFormat,
	TVoxelArray<uint8>& OutValues)
{
	int32 SizeX = 0;
	int32 SizeY = 0;
	TVoxelArray<float> Values;
	FVoxelTextureUtilities::ExtractTextureChannel(*Texture, Settings.Channel, SizeX, SizeY, Values);

	double AverageValue = 0.f;
	if (Settings.bNormalizeValues)
	{
		FFloatInterval Interval;
		if (Settings.DarkCutoffPercentage > 0.f ||
			Settings.BrightCutoffPercentage > 0.f)
		{
			TVoxelArray<float> SortedValues = Values;
			SortedValues.Sort(TLess<float>());

			const int32 StartIndex = FMath::RoundHalfFromZero(SortedValues.Num() * (Settings.DarkCutoffPercentage * 0.01f));
			const int32 Num = SortedValues.Num() - StartIndex - FMath::RoundHalfFromZero(SortedValues.Num() * (Settings.BrightCutoffPercentage * 0.01f));

			Interval = FVoxelUtilities::GetMinMax(MakeVoxelArrayView(SortedValues).Slice(StartIndex, Num));
		}
		else
		{
			Interval = FVoxelUtilities::GetMinMax(Values);
		}

		for (float& Value : Values)
		{
			Value = FMath::Clamp(FMath::GetRangePct(Interval.Min, Interval.Max, Value), 0.f, 1.f);
			AverageValue += Value;
		}
	}
	else
	{
		for (const float& Value : Values)
		{
			AverageValue += Value;
		}
	}

	AverageValue /= SizeX * SizeY;

	if (Settings.bUpdateMidTones)
	{
		for (float& Value : Values)
		{
			Value *= FMath::Clamp(0.5f / AverageValue, 0.f, 1.f);
		}
	}

	if (NewTextureFormat == TSF_G8)
	{
		FVoxelUtilities::SetNum(OutValues, Values.Num());
		for (int32 Index = 0; Index < Values.Num(); Index++)
		{
			OutValues[Index] = Values[Index] * MAX_uint8;
		}
	}
	else if (NewTextureFormat == TSF_R16F)
	{
		FVoxelUtilities::SetNum(OutValues, Values.Num() * 2);
		const TVoxelArrayView<FFloat16> OutValuesView = OutValues.View<FFloat16>();
		for (int32 Index = 0; Index < Values.Num(); Index++)
		{
			OutValuesView[Index] = FFloat16(Values[Index]);
		}
	}
	else
	{
		FVoxelUtilities::SetNum(OutValues, Values.Num() * 4);
		FVoxelUtilities::Memcpy(OutValues.View<float>(), Values);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelHeightmapAutoLevelSettingsDialog::Construct(const FArguments& InArgs)
{
	WeakParentWindow = InArgs._ParentWindow;

	CreateDetailsView(InArgs._ShowChannelSelector);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		[
			SNew(SBox)
			.WidthOverride(450.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsView->GetWidget().ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(2.f)
				[
					SNew(SButton)
					.Text(INVTEXT("Create"))
					.OnClicked(this, &SVoxelHeightmapAutoLevelSettingsDialog::OnCreateClicked)
				]
			]
		]
	];
}

bool SVoxelHeightmapAutoLevelSettingsDialog::OpenSettings(const bool bShowChannelSelector, FVoxelHeightmapAutoLevelSettings& OutSettings)
{
	const TSharedRef<SWindow> EditWindow = SNew(SWindow)
		.Title(INVTEXT("Auto-Level Heightmap"))
		.SizingRule(ESizingRule::Autosized)
		.ClientSize(FVector2D(0.f, 300.f))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	const TSharedRef<SVoxelHeightmapAutoLevelSettingsDialog> SettingsDialog = SNew(SVoxelHeightmapAutoLevelSettingsDialog)
		.ParentWindow(EditWindow)
		.ShowChannelSelector(bShowChannelSelector);

	EditWindow->SetContent(SettingsDialog);

	GEditor->EditorAddModalWindow(EditWindow);

	OutSettings = *SettingsDialog->StructOnScope->Get();
	return SettingsDialog->bConfirm;
}

FReply SVoxelHeightmapAutoLevelSettingsDialog::OnCreateClicked()
{
	ON_SCOPE_EXIT
	{
		if (const TSharedPtr<SWindow> PinnedWindow = WeakParentWindow.Pin())
		{
			PinnedWindow->RequestDestroyWindow();
		}
	};

	bConfirm = true;

	return FReply::Handled();
}

void SVoxelHeightmapAutoLevelSettingsDialog::CreateDetailsView(const bool bShowChannelSelector)
{
	StructOnScope = MakeShared<TStructOnScope<FVoxelHeightmapAutoLevelSettings>>();
	StructOnScope->InitializeAs<FVoxelHeightmapAutoLevelSettings>();
	(*StructOnScope)->bShowChannelSelector = bShowChannelSelector;

	FDetailsViewArgs Args;
	Args.bAllowSearch = false;
	Args.bShowOptions = false;
	Args.bHideSelectionTip = true;
	Args.bShowPropertyMatrixButton = false;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
	const FStructureDetailsViewArgs StructArgs;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	DetailsView = PropertyModule.CreateStructureDetailView(Args, StructArgs, StructOnScope);
	DetailsView->GetDetailsView()->SetDisableCustomDetailLayouts(false);
	DetailsView->GetDetailsView()->SetGenericLayoutDetailsDelegate(MakeLambdaDelegate([this]() -> TSharedRef<IDetailCustomization>
	{
		return MakeShared<FVoxelHeightmapAutoLevelSettingsCustomization>();
	}));
	DetailsView->GetDetailsView()->ForceRefresh();
}