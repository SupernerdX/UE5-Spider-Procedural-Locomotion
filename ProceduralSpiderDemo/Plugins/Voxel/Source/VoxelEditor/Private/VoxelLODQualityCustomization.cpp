// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelLODQuality.h"
#include "SVoxelMinMaxSlider.h"

class FVoxelLODQualityCustomization : public FVoxelPropertyTypeCustomizationBase
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		TSharedPtr<SWidget> ValueWidget;

		const TArray<TWeakObjectPtr<UObject>> SelectedObjects = CustomizationUtils.GetPropertyUtilities()->GetSelectedObjects();
		if (SelectedObjects.Num() != 1)
		{
			ValueWidget =
				SNew(SVoxelDetailText)
				.Text(INVTEXT("Multiple values"));
		}
		else
		{
			const TSharedPtr<IPropertyHandle> AlwaysUseGameWorldHandle = PropertyHandle->GetChildHandleStatic(FVoxelLODQuality, bAlwaysUseGameQuality);

			ValueWidget =
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SVoxelDetailText)
					.Text_Lambda([SelectedObjects, AlwaysUseGameWorldHandle]
					{
						if (!AlwaysUseGameWorldHandle->IsValidHandle() ||
							!ensure(SelectedObjects.Num() == 1))
						{
							return INVTEXT("Invalid");
						}

						const UObject* Object = SelectedObjects[0].Get();
						if (!ensureVoxelSlow(Object) ||
							!ensureVoxelSlow(Object->GetWorld()))
						{
							return INVTEXT("Invalid");
						}

						bool bAlwaysUseGameWorld = false;
						ensure(AlwaysUseGameWorldHandle->GetValue(bAlwaysUseGameWorld) == FPropertyAccess::Success);
						return Object->GetWorld()->IsGameWorld() || bAlwaysUseGameWorld ? INVTEXT("Game: ") : INVTEXT("Editor: ");
					})
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(SWidgetSwitcher)
					.WidgetIndex_Lambda([SelectedObjects, AlwaysUseGameWorldHandle]
					{
						if (!AlwaysUseGameWorldHandle->IsValidHandle() ||
							!ensure(SelectedObjects.Num() == 1))
						{
							return 2;
						}

						const UObject* Object = SelectedObjects[0].Get();
						if (!Object ||
							!Object->GetWorld())
						{
							return 2;
						}

						bool bAlwaysUseGameWorld = false;
						if (!AlwaysUseGameWorldHandle->GetValue(bAlwaysUseGameWorld))
						{
							return 2;
						}

						if (Object->GetWorld()->IsGameWorld() ||
							bAlwaysUseGameWorld)
						{
							return 0;
						}

						return 1;
					})
					+ SWidgetSwitcher::Slot()
					[
						CreateQualityWidget(PropertyHandle->GetChildHandleStatic(FVoxelLODQuality, GameQuality))
					]
					+ SWidgetSwitcher::Slot()
					[
						CreateQualityWidget(PropertyHandle->GetChildHandleStatic(FVoxelLODQuality, EditorQuality))
					]
					+ SWidgetSwitcher::Slot()
					[
						SNullWidget::NullWidget
					]
				];
		}
		HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			ValueWidget.ToSharedRef()
		];
	}

	virtual void CustomizeChildren(const TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		ChildBuilder.AddProperty(PropertyHandle->GetChildHandleStatic(FVoxelLODQuality, GameQuality));
		ChildBuilder.AddProperty(PropertyHandle->GetChildHandleStatic(FVoxelLODQuality, EditorQuality));
		ChildBuilder.AddProperty(PropertyHandle->GetChildHandleStatic(FVoxelLODQuality, bAlwaysUseGameQuality));
	}

private:
	TSharedRef<SWidget> CreateQualityWidget(const TSharedPtr<IPropertyHandle>& QualityHandle)
	{
		return
			SNew(SVoxelMinMaxSlider)
			.MinValue(0.f)
			.MaxValue(10.f)
			.SliderBarInnerColor(FStyleColors::Primary)
			.LowerValueColorAndOpacity(FSlateColor::UseForeground())
			.UpperValueColorAndOpacity(FSlateColor::UseForeground())
			.OnLowerValueFormat_Lambda([](const float Value)
			{
				const int32 Power = FMath::Max(FMath::CeilToInt32(FMath::LogX(10, Value)), 0);
				const int32 NumDigits = FMath::Max(2 - Power, 0);

				FNumberFormattingOptions Options;
				Options.MinimumFractionalDigits = NumDigits;
				Options.MaximumFractionalDigits = NumDigits;

				return FText::AsNumber(Value, &Options);
			})
			.OnUpperValueFormat_Lambda([](const float Value)
			{
				const int32 Power = FMath::Max(FMath::CeilToInt32(FMath::LogX(10, Value)), 0);
				const int32 NumDigits = FMath::Max(2 - Power, 0);

				FNumberFormattingOptions Options;
				Options.MinimumFractionalDigits = NumDigits;
				Options.MaximumFractionalDigits = NumDigits;

				return FText::AsNumber(Value, &Options);
			})
			.LowerHandleValue_Lambda(MakeWeakPtrLambda(this, [QualityHandle]
			{
				if (!QualityHandle->IsValidHandle())
				{
					return 0.f;
				}

				const FFloatInterval& Interval = FVoxelEditorUtilities::GetStructPropertyValue<FFloatInterval>(QualityHandle);
				return Interval.Min;
			}))
			.UpperHandleValue_Lambda(MakeWeakPtrLambda(this, [QualityHandle]
			{
				if (!QualityHandle->IsValidHandle())
				{
					return 0.f;
				}

				const FFloatInterval& Interval = FVoxelEditorUtilities::GetStructPropertyValue<FFloatInterval>(QualityHandle);
				return Interval.Max;
			}))
			.OnLowerHandleValueChanged_Lambda(MakeWeakPtrLambda(this, [QualityHandle](const float NewValue)
			{
				if (!QualityHandle->IsValidHandle())
				{
					return;
				}

				FFloatInterval NewInterval = FVoxelEditorUtilities::GetStructPropertyValue<FFloatInterval>(QualityHandle);
				NewInterval.Min = NewValue;

				FVoxelEditorUtilities::SetStructPropertyValue(QualityHandle, NewInterval);
			}))
			.OnUpperHandleValueChanged_Lambda(MakeWeakPtrLambda(this, [QualityHandle](const float NewValue)
			{
				if (!QualityHandle->IsValidHandle())
				{
					return;
				}

				FFloatInterval NewInterval = FVoxelEditorUtilities::GetStructPropertyValue<FFloatInterval>(QualityHandle);
				NewInterval.Max = NewValue;

				FVoxelEditorUtilities::SetStructPropertyValue(QualityHandle, NewInterval);
			}));
	}
};

DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelLODQuality, FVoxelLODQualityCustomization);