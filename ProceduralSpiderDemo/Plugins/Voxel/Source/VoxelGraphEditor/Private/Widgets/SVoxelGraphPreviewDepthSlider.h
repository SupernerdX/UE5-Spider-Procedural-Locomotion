// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

class SVoxelGraphPreviewDepthSlider : public SCompoundWidget
{
public:
	VOXEL_SLATE_ARGS()
	{
		FArguments()
			: _MinValue(-100.f)
			, _MaxValue(100.f)
			, _Value(0.f)
		{
		}

		SLATE_ARGUMENT(float, MinValue)
		SLATE_ARGUMENT(float, MaxValue)
		SLATE_ARGUMENT(float, Value)

		SLATE_ARGUMENT(FText, ValueText)

		SLATE_EVENT(FOnFloatValueChanged, OnValueChanged)
	};

	void Construct(const FArguments& InArgs);

	//~ Begin SCompoundWidget Interface
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;
	//~ End SCompoundWidget Interface

	void ResetValue(float NewValue, bool bKeepRange);

private:
	FString GetValueText() const;
	void CreateMinMaxValue(bool bMin);

	TOptional<float> GetMinMaxValue(bool bMin) const;

	void UpdateRange(float NewMinMaxValue, ETextCommit::Type, bool bMin);

private:
	FText ValueText;
	float Value = 0.f;
	float MinValue = 0.f;
	float MaxValue = 0.f;

	FOnFloatValueChanged OnValueChanged;

	TSharedPtr<SNumericEntryBox<float>> MinValueBox;
	TSharedPtr<SNumericEntryBox<float>> MaxValueBox;
	TSharedPtr<SSlider> Slider;

	TSharedPtr<TNumericUnitTypeInterface<float>> MinValueTypeInterface;
	TSharedPtr<TNumericUnitTypeInterface<float>> MaxValueTypeInterface;
	FNumberFormattingOptions Options;
};