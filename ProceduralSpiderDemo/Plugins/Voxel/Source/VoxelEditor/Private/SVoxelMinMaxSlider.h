// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Widgets/Input/SSlider.h"

DECLARE_DELEGATE_OneParam(FOnLowerValueChanged, float)
DECLARE_DELEGATE_OneParam(FOnUpperValueChanged, float)
DECLARE_DELEGATE_RetVal_OneParam(FText, FFormatValue, float)

class SVoxelMinMaxSlider : public SSlider
{
public:
	VOXEL_SLATE_ARGS()
	{
		FArguments()
			: _Orientation(Orient_Horizontal)
			, _LowerHandleValue(0.0f)
			, _UpperHandleValue(1.0f)
			, _IndentHandle(true)
			, _SliderBarInnerColor(FSlateColor::UseSubduedForeground())
			, _SliderBarOuterColor(FSlateColor::UseSubduedForeground())
			, _SliderLowerHandleColor(FLinearColor::White)
			, _SliderUpperHandleColor(FLinearColor::White)
			, _MinValue(0.0f)
			, _MaxValue(1.0f)
			, _ShowMinMaxValues(true)
			, _MinValueFont(FCoreStyle::Get().GetFontStyle("ToolTip.LargerFont"))
			, _MaxValueFont(FCoreStyle::Get().GetFontStyle("ToolTip.LargerFont"))
			, _MinValueColorAndOpacity(FSlateColor::UseSubduedForeground())
			, _MaxValueColorAndOpacity(FSlateColor::UseSubduedForeground())
			, _ShowLowerUpperValues(true)
			, _LowerValueFont(FCoreStyle::Get().GetFontStyle("ToolTip.LargerFont"))
			, _UpperValueFont(FCoreStyle::Get().GetFontStyle("ToolTip.LargerFont"))
			, _LowerValueColorAndOpacity(FSlateColor::UseSubduedForeground())
			, _UpperValueColorAndOpacity(FSlateColor::UseSubduedForeground())
		{}
		SLATE_ARGUMENT( EOrientation, Orientation)
		SLATE_ATTRIBUTE(float, LowerHandleValue)
		SLATE_ATTRIBUTE(float, UpperHandleValue)
		SLATE_ATTRIBUTE(bool, IndentHandle)
		SLATE_ATTRIBUTE(FSlateColor, SliderBarInnerColor)
		SLATE_ATTRIBUTE(FSlateColor, SliderBarOuterColor)
		SLATE_ATTRIBUTE(FSlateColor, SliderLowerHandleColor)
		SLATE_ATTRIBUTE(FSlateColor, SliderUpperHandleColor)
		SLATE_EVENT(FOnLowerValueChanged, OnLowerHandleValueChanged)
		SLATE_EVENT(FOnUpperValueChanged, OnUpperHandleValueChanged)
		SLATE_ARGUMENT(float, MinValue)
		SLATE_ARGUMENT(float, MaxValue)

		SLATE_ATTRIBUTE(bool, ShowMinMaxValues)
		SLATE_ATTRIBUTE(FSlateFontInfo, MinValueFont)
		SLATE_ATTRIBUTE(FSlateFontInfo, MaxValueFont)
		SLATE_ATTRIBUTE(FSlateColor, MinValueColorAndOpacity)
		SLATE_ATTRIBUTE(FSlateColor, MaxValueColorAndOpacity)
		SLATE_EVENT(FFormatValue, OnMinValueFormat)
		SLATE_EVENT(FFormatValue, OnMaxValueFormat)
		SLATE_ATTRIBUTE(bool, ShowLowerUpperValues)
		SLATE_ATTRIBUTE(FSlateFontInfo, LowerValueFont)
		SLATE_ATTRIBUTE(FSlateFontInfo, UpperValueFont)
		SLATE_ATTRIBUTE(FSlateColor, LowerValueColorAndOpacity)
		SLATE_ATTRIBUTE(FSlateColor, UpperValueColorAndOpacity)
		SLATE_EVENT(FFormatValue, OnLowerValueFormat)
		SLATE_EVENT(FFormatValue, OnUpperValueFormat)
	};

	void Construct(const FArguments& Args);

    float GetLowerNormalizedValue() const;
	float GetUpperNormalizedValue() const;
	void SetLowerValue(const TAttribute<float>& InValueAttribute);
	void SetUpperValue(const TAttribute<float>& InValueAttribute);
	void OnSliderValueChanged(float NewValue) const;

protected:
	enum class ESliderHandle
	{
		None,
		LowerHandle,
		UpperHandle
	};

	FVector2D CalculateHandlePosition(const FGeometry& HandleGeometry, float Value) const;
	ESliderHandle DetermineClickedHandle(const FGeometry& HandleGeometry, const FVector2D& LocalMousePosition) const;
	float PositionToHandleValue(const FGeometry& MyGeometry, const UE::Slate::FDeprecateVector2DParameter& AbsolutePosition) const;

	//~ Begin SSlider Interface
	virtual void CommitValue(float NewValue) override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove( const FGeometry& MyGeometry, const FPointerEvent& MouseEvent ) override;
	virtual FReply OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) override;
	virtual FReply OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) override;
	//~ End SSlider Interface

	void ResetState();

private:
	TAttribute<float> LowerHandleValue;
	TAttribute<float> UpperHandleValue;
	FOnLowerValueChanged OnLowerHandleValueChanged;
	FOnUpperValueChanged OnUpperHandleValueChanged;
	TAttribute<bool> IndentHandleAttribute;
	TAttribute<FSlateColor> SliderBarInnerColorAttribute;
	TAttribute<FSlateColor> SliderBarOuterColorAttribute;
	TAttribute<FSlateColor> SliderLowerHandleColorAttribute;
	TAttribute<FSlateColor> SliderUpperHandleColorAttribute;

	TAttribute<bool> ShowMinMaxValues;
	TAttribute<FSlateFontInfo> MinValueFont;
	TAttribute<FSlateFontInfo> MaxValueFont;
	TAttribute<FSlateColor> MinValueColorAndOpacity;
	TAttribute<FSlateColor> MaxValueColorAndOpacity;
	FFormatValue OnMinValueFormat;
	FFormatValue OnMaxValueFormat;
	TAttribute<bool> ShowLowerUpperValues;
	TAttribute<FSlateFontInfo> LowerValueFont;
	TAttribute<FSlateFontInfo> UpperValueFont;
	TAttribute<FSlateColor> LowerValueColorAndOpacity;
	TAttribute<FSlateColor> UpperValueColorAndOpacity;
	FFormatValue OnLowerValueFormat;
	FFormatValue OnUpperValueFormat;

	ESliderHandle CurrentHandle = ESliderHandle::None;
};