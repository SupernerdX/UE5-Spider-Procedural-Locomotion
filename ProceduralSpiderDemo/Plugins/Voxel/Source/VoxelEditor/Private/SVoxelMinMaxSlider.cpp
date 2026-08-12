// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelMinMaxSlider.h"
#include "Fonts/FontMeasure.h"

void SVoxelMinMaxSlider::Construct(const FArguments& Args)
{
	SSlider::Construct(SSlider::FArguments()
		.MinValue(Args._MinValue)
		.MaxValue(Args._MaxValue)
		.Orientation(Args._Orientation)
	);

	LowerHandleValue = Args._LowerHandleValue;
	UpperHandleValue = Args._UpperHandleValue;
	OnLowerHandleValueChanged = Args._OnLowerHandleValueChanged;
	OnUpperHandleValueChanged = Args._OnUpperHandleValueChanged;
	IndentHandleAttribute = Args._IndentHandle;
	SliderBarInnerColorAttribute = Args._SliderBarInnerColor;
	SliderBarOuterColorAttribute = Args._SliderBarOuterColor;
	SliderLowerHandleColorAttribute = Args._SliderLowerHandleColor;
	SliderUpperHandleColorAttribute = Args._SliderUpperHandleColor;

	ShowMinMaxValues = Args._ShowMinMaxValues;
	MinValueFont = Args._MinValueFont;
	MaxValueFont = Args._MaxValueFont;
	MinValueColorAndOpacity = Args._MinValueColorAndOpacity;
	MaxValueColorAndOpacity = Args._MaxValueColorAndOpacity;
	OnMinValueFormat = Args._OnMinValueFormat;
	OnMaxValueFormat = Args._OnMaxValueFormat;

	ShowLowerUpperValues = Args._ShowLowerUpperValues;
	LowerValueFont = Args._LowerValueFont;
	UpperValueFont = Args._UpperValueFont;
	LowerValueColorAndOpacity = Args._LowerValueColorAndOpacity;
	UpperValueColorAndOpacity = Args._UpperValueColorAndOpacity;
	OnLowerValueFormat = Args._OnLowerValueFormat;
	OnUpperValueFormat = Args._OnUpperValueFormat;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float SVoxelMinMaxSlider::GetLowerNormalizedValue() const
{
	if (MaxValue == MinValue)
	{
		return 1.0f;
	}

	return (LowerHandleValue.Get() - MinValue) / (MaxValue - MinValue);
}

float SVoxelMinMaxSlider::GetUpperNormalizedValue() const
{
	if (MaxValue == MinValue)
	{
		return 1.0f;
	}

	return (UpperHandleValue.Get() - MinValue) / (MaxValue - MinValue);
}

void SVoxelMinMaxSlider::SetLowerValue(const TAttribute<float>& InValueAttribute)
{
	const float NewLowerValue = InValueAttribute.Get() <= UpperHandleValue.Get() ? InValueAttribute.Get() : UpperHandleValue.Get();
	LowerHandleValue.Set(NewLowerValue);
}

void SVoxelMinMaxSlider::SetUpperValue(const TAttribute<float>& InValueAttribute)
{
	const float NewUpperValue = InValueAttribute.Get() >= LowerHandleValue.Get() ? InValueAttribute.Get() : LowerHandleValue.Get();
	UpperHandleValue.Set(NewUpperValue);
}

void SVoxelMinMaxSlider::OnSliderValueChanged(const float NewValue) const
{
	if (CurrentHandle == ESliderHandle::LowerHandle)
	{
		OnLowerHandleValueChanged.ExecuteIfBound(NewValue);
	}
	if (CurrentHandle == ESliderHandle::UpperHandle)
	{
		OnUpperHandleValueChanged.ExecuteIfBound(NewValue);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


FVector2D SVoxelMinMaxSlider::CalculateHandlePosition(const FGeometry& HandleGeometry, const float Value) const
{
	float ClampedValue = FMath::Clamp(Value, 0.0f, 1.0f);

	FVector2D Position;
	if (Orientation == Orient_Horizontal)
	{
		Position.X = ClampedValue * HandleGeometry.GetLocalSize().X;
		Position.Y = HandleGeometry.GetLocalSize().Y * 0.5f;
	}
	else
	{
		ClampedValue = 1.f - ClampedValue;
		Position.X = HandleGeometry.GetLocalSize().X * 0.5f;
		Position.Y = ClampedValue * HandleGeometry.GetLocalSize().Y;
	}

	return Position;
}

SVoxelMinMaxSlider::ESliderHandle SVoxelMinMaxSlider::DetermineClickedHandle(const FGeometry& HandleGeometry, const FVector2D& LocalMousePosition) const
{
	const FVector2D LowerHandlePos = CalculateHandlePosition(HandleGeometry, GetLowerNormalizedValue());
	const FVector2D UpperHandlePos = CalculateHandlePosition(HandleGeometry, GetUpperNormalizedValue());

	const float LowerHandleDistance = FVector2D::DistSquared(LowerHandlePos, LocalMousePosition);
	const float UpperHandleDistance = FVector2D::DistSquared(UpperHandlePos, LocalMousePosition);

	return
		LowerHandleDistance < UpperHandleDistance
		? ESliderHandle::LowerHandle
		: ESliderHandle::UpperHandle;
}

float SVoxelMinMaxSlider::PositionToHandleValue(const FGeometry& MyGeometry, const UE::Slate::FDeprecateVector2DParameter& AbsolutePosition) const
{
	const float HandleValue =
		CurrentHandle == ESliderHandle::UpperHandle
		? UpperHandleValue.Get()
		: LowerHandleValue.Get();

	
	const FVector2f LocalPosition = MyGeometry.AbsoluteToLocal(AbsolutePosition);

	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	const FText MinValueText = OnMinValueFormat.IsBound() ? OnMinValueFormat.Execute(MinValue) : FText::AsNumber(MinValue);
	const FVector2D MinValueLabelSize = FontMeasure->Measure(MinValueText, MinValueFont.Get());

	const FText MaxValueText = OnMaxValueFormat.IsBound() ? OnMaxValueFormat.Execute(MaxValue) : FText::AsNumber(MaxValue);
	const FVector2D MaxValueLabelSize = FontMeasure->Measure(MaxValueText, MaxValueFont.Get());

	float RelativeValue;
	float Denominator;

	// Only need X as we rotate the thumb image when rendering vertically
	const float HandleSize = GetThumbImage()->ImageSize.X;
	const float Indentation = HandleSize * (IndentHandleAttribute.Get() ? 1.f : 0.f);

	if (Orientation == Orient_Horizontal)
	{
		Denominator = MyGeometry.Size.X - Indentation - HandleSize;
		float Position = LocalPosition.X;
		Position -= HandleSize * 0.5f + Indentation * 0.5f;

		if (ShowMinMaxValues.Get())
		{
			Denominator -= MaxValueLabelSize.X + 6.f + MinValueLabelSize.X + 6.f;
			Position -= MinValueLabelSize.X + 6.f;
		}

		RelativeValue = Denominator != 0.f ? Position / Denominator : 0.f;
	}
	else
	{
		Denominator = MyGeometry.Size.Y - Indentation - HandleSize;
		float Position = MyGeometry.Size.Y - LocalPosition.Y;
		Position -= HandleSize * 0.5f + Indentation * 0.5f;

		if (ShowMinMaxValues.Get())
		{
			Denominator -= MaxValueLabelSize.Y + 6.f + MinValueLabelSize.Y + 6.f;
			Position -= MinValueLabelSize.Y + 6.f;
		}

		// Inverse the calculation as top is 0 and bottom is 1
		RelativeValue = Denominator != 0.f ? Position / Denominator : 0.f;
	}

	RelativeValue = FMath::Clamp(RelativeValue, 0.0f, 1.0f) * (MaxValue - MinValue) + MinValue;

	if (bMouseUsesStep)
	{
		const float Direction = HandleValue - RelativeValue;
		const float CurrentStepSize = StepSize.Get();
		if (Direction > CurrentStepSize / 2.0f)
		{
			return FMath::Clamp(HandleValue - CurrentStepSize, MinValue, MaxValue);
		}

		if (Direction < CurrentStepSize / -2.0f)
		{
			return FMath::Clamp(HandleValue + CurrentStepSize, MinValue, MaxValue);
		}

		return HandleValue;
	}

	return RelativeValue;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelMinMaxSlider::CommitValue(const float NewValue)
{
	if (NewValue == GetValue())
	{
		return;
	}

	if (CurrentHandle == ESliderHandle::LowerHandle)
	{
		SetLowerValue(NewValue);

		Invalidate(EInvalidateWidgetReason::Paint);

		OnLowerHandleValueChanged.ExecuteIfBound(LowerHandleValue.Get());
	}

	if (CurrentHandle == ESliderHandle::UpperHandle)
	{
		SetUpperValue(NewValue);

		Invalidate(EInvalidateWidgetReason::Paint);

		OnUpperHandleValueChanged.ExecuteIfBound(UpperHandleValue.Get());
	}
}

FVector2D SVoxelMinMaxSlider::ComputeDesiredSize(float X) const
{
	const FVector2D Size = SSlider::ComputeDesiredSize(X);

	if (!ShowLowerUpperValues.Get())
	{
		return Size;
	}

	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	const FText LowerValueText = FText::AsNumber(LowerHandleValue.Get());
	const FVector2D LowerLabelSize = FontMeasure->Measure(LowerValueText, LowerValueFont.Get());

	const FText UpperValueText = FText::AsNumber(UpperHandleValue.Get());
	const FVector2D UpperLabelSize = FontMeasure->Measure(UpperValueText, UpperValueFont.Get());

	FVector2D TextOffset = FVector2D::Zero();
	if (Orientation == Orient_Vertical)
	{
		TextOffset.X += 12.f + LowerLabelSize.X + UpperLabelSize.X;
	}
	else
	{
		TextOffset.Y += 12.f + LowerLabelSize.Y + UpperLabelSize.Y;
	}

	return Size + TextOffset;
}

int32 SVoxelMinMaxSlider::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	const FText MinValueText = OnMinValueFormat.IsBound() ? OnMinValueFormat.Execute(MinValue) : FText::AsNumber(MinValue);
	const FText MaxValueText = OnMaxValueFormat.IsBound() ? OnMaxValueFormat.Execute(MaxValue) : FText::AsNumber(MaxValue);

	FVector2D MinLabelSize = FVector2D::Zero();
	FVector2D MaxLabelSize = FVector2D::Zero();

	float LeftOffset = 0.f;
	float RightOffset = 0.f;
	if (ShowMinMaxValues.Get())
	{
		MinLabelSize = FontMeasure->Measure(MinValueText, MinValueFont.Get());
		MaxLabelSize = FontMeasure->Measure(MaxValueText, MaxValueFont.Get());

		if (Orientation == Orient_Vertical)
		{
			LeftOffset = 2.f + MinLabelSize.Y + 4.f;
			RightOffset = 2.f + MaxLabelSize.Y + 4.f;
		}
		else
		{
			LeftOffset = 2.f + MinLabelSize.X + 4.f;
			RightOffset = 2.f + MaxLabelSize.X + 4.f;
		}
	}

	const float AllottedWidth = Orientation == Orient_Horizontal ? AllottedGeometry.GetLocalSize().X : AllottedGeometry.GetLocalSize().Y;
	const float AllottedHeight = Orientation == Orient_Horizontal ? AllottedGeometry.GetLocalSize().Y : AllottedGeometry.GetLocalSize().X;

	// calculate slider geometry as if it's a horizontal slider (we'll rotate it later if it's vertical)
	const FVector2f HandleSize = GetThumbImage()->ImageSize;
	const FVector2f HalfHandleSize = HandleSize * 0.5f;
	const float Indentation = IndentHandleAttribute.Get() ? HandleSize.X : 0.0f;
	const float HalfIndentation = Indentation * 0.5f;

	// We clamp to make sure that the slider cannot go out of the slider Length.
	const float LowerSliderPercent = FMath::Clamp(GetLowerNormalizedValue(), 0.0f, 1.0f);
	const float UpperSliderPercent = FMath::Clamp(GetUpperNormalizedValue(), 0.0f, 1.0f);

	const float SliderLength = AllottedWidth - (Indentation + HandleSize.X + LeftOffset + RightOffset);
	const float SliderLowerHandleOffset = LowerSliderPercent * SliderLength;
	const float SliderUpperHandleOffset = UpperSliderPercent * SliderLength;
	const float SliderY = 0.5f * AllottedHeight;

	FVector2D LowerHandleTopLeft = FVector2D(SliderLowerHandleOffset + HalfIndentation + LeftOffset, SliderY - HalfHandleSize.Y);
	FVector2D UpperHandleTopLeft = FVector2D(SliderUpperHandleOffset + HalfIndentation + LeftOffset, SliderY - HalfHandleSize.Y);

	FVector2D SliderStartPoint = FVector2D(HalfHandleSize.X + LeftOffset, SliderY);
	FVector2D SliderEndPoint = FVector2D(AllottedWidth - RightOffset - Indentation, SliderY);

	FGeometry SliderGeometry = AllottedGeometry;

	// rotate the slider 90deg if it's vertical. The 0 side goes on the bottom, the 1 side on the top.
	if (Orientation == Orient_Vertical)
	{
		FSlateRenderTransform SlateRenderTransform = TransformCast<FSlateRenderTransform>(
			Concatenate(Inverse(FVector2f(AllottedWidth, 0)), FQuat2D(FMath::DegreesToRadians(-90.0f))));
		SliderGeometry = AllottedGeometry.MakeChild(
			FVector2f(AllottedWidth, AllottedHeight),
			FSlateLayoutTransform(),
			SlateRenderTransform, FVector2f::ZeroVector);
	}

	const bool bEnabled = ShouldBeEnabled(bParentEnabled);
	const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	// Slider Bar
	{
		const float BarY = SliderStartPoint.Y - Style->BarThickness * 0.5f;
		const FSlateBrush* BarImage = GetBarImage();

		const FLinearColor OuterColor =
			SliderBarOuterColorAttribute.IsSet()
			? SliderBarOuterColorAttribute.Get().GetColor(InWidgetStyle)
			: BarImage->GetTint(InWidgetStyle);

		const FLinearColor InnerColor =
			SliderBarInnerColorAttribute.IsSet()
			? SliderBarInnerColorAttribute.Get().GetColor(InWidgetStyle)
			: BarImage->GetTint(InWidgetStyle);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			SliderGeometry.ToPaintGeometry(
				FVector2D(LowerHandleTopLeft.X - SliderStartPoint.X + HalfHandleSize.X, Style->BarThickness),
				FSlateLayoutTransform(FVector2D(SliderStartPoint.X, BarY))),
			BarImage,
			DrawEffects,
			OuterColor * InWidgetStyle.GetColorAndOpacityTint()
		);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			SliderGeometry.ToPaintGeometry(
				FVector2D(UpperHandleTopLeft.X + HalfHandleSize.X - (LowerHandleTopLeft.X + HalfHandleSize.X), Style->BarThickness),
				FSlateLayoutTransform(FVector2D(LowerHandleTopLeft.X + HalfHandleSize.X, BarY))),
			BarImage,
			DrawEffects,
			InnerColor * InWidgetStyle.GetColorAndOpacityTint()
		);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			SliderGeometry.ToPaintGeometry(
				FVector2D(SliderEndPoint.X - (UpperHandleTopLeft.X + HalfHandleSize.X), Style->BarThickness),
				FSlateLayoutTransform(FVector2D(UpperHandleTopLeft.X + HalfHandleSize.X, BarY))),
			BarImage,
			DrawEffects,
			OuterColor * InWidgetStyle.GetColorAndOpacityTint()
		);
	}

	// Min Max values
	if (ShowMinMaxValues.Get())
	{
		LayerId++;

		FVector2D MinValuePosition;
		if (Orientation == Orient_Horizontal)
		{
			MinValuePosition = FVector2D(2.f, SliderY - MinLabelSize.Y * 0.5f);
		}
		else
		{
			MinValuePosition = FVector2D(AllottedGeometry.Size.X * 0.5f - MinLabelSize.X * 0.5f, AllottedGeometry.Size.Y - MaxLabelSize.Y - 2.f);
		}

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(MinValuePosition)),
			MinValueText,
			MinValueFont.Get(),
			DrawEffects,
			MinValueColorAndOpacity.Get().GetColor(InWidgetStyle));
	
		FVector2D MaxValuePosition;
		if (Orientation == Orient_Horizontal)
		{
			MaxValuePosition = FVector2D(AllottedWidth - MaxLabelSize.Y - 2.f, SliderY - MinLabelSize.Y * 0.5f);
		}
		else
		{
			MaxValuePosition = FVector2D(AllottedGeometry.Size.X * 0.5f - MaxLabelSize.X * 0.5f, 2.f);
		}

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(MaxValuePosition)),
			MaxValueText,
			MaxValueFont.Get(),
			DrawEffects,
			MaxValueColorAndOpacity.Get().GetColor(InWidgetStyle));
	}

	// Thumbs
	{
		LayerId++;

		const FSlateBrush* ThumbImage = GetThumbImage();
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			SliderGeometry.ToPaintGeometry(GetThumbImage()->ImageSize, FSlateLayoutTransform(LowerHandleTopLeft)),
			ThumbImage,
			DrawEffects,
			ThumbImage->GetTint(InWidgetStyle) * SliderLowerHandleColorAttribute.Get().GetColor(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint()
		);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			SliderGeometry.ToPaintGeometry(GetThumbImage()->ImageSize, FSlateLayoutTransform(UpperHandleTopLeft)),
			ThumbImage,
			DrawEffects,
			ThumbImage->GetTint(InWidgetStyle) * SliderUpperHandleColorAttribute.Get().GetColor(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint()
		);
	}

	// Lower/Upper values
	if (ShowLowerUpperValues.Get())
	{
		LayerId++;

		const FText LowerValueText = OnLowerValueFormat.IsBound() ? OnLowerValueFormat.Execute(LowerHandleValue.Get()) : FText::AsNumber(LowerHandleValue.Get());
		const FVector2D LowerLabelSize = FontMeasure->Measure(LowerValueText, LowerValueFont.Get());

		const FText UpperValueText = OnUpperValueFormat.IsBound() ? OnUpperValueFormat.Execute(UpperHandleValue.Get()) : FText::AsNumber(UpperHandleValue.Get());
		const FVector2D UpperLabelSize = FontMeasure->Measure(UpperValueText, UpperValueFont.Get());

		FVector2D LowerLabelPosition = LowerHandleTopLeft;
		if (Orientation == Orient_Horizontal)
		{
			LowerLabelPosition.X -= LowerLabelSize.X * 0.5f;
			LowerLabelPosition.Y += HalfHandleSize.Y + 4.f;
		}
		else
		{
			Swap(LowerLabelPosition.X, LowerLabelPosition.Y);
			LowerLabelPosition.Y = AllottedGeometry.Size.Y - LowerLabelPosition.Y;
			LowerLabelPosition.X -= LowerLabelSize.X + 4.f;
			LowerLabelPosition.Y -= LowerLabelSize.Y * 0.5f + HalfHandleSize.Y;
		}

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(LowerLabelPosition)),
			LowerValueText,
			LowerValueFont.Get(),
			DrawEffects,
			LowerValueColorAndOpacity.Get().GetColor(InWidgetStyle));
	
		FVector2D UpperLabelPosition = UpperHandleTopLeft;
		if (Orientation == Orient_Horizontal)
		{
			UpperLabelPosition.X -= UpperLabelSize.X * 0.5f;

			// If lower handle label overlaps with upper value, move upper value above handle
			if (LowerLabelPosition.X + LowerLabelSize.X + 4.f >= UpperLabelPosition.X)
			{
				UpperLabelPosition.Y -= UpperLabelSize.Y;
			}
			else
			{
				UpperLabelPosition.Y += HalfHandleSize.Y + 4.f;
			}
		}
		else
		{
			Swap(UpperLabelPosition.X, UpperLabelPosition.Y);
			UpperLabelPosition.Y = AllottedGeometry.Size.Y - UpperLabelPosition.Y;
			UpperLabelPosition.Y -= UpperLabelSize.Y * 0.5f + HalfHandleSize.Y;

			// If lower handle label overlaps with upper value, move upper value above handle
			if (LowerLabelPosition.Y + LowerLabelSize.Y + 4.f < UpperLabelPosition.X)
			{
				UpperLabelPosition.X += HandleSize.X + 4.f;
			}
			else
			{
				UpperLabelPosition.X -= UpperLabelSize.X + 4.f;
			}
		}

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(UpperLabelPosition)),
			UpperValueText,
			UpperValueFont.Get(),
			DrawEffects,
			UpperValueColorAndOpacity.Get().GetColor(InWidgetStyle));
	}

	return LayerId;
}

FReply SVoxelMinMaxSlider::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton &&
		!IsLocked())
	{
		const FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		CurrentHandle = DetermineClickedHandle(MyGeometry, LocalMousePosition);
		CachedCursor = GetCursor().Get(EMouseCursor::Default);

		CommitValue(PositionToHandleValue(MyGeometry, MouseEvent.GetScreenSpacePosition()));

		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

	return FReply::Unhandled();
}

FReply SVoxelMinMaxSlider::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton &&
		HasMouseCaptureByUser(MouseEvent.GetUserIndex(), MouseEvent.GetPointerIndex()))
	{
		CurrentHandle = ESliderHandle::None;
		SetCursor(CachedCursor);

		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

FReply SVoxelMinMaxSlider::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (HasMouseCaptureByUser(MouseEvent.GetUserIndex(), MouseEvent.GetPointerIndex()) && !IsLocked())
	{
		SetCursor(Orientation == Orient_Horizontal ? EMouseCursor::ResizeLeftRight : EMouseCursor::ResizeUpDown);
		CommitValue(PositionToHandleValue(MyGeometry, MouseEvent.GetScreenSpacePosition()));
		
		// Release capture for controller/keyboard when switching to mouse
		ResetState();

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SVoxelMinMaxSlider::OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent)
{
	if (HasMouseCaptureByUser(InTouchEvent.GetUserIndex(), InTouchEvent.GetPointerIndex()))
	{
		CommitValue(PositionToHandleValue(MyGeometry, InTouchEvent.GetScreenSpacePosition()));

		// Release capture for controller/keyboard when switching to mouse
		ResetState();

		return FReply::Handled();
	}

	if (!HasMouseCapture())
	{
		if (FSlateApplication::Get().HasTraveledFarEnoughToTriggerDrag(InTouchEvent, PressedScreenSpaceTouchDownPosition, Orientation))
		{
			CachedCursor = GetCursor().Get(EMouseCursor::Default);
			// OnMouseCaptureBegin.ExecuteIfBound();

			CommitValue(PositionToHandleValue(MyGeometry, InTouchEvent.GetScreenSpacePosition()));

			// Release capture for controller/keyboard when switching to mouse
			ResetState();

			return FReply::Handled().CaptureMouse(SharedThis(this));
		}
	}

	return FReply::Unhandled();
}

FReply SVoxelMinMaxSlider::OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent)
{
	if (HasMouseCaptureByUser(InTouchEvent.GetUserIndex(), InTouchEvent.GetPointerIndex()))
	{
		SetCursor(CachedCursor);
		// OnMouseCaptureEnd.ExecuteIfBound();

		CommitValue(PositionToHandleValue(MyGeometry, InTouchEvent.GetScreenSpacePosition()));

		// Release capture for controller/keyboard when switching to mouse.
		ResetState();

		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

void SVoxelMinMaxSlider::ResetState()
{
	if (bControllerInputCaptured)
	{
		// OnControllerCaptureEnd.ExecuteIfBound();
		bControllerInputCaptured = false;
	}
}
