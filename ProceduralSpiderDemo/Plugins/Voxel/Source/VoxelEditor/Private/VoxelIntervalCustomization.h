// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class FVoxelIntervalStructCustomizationUtils
{
private:
	enum class EIntervalField
	{
		Min,
		Max
	};

	template<typename NumericType>
	struct TData : public TSharedFromThis<TData<NumericType>>
	{
		TSharedPtr<IPropertyHandle> MinValueHandle;
		TSharedPtr<IPropertyHandle> MaxValueHandle;

		TOptional<NumericType> MinAllowedValue;
		TOptional<NumericType> MaxAllowedValue;
		TOptional<NumericType> MinAllowedSliderValue;
		TOptional<NumericType> MaxAllowedSliderValue;

		bool bIsUsingSlider = false;
		bool bAllowInvertedInterval = false;
		bool bClampToMinMaxLimits = false;

		TData(const IPropertyHandle& Handle)
			: MinValueHandle(Handle.GetChildHandle("Min"))
			, MaxValueHandle(Handle.GetChildHandle("Max"))
		{
			const FString& MetaUIMinString = Handle.GetMetaData(TEXT("UIMin"));
			const FString& MetaUIMaxString = Handle.GetMetaData(TEXT("UIMax"));
			const FString& MetaClampMinString = Handle.GetMetaData(TEXT("ClampMin"));
			const FString& MetaClampMaxString = Handle.GetMetaData(TEXT("ClampMax"));
			const FString& UIMinString = MetaUIMinString.Len() ? MetaUIMinString : MetaClampMinString;
			const FString& UIMaxString = MetaUIMaxString.Len() ? MetaUIMaxString : MetaClampMaxString;

			NumericType ClampMin = std::numeric_limits<NumericType>::lowest();
			NumericType ClampMax = std::numeric_limits<NumericType>::max();
			TTypeFromString<NumericType>::FromString(ClampMin, *MetaClampMinString);
			TTypeFromString<NumericType>::FromString(ClampMax, *MetaClampMaxString);

			NumericType UIMin = std::numeric_limits<NumericType>::lowest();
			NumericType UIMax = std::numeric_limits<NumericType>::max();
			TTypeFromString<NumericType>::FromString(UIMin, *UIMinString);
			TTypeFromString<NumericType>::FromString(UIMax, *UIMaxString);

			const NumericType ActualUIMin = FMath::Max(UIMin, ClampMin);
			const NumericType ActualUIMax = FMath::Min(UIMax, ClampMax);

			MinAllowedValue = MetaClampMinString.Len() ? ClampMin : TOptional<NumericType>();
			MaxAllowedValue = MetaClampMaxString.Len() ? ClampMax : TOptional<NumericType>();
			MinAllowedSliderValue = (UIMinString.Len()) ? ActualUIMin : TOptional<NumericType>();
			MaxAllowedSliderValue = (UIMaxString.Len()) ? ActualUIMax : TOptional<NumericType>();

			bAllowInvertedInterval = Handle.HasMetaData("AllowInvertedInterval");
			bClampToMinMaxLimits = Handle.HasMetaData("ClampToMinMaxLimits");
		}

		static TOptional<NumericType> GetValue(IPropertyHandle& Handle)
		{
			NumericType Value;
			if (Handle.GetValue(Value) == FPropertyAccess::Success)
			{
				return TOptional<NumericType>(Value);
			}

			return TOptional<NumericType>();
		}

		TOptional<NumericType> OnGetValue(const EIntervalField Field) const
		{
			return GetValue(
				Field == EIntervalField::Min
				? *MinValueHandle
				: *MaxValueHandle);
		}

		TOptional<NumericType> OnGetMinValue() const
		{
			if (bClampToMinMaxLimits)
			{
				return GetValue(*MinValueHandle);
			}

			return MinAllowedValue;
		}

		TOptional<NumericType> OnGetMinSliderValue() const
		{
			if (bClampToMinMaxLimits)
			{
				return GetValue(*MinValueHandle);
			}

			return MinAllowedSliderValue;
		}

		TOptional<NumericType> OnGetMaxValue() const
		{
			if (bClampToMinMaxLimits)
			{
				return GetValue(*MaxValueHandle);
			}

			return MaxAllowedValue;
		}

		TOptional<NumericType> OnGetMaxSliderValue() const
		{
			if (bClampToMinMaxLimits)
			{
				return GetValue(*MaxValueHandle);
			}

			return MaxAllowedSliderValue;
		}

		void SetValue(
			NumericType NewValue,
			const EIntervalField Field,
			EPropertyValueSetFlags::Type Flags = EPropertyValueSetFlags::DefaultFlags)
		{
			IPropertyHandle& Handle = Field
				== EIntervalField::Min
				? *MinValueHandle
				: *MaxValueHandle;
			IPropertyHandle& OtherHandle =
				Field == EIntervalField::Min
				? *MaxValueHandle
				: *MinValueHandle;

			const TOptional<NumericType> OtherValue = GetValue(OtherHandle);
			const bool bOutOfRange = OtherValue.IsSet() && ((Field == EIntervalField::Min && NewValue > OtherValue.GetValue()) || (Field == EIntervalField::Max && NewValue < OtherValue.GetValue()));

			// The order of execution of the Intertactive change vs commit is super important for the Undo history
			// So Interactive we must do, Handle, Other Handle
			// For Commit: we must do the reverse to properly close the scope of traction
			if (!bOutOfRange ||
				bAllowInvertedInterval)
			{
				if (Flags == EPropertyValueSetFlags::InteractiveChange)
				{
					ensure(Handle.SetValue(NewValue, Flags) == FPropertyAccess::Success);

					if (OtherValue.IsSet())
					{
						ensure(OtherHandle.SetValue(OtherValue.GetValue(), Flags) == FPropertyAccess::Success);
					}
				}
				else
				{
					if (OtherValue.IsSet())
					{
						ensure(OtherHandle.SetValue(OtherValue.GetValue(), Flags) == FPropertyAccess::Success);
					}

					ensure(Handle.SetValue(NewValue, Flags) == FPropertyAccess::Success);
				}
			}
			else if (!bClampToMinMaxLimits)
			{
				if (Flags == EPropertyValueSetFlags::InteractiveChange)
				{
					ensure(Handle.SetValue(NewValue, Flags) == FPropertyAccess::Success);
					ensure(OtherHandle.SetValue(NewValue, Flags) == FPropertyAccess::Success);
				}
				else
				{
					ensure(OtherHandle.SetValue(NewValue, Flags) == FPropertyAccess::Success);
					ensure(Handle.SetValue(NewValue, Flags) == FPropertyAccess::Success);
				}
			}
		}

		void OnValueCommitted(
			NumericType NewValue,
			ETextCommit::Type,
			const EIntervalField Field)
		{
			SetValue(NewValue, Field);
		}

		void OnValueChanged(
			NumericType NewValue,
			const EIntervalField Field)
		{
			if (bIsUsingSlider)
			{
				SetValue(NewValue, Field, EPropertyValueSetFlags::InteractiveChange);
			}
		}


		void OnBeginSliderMovement()
		{
			bIsUsingSlider = true;
			GEditor->BeginTransaction(INVTEXT("Set Interval Property"));
		}


		void OnEndSliderMovement(NumericType)
		{
			bIsUsingSlider = false;
			GEditor->EndTransaction();
		}

		bool IsPropertyEnabled(const EIntervalField Field) const
		{
			const TSharedPtr<IPropertyHandle> Handle = Field == EIntervalField::Min ? MinValueHandle : MaxValueHandle;
			if (!Handle)
			{
				return false;
			}
			return !Handle->IsEditConst();
		}

	};

public:
	template<typename NumericType>
	static void UpdateIntervalRow(
		const TSharedRef<IPropertyHandle>& Handle,
		IDetailPropertyRow& Row)
	{
		TSharedRef<TData<NumericType>> Data = MakeShared<TData<NumericType>>(*Handle);

		FDetailWidgetRow& WidgetRow = Row.CustomWidget();
		WidgetRow
		.NameContent()
		[
			Handle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(251.0f)
		.MaxDesiredWidth(251.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(FMargin(0.0f, 0.0f, 3.0f, 0.0f))
			.AutoWidth()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(INVTEXT("Min"))
			]
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 3.0f, 0.0f))
			.VAlign(VAlign_Center)
			[
				SNew(SNumericEntryBox<NumericType>)
				.Value_Lambda([Data]
				{
					return Data->OnGetValue(EIntervalField::Min);
				})
				.MinValue(Data->MinAllowedValue)
				.MinSliderValue(Data->MinAllowedSliderValue)
				.MaxValue_Lambda([Data]
				{
					return Data->OnGetMaxValue();
				})
				.MaxSliderValue_Lambda([Data]
				{
					return Data->OnGetMaxSliderValue();
				})
				.OnValueCommitted(Data, &TData<NumericType>::OnValueCommitted, EIntervalField::Min)
				.OnValueChanged(Data, &TData<NumericType>::OnValueChanged, EIntervalField::Min)
				.OnBeginSliderMovement(Data, &TData<NumericType>::OnBeginSliderMovement)
				.OnEndSliderMovement(Data, &TData<NumericType>::OnEndSliderMovement)
				.UndeterminedString(INVTEXT("Multiple Values"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.AllowSpin(true)
				.IsEnabled_Lambda([Data]
				{
					return Data->IsPropertyEnabled(EIntervalField::Min);
				})
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(2.0f, 0.0f)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(INVTEXT("Max"))
			]
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 3.0f, 0.0f))
			.VAlign(VAlign_Center)
			[
				SNew(SNumericEntryBox<NumericType>)
				.Value_Lambda([Data]
				{
					return Data->OnGetValue(EIntervalField::Max);
				})
				.MinValue_Lambda([Data]
				{
					return Data->OnGetMinValue();
				})
				.MinSliderValue_Lambda([Data]
				{
					return Data->OnGetMinSliderValue();
				})
				.MaxValue(Data->MaxAllowedValue)
				.MaxSliderValue(Data->MaxAllowedSliderValue)
				.OnValueCommitted(Data, &TData<NumericType>::OnValueCommitted, EIntervalField::Max)
				.OnValueChanged(Data, &TData<NumericType>::OnValueChanged, EIntervalField::Max)
				.OnBeginSliderMovement(Data, &TData<NumericType>::OnBeginSliderMovement)
				.OnEndSliderMovement(Data, &TData<NumericType>::OnEndSliderMovement)
				.UndeterminedString(INVTEXT("Multiple Values"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.AllowSpin(true)
				.IsEnabled_Lambda([Data]
				{
					return Data->IsPropertyEnabled(EIntervalField::Max);
				})
			]
		];
	}
};