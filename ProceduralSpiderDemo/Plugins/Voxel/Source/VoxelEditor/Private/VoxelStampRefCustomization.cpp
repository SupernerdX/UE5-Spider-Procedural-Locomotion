// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraph.h"
#include "VoxelStampRef.h"
#include "VoxelHeightStampRef.h"
#include "VoxelVolumeStampRef.h"
#include "SVoxelStampTypeSelector.h"
#include "VoxelStampRefNodeBuilder.h"
#include "VoxelStampRefDataProvider.h"
#include "VoxelIntervalCustomization.h"
#include "Graphs/VoxelHeightGraphStamp.h"
#include "Graphs/VoxelVolumeGraphStamp.h"
#include "VoxelStampComponentBase.h"
#include "VoxelInstancedStampComponent.h"

class FVoxelStampRefCustomization : public FVoxelPropertyTypeCustomizationBase
{
public:
	virtual void CustomizeHeader(const TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		if (PropertyHandle->HasMetaData(STATIC_FNAME("ShowOnlyInnerProperties")))
		{
			return;
		}

		HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		];
	}

	virtual void CustomizeChildren(const TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		VOXEL_FUNCTION_COUNTER();

		FVoxelEditorUtilities::TrackHandle(PropertyHandle);

		const FSimpleDelegate RefreshDelegate = FVoxelEditorUtilities::MakeRefreshDelegate(this, CustomizationUtils);

		FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(PropertyHandle, [&](const FVoxelStampRef& StampRef)
		{
			StampRef.OnRefreshDetails_Editor().Add(RefreshDelegate);
		});

		const TSharedRef<FVoxelStampRefDataProvider> StructProvider = MakeShared<FVoxelStampRefDataProvider>(PropertyHandle);
		StructProvider->bIsPropertyIndirection = false;

		const auto GetPropertyInfo = [PropertyHandle]
		{
			VOXEL_SCOPE_COUNTER("FVoxelStampRefCustomization::GetPropertyInfo");

			FVoxelStamp::FPropertyInfo PropertyInfo;

			FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(PropertyHandle, [&](const FVoxelStampRef& StampRef)
			{
				if (!StampRef)
				{
					return;
				}

				FVoxelStamp::FPropertyInfo LocalPropertyInfo;
				StampRef->GetPropertyInfo(LocalPropertyInfo);

				PropertyInfo &= LocalPropertyInfo;
			});

			return PropertyInfo;
		};

		if (!PropertyHandle->HasMetaData("HideStampData") &&
			StructProvider->GetBaseStructure())
		{
			const FVoxelDetailInterface DetailInterface = INLINE_LAMBDA -> FVoxelDetailInterface
			{
				if (PropertyHandle->HasMetaData("SplitStampProperties"))
				{
					IDetailCategoryBuilder& Category = ChildBuilder.GetParentCategory().GetParentLayout().EditCategory(
						"Stamp",
						INVTEXT("Stamp"));
					Category.SetSortOrder(ChildBuilder.GetParentCategory().GetSortOrder() + 1);

					return Category;
				}
				else
				{
					return ChildBuilder;
				}
			};

			const auto AddProperty = [&](const FName Name) -> IDetailPropertyRow*
			{
				IDetailPropertyRow* Row = DetailInterface.AddExternalStructureProperty(StructProvider, Name);
				if (!Row)
				{
					// Will be null when multi-selecting stamps that don't share all properties
					return nullptr;
				}

				const TSharedPtr<IPropertyHandle> RowProperty = Row->GetPropertyHandle();
				if (!ensure(RowProperty))
				{
					return nullptr;
				}

				FixupWarningRow(DetailInterface, StructProvider, PropertyHandle, Row);
				return Row;
			};

			AddProperty(GET_MEMBER_NAME_CHECKED(FVoxelStamp, Behavior));

			if (IDetailPropertyRow* Row = AddProperty("BlendMode"))
			{
				Row->Visibility(MakeAttributeLambda([GetPropertyInfo]
				{
					return GetPropertyInfo().bIsBlendModeVisible ? EVisibility::Visible : EVisibility::Collapsed;
				}));
			}

			if (IDetailPropertyRow* Row = AddProperty(GET_MEMBER_NAME_CHECKED(FVoxelStamp, Smoothness)))
			{
				Row->Visibility(MakeAttributeLambda([GetPropertyInfo]
				{
					return GetPropertyInfo().bIsSmoothnessVisible ? EVisibility::Visible : EVisibility::Collapsed;
				}));
			}

			AddProperty("Layer");
			AddProperty(GET_MEMBER_NAME_CHECKED(FVoxelStamp, Priority));

			if (IDetailPropertyRow* Row = AddProperty(GET_MEMBER_NAME_CHECKED(FVoxelStamp, MetadataOverrides)))
			{
				Row->Visibility(MakeAttributeLambda([GetPropertyInfo]
				{
					return GetPropertyInfo().bIsMetadataOverridesVisible ? EVisibility::Visible : EVisibility::Collapsed;
				}));
			}

			if (!PropertyHandle->HasMetaData("HideTransform"))
			{
				AddProperty(GET_MEMBER_NAME_CHECKED(FVoxelStamp, Transform));
			}

			AddProperty(GET_MEMBER_NAME_CHECKED(FVoxelStamp, StampSeed));

			IDetailCategoryBuilder* ParentCategory;
			if (DetailInterface.IsCategoryBuilder())
			{
				ParentCategory = &DetailInterface.GetCategoryBuilder();
			}
			else
			{
				ParentCategory = &DetailInterface.GetChildrenBuilder().GetParentCategory();
			}

			const auto AddAdvanced = [&](const FName Name)
			{
				IDetailPropertyRow* Row = DetailInterface.AddExternalStructureProperty(StructProvider, Name);
				if (!Row)
				{
					return;
				}

				const TSharedPtr<IPropertyHandle> RowProperty = Row->GetPropertyHandle();
				if (!ensure(RowProperty))
				{
					return;
				}

				Row->Visibility(EVisibility::Collapsed);

				if (Name == GET_MEMBER_NAME_CHECKED(FVoxelStamp, LODRange))
				{
					RowProperty->SetInstanceMetaData("VoxelPropertyChain", FVoxelEditorUtilities::GeneratePropertyChain(PropertyHandle));

					RowProperty->SetOnPropertyValuePreChange(MakeLambdaDelegate([PropertyHandle]
					{
						PropertyHandle->NotifyPreChange();
					}));
					RowProperty->SetOnPropertyValueChangedWithData(MakeLambdaDelegate([PropertyHandle](const FPropertyChangedEvent& PropertyChangedEvent)
					{
						FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(PropertyHandle, [&](FVoxelStampRef& StampRef, UObject* Object)
						{
							StampRef->FixupProperties();
						});

						PropertyHandle->NotifyPostChange(PropertyChangedEvent.ChangeType);
					}));

					RowProperty->SetOnChildPropertyValuePreChange(MakeLambdaDelegate([PropertyHandle]
					{
						PropertyHandle->NotifyPreChange();
					}));
					RowProperty->SetOnChildPropertyValueChangedWithData(MakeLambdaDelegate([PropertyHandle](const FPropertyChangedEvent& PropertyChangedEvent)
					{
						FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(PropertyHandle, [&](FVoxelStampRef& StampRef, UObject* Object)
						{
							StampRef->FixupProperties();
						});

						PropertyHandle->NotifyPostChange(PropertyChangedEvent.ChangeType);
					}));

					FVoxelIntervalStructCustomizationUtils::UpdateIntervalRow<int32>(
						RowProperty.ToSharedRef(), 
						ParentCategory->AddProperty(RowProperty.ToSharedRef(), EPropertyLocation::Advanced));
					return;
				}

				FixupWarningRow(DetailInterface, StructProvider, PropertyHandle, &ParentCategory->AddProperty(RowProperty.ToSharedRef(), EPropertyLocation::Advanced));
			};

			AddAdvanced("AdditionalLayers");
			AddAdvanced(GET_MEMBER_NAME_CHECKED(FVoxelStamp, LODRange));
			AddAdvanced(GET_MEMBER_NAME_CHECKED(FVoxelStamp, bDisableStampSelection));
			AddAdvanced(GET_MEMBER_NAME_CHECKED(FVoxelVolumeStamp, BoundsExtensionMultiplier));
			AddAdvanced(GET_MEMBER_NAME_CHECKED(FVoxelVolumeStamp, MaximumBoundsExtension));
			AddAdvanced(GET_MEMBER_NAME_CHECKED(FVoxelHeightStamp, HeightPaddingMultiplier));
			AddAdvanced(GET_MEMBER_NAME_CHECKED(FVoxelStamp, bExcludeFromPriorityIncrements));
		}

		const UScriptStruct* Struct = FVoxelStampRef::StaticStruct();
		if (const FStructProperty* Property = CastField<FStructProperty>(PropertyHandle->GetProperty()))
		{
			Struct = Property->Struct;
		}

		const TVoxelSet<const UScriptStruct*> VisibleTypeSelectorStructs =
		{
			FVoxelStampRef::StaticStruct(),
			FVoxelHeightStampRef::StaticStruct(),
			FVoxelVolumeStampRef::StaticStruct(),
			FVoxelHeightInstancedStampRef::StaticStruct(),
			FVoxelVolumeInstancedStampRef::StaticStruct(),
		};

		if (Struct &&
			VisibleTypeSelectorStructs.Contains(Struct))
		{
			ChildBuilder.AddCustomRow(INVTEXT("Type"))
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SVoxelStampTypeSelector)
				.Handle(PropertyHandle)
				.PropertyUtilities(CustomizationUtils.GetPropertyUtilities())
			];
		}

		const TSharedRef<FVoxelStampRefNodeBuilder> Builder = MakeShared<FVoxelStampRefNodeBuilder>(PropertyHandle);
		Builder->Initialize();
		ChildBuilder.AddCustomBuilder(Builder);
	}

private:
	static void FixupWarningRow(
		const FVoxelDetailInterface& DetailInterface,
		const TSharedRef<FVoxelStampRefDataProvider>& StructProvider,
		const TSharedPtr<IPropertyHandle>& PropertyHandle,
		IDetailPropertyRow* Row)
	{
		Row->GetPropertyHandle()->SetInstanceMetaData("VoxelPropertyChain", FVoxelEditorUtilities::GeneratePropertyChain(PropertyHandle));

		const auto FixPrePostChange = [&](const TSharedPtr<IPropertyHandle>& Handle)
		{
			Handle->SetOnPropertyValuePreChange(MakeLambdaDelegate([PropertyHandle]
			{
				PropertyHandle->NotifyPreChange();
			}));
			Handle->SetOnPropertyValueChangedWithData(MakeLambdaDelegate([PropertyHandle](const FPropertyChangedEvent& PropertyChangedEvent)
			{
				FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(PropertyHandle, [&](FVoxelStampRef& StampRef, UObject* Object)
				{
					StampRef->FixupProperties();
				});

				PropertyHandle->NotifyPostChange(PropertyChangedEvent.ChangeType);
			}));

			Handle->SetOnChildPropertyValuePreChange(MakeLambdaDelegate([PropertyHandle]
			{
				PropertyHandle->NotifyPreChange();
			}));
			Handle->SetOnChildPropertyValueChangedWithData(MakeLambdaDelegate([PropertyHandle](const FPropertyChangedEvent& PropertyChangedEvent)
			{
				FVoxelEditorUtilities::ForeachData<FVoxelStampRef>(PropertyHandle, [&](FVoxelStampRef& StampRef, UObject* Object)
				{
					StampRef->FixupProperties();
				});

				PropertyHandle->NotifyPostChange(PropertyChangedEvent.ChangeType);
			}));
		};

		const TSharedPtr<IPropertyHandle> RowProperty = Row->GetPropertyHandle();
		FixPrePostChange(RowProperty);

		const FProperty* Property = RowProperty->GetProperty();
		if (!Property)
		{
			return;
		}

		float Width = 125.f;

		FString DisableMessage;
		TSharedPtr<IPropertyHandle> DisableEditingHandle;
		IDetailPropertyRow* AdditionalRow = nullptr;
		if (Property == &FindFPropertyChecked(FVoxelHeightStamp, Layer) ||
			Property == &FindFPropertyChecked(FVoxelHeightStamp, AdditionalLayers))
		{
			if (Property == &FindFPropertyChecked(FVoxelHeightStamp, Layer))
			{
				Width = 250.f;
			}
			else
			{
				Width = 170.f;
			}

			if (IDetailPropertyRow* DisableEditingRow = DetailInterface.AddExternalStructureProperty(StructProvider, GET_MEMBER_NAME_CHECKED(FVoxelHeightStamp, bDisableEditingLayers)))
			{
				DisableEditingRow->Visibility(EVisibility::Collapsed);
				DisableEditingHandle = DisableEditingRow->GetPropertyHandle();
				DisableMessage = "Layers are overriden by graph.";
			}
		}
		else if (Property == &FindFPropertyChecked(FVoxelHeightGraphStamp, BlendMode))
		{
			if (IDetailPropertyRow* DisableEditingRow = DetailInterface.AddExternalStructureProperty(StructProvider, GET_MEMBER_NAME_CHECKED(FVoxelHeightStamp, bDisableEditingBlendMode)))
			{
				AdditionalRow = DetailInterface.AddExternalStructureProperty(StructProvider, GET_MEMBER_NAME_CHECKED(FVoxelStamp, bApplyOnVoid));
				AdditionalRow->Visibility(EVisibility::Collapsed);
				DisableEditingRow->Visibility(EVisibility::Collapsed);
				DisableEditingHandle = DisableEditingRow->GetPropertyHandle();
				DisableMessage = "Blend Mode is overriden by graph.";
			}
		}
		else if (
			Property == &FindFPropertyChecked(FVoxelVolumeStamp, Layer) ||
			Property == &FindFPropertyChecked(FVoxelVolumeStamp, AdditionalLayers))
		{
			if (Property == &FindFPropertyChecked(FVoxelVolumeStamp, Layer))
			{
				Width = 250.f;
			}
			else
			{
				Width = 170.f;
			}

			if (IDetailPropertyRow* DisableEditingRow = DetailInterface.AddExternalStructureProperty(StructProvider, GET_MEMBER_NAME_CHECKED(FVoxelVolumeStamp, bDisableEditingLayers)))
			{
				DisableEditingRow->Visibility(EVisibility::Collapsed);
				DisableEditingHandle = DisableEditingRow->GetPropertyHandle();
				DisableMessage = "Layers are overriden by graph.";
			}
		}
		else if (Property == &FindFPropertyChecked(FVoxelVolumeGraphStamp, BlendMode))
		{
			if (IDetailPropertyRow* DisableEditingRow = DetailInterface.AddExternalStructureProperty(StructProvider, GET_MEMBER_NAME_CHECKED(FVoxelVolumeStamp, bDisableEditingBlendMode)))
			{
				AdditionalRow = DetailInterface.AddExternalStructureProperty(StructProvider, GET_MEMBER_NAME_CHECKED(FVoxelStamp, bApplyOnVoid));
				AdditionalRow->Visibility(EVisibility::Collapsed);
				DisableEditingRow->Visibility(EVisibility::Collapsed);
				DisableEditingHandle = DisableEditingRow->GetPropertyHandle();
				DisableMessage = "Blend Mode is overriden by graph.";
			}
		}

		if (!DisableEditingHandle ||
			!DisableEditingHandle->IsValidHandle())
		{
			return;
		}

		FDetailWidgetRow& CustomWidget = Row->CustomWidget(true);

		TSharedPtr<SWidget> ValueWidget;
		if (AdditionalRow)
		{
			if (const TSharedPtr<IPropertyHandle> Handle = AdditionalRow->GetPropertyHandle())
			{
				FixPrePostChange(Handle);
			}

			TSharedPtr<SWidget> DummyNameWidget;
			TSharedPtr<SWidget> AdditionalRowValueWidget;
			AdditionalRow->GetDefaultWidgets(DummyNameWidget, AdditionalRowValueWidget, true);

			AdditionalRowValueWidget->SetToolTipText(AdditionalRow->GetPropertyHandle()->GetToolTipText());

			ValueWidget =
				SNew(SHorizontalBox)
				.ToolTipText(AdditionalRow->GetPropertyHandle()->GetToolTipText())
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					RowProperty->CreatePropertyValueWidgetWithCustomization(nullptr)
				]
				+ SHorizontalBox::Slot()
				.MaxWidth(15.f)
				[
					SNew(SSpacer)
					.Size(15.f)
				]
				+ SHorizontalBox::Slot()
				.FillContentWidth(1.f)
				.Padding(2.f, 0.f, 0.f, 0.f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
					.Text(FText::Format(INVTEXT("{0}: "), AdditionalRow->GetPropertyHandle()->GetPropertyDisplayName()))
				]
				+ SHorizontalBox::Slot()
				.Padding(2.f, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					AdditionalRowValueWidget.ToSharedRef()
				];

			CustomWidget.FilterString(AdditionalRow->GetPropertyHandle()->GetPropertyDisplayName());
		}
		else
		{
			ValueWidget = RowProperty->CreatePropertyValueWidgetWithCustomization(nullptr);
		}

		CustomWidget.NameContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				RowProperty->CreatePropertyNameWidget()
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(4.f, 0.f, 0.f, 0.f)
			.AutoWidth()
			[
				SNew(SBox)
				.Visibility_Lambda([DisableEditingHandle]
				{
					if (!DisableEditingHandle->IsValidHandle())
					{
						return EVisibility::Collapsed;
					}

					bool bValue = false;
					DisableEditingHandle->GetValue(bValue);
					return bValue ? EVisibility::Visible : EVisibility::Collapsed;
				})
				.ToolTipText(FText::FromString(DisableMessage))
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Error"))
				]
			]
		];

		CustomWidget.ValueContent()
		.MinDesiredWidth(Width)
		.MaxDesiredWidth(Width)
		[
			ValueWidget.ToSharedRef()
		];
	}
};

DEFINE_VOXEL_STRUCT_LAYOUT_RECURSIVE(FVoxelStampRef, FVoxelStampRefCustomization);