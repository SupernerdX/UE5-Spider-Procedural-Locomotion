// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphPreviewDepthSlider.h"
#include "Fonts/FontMeasure.h"

void SVoxelGraphPreviewDepthSlider::Construct(const FArguments& InArgs)
{
	ValueText = InArgs._ValueText;
	Value = InArgs._Value;
	OnValueChanged = InArgs._OnValueChanged;
	MinValue = InArgs._MinValue;
	MaxValue = InArgs._MaxValue;

	MinValueTypeInterface = MakeShared<TNumericUnitTypeInterface<float>>(EUnit::Meters);
	MaxValueTypeInterface = MakeShared<TNumericUnitTypeInterface<float>>(EUnit::Meters);

	Options.SetMinimumFractionalDigits(2);
	Options.SetMaximumFractionalDigits(2);

	CreateMinMaxValue(true);
	CreateMinMaxValue(false);

	ChildSlot
	[
		SNew(SBox)
		.Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(0.f, -2.f, 0.f, 3.f)
			[
				SNew(STextBlock)
				.Text(INVTEXT("Depth"))
				.Font(FAppStyle::GetFontStyle("Persona.RetargetManager.SmallBoldFont"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MaxValueBox.ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.FillHeight(0.9f)
			[
				SNew(SBox)
				.MinDesiredHeight(200.f)
				.HAlign(HAlign_Center)
				[
					SAssignNew(Slider, SSlider)
					.ToolTip(GetToolTip())
					.Orientation(Orient_Vertical)
					.MinValue(MinValue)
					.MaxValue(MaxValue)
					.StepSize(1.f)
					.Value(Value)
					.OnValueChanged_Lambda([this](const float NewValue)
					{
						OnValueChanged.ExecuteIfBound(NewValue);
						Value = NewValue;
					})
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MinValueBox.ToSharedRef()
			]
		]
	];
}

int32 SVoxelGraphPreviewDepthSlider::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FSlateBrush* CommentCalloutArrow = FAppStyle::GetBrush(TEXT("Graph.Node.CommentArrow"));
	const FSlateFontInfo CommentFont = FAppStyle::GetFontStyle(TEXT("Graph.Node.CommentFont"));
	const FVector2D CommentBubblePadding = FAppStyle::GetVector(TEXT("Graph.Node.Comment.BubblePadding"));

	class SSliderAccessor : public SSlider
	{
	public:
		const FSlateBrush* GetThumbBrush() const
		{
			return GetThumbImage();
		}
	};

	LayerId = SCompoundWidget::OnPaint(
		Args,
		AllottedGeometry,
		MyCullingRect,
		OutDrawElements,
		LayerId,
		InWidgetStyle,
		bParentEnabled);

	const FGeometry& SliderGeometry = Slider->GetPaintSpaceGeometry();
	const FVector2D Size = SliderGeometry.GetLocalSize();
	const FVector2D TopPosition = AllottedGeometry.AbsoluteToLocal(SliderGeometry.LocalToAbsolute(FVector2D::ZeroVector));

	FString CommentText = GetValueText();

	const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FVector2D CommentTextSize = FontMeasureService->Measure(CommentText, CommentFont) + CommentBubblePadding * 2;

	const float IndentSize = reinterpret_cast<SSliderAccessor*>(Slider.Get())->GetThumbBrush()->ImageSize.X * 2.f;

	float XOffset = TopPosition.X - CommentCalloutArrow->ImageSize.X;
	float YOffset = TopPosition.Y + (Size.Y - IndentSize) * (1.f - Slider->GetNormalizedValue()) + CommentCalloutArrow->ImageSize.Y * 0.5f;
	const FVector2D Offset = FVector2D(
		XOffset - CommentTextSize.X,
		FMath::Clamp(YOffset - CommentTextSize.Y * 0.5f + CommentCalloutArrow->ImageSize.Y * 0.5f, TopPosition.Y, TopPosition.Y + Size.Y - CommentTextSize.Y));

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(
			CommentTextSize,
			FSlateLayoutTransform(Offset)),
		FAppStyle::GetBrush(TEXT("Graph.Node.CommentBubble")),
		ESlateDrawEffect::None,
		FLinearColor(0.15f, 0.15f, 0.15f, 0.8f));

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(
			CommentCalloutArrow->ImageSize,
			FSlateLayoutTransform(FVector2D(XOffset, YOffset)),
			FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-90.f)))),
		CommentCalloutArrow,
		ESlateDrawEffect::None,
		FLinearColor(0.15f, 0.15f, 0.15f, 0.8f));

	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId + 2,
		AllottedGeometry.ToPaintGeometry(
			CommentTextSize,
			FSlateLayoutTransform(Offset + CommentBubblePadding)),
		CommentText,
		CommentFont,
		ESlateDrawEffect::None,
		FAppStyle::GetColor(TEXT("Graph.Node.Comment.TextColor")));

	return LayerId;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelGraphPreviewDepthSlider::ResetValue(const float NewValue, const bool bKeepRange)
{
	if (NewValue < 100.f &&
		NewValue > -100.f)
	{
		Value = NewValue;
		if (bKeepRange)
		{
			const float HalfRange = (MaxValue - MinValue) / 2.f;
			MinValue = NewValue - HalfRange;
			MaxValue = NewValue + HalfRange;
		}
		else
		{
			MinValue = -100.f;
			MaxValue = 100.f;
		}
	}
	else
	{
		Value = NewValue;
		if (bKeepRange)
		{
			const float HalfRange = (MaxValue - MinValue) / 2.f;
			MinValue = NewValue - HalfRange;
			MaxValue = NewValue + HalfRange;
		}
		else
		{
			const int32 Power = FMath::Max(2, FMath::FloorToInt(FMath::LogX(10.f, FMath::Abs(Value))));
			MinValue = NewValue - FMath::Pow(10.f, Power);
			MaxValue = NewValue + FMath::Pow(10.f, Power);
		}
	}

	Slider->SetValue(Value);
	Slider->SetMinAndMaxValues(MinValue, MaxValue);
}

FString SVoxelGraphPreviewDepthSlider::GetValueText() const
{
	return FVoxelUtilities::DistanceToString(Value, 2);
}

DEFINE_PRIVATE_ACCESS(SNumericEntryBox<float>, EditableText)

void SVoxelGraphPreviewDepthSlider::CreateMinMaxValue(const bool bMin)
{
	SAssignNew((bMin ? MinValueBox : MaxValueBox), SNumericEntryBox<float>)
      	.Justification(ETextJustify::Center)
      	.Value(this, &SVoxelGraphPreviewDepthSlider::GetMinMaxValue, bMin)
		.EditableTextBoxStyle(&FAppStyle::GetWidgetStyle<FEditableTextBoxStyle>("CurveTableEditor.Cell.Text"))
		.TypeInterface(bMin ? MinValueTypeInterface : MaxValueTypeInterface)
		.MinFractionalDigits(1)
		.MaxFractionalDigits(1)
      	.OnValueCommitted(this, &SVoxelGraphPreviewDepthSlider::UpdateRange, bMin);

	const TSharedPtr<SEditableText> EditableTextBox = PrivateAccess::EditableText(bMin ? *MinValueBox.Get() : *MaxValueBox.Get());
	EditableTextBox->SetToolTipText(bMin ? INVTEXT("Min Depth") : INVTEXT("Max Depth"));
}

TOptional<float> SVoxelGraphPreviewDepthSlider::GetMinMaxValue(const bool bMin) const
{
	return (bMin ? MinValue : MaxValue) / 100.f;
}

void SVoxelGraphPreviewDepthSlider::UpdateRange(float NewMinMaxValue, ETextCommit::Type, const bool bMin)
{
	NewMinMaxValue *= 100.f;

	float& TargetValue = bMin ? MinValue : MaxValue;
	float& OpposingValue = bMin ? MaxValue : MinValue;

	TargetValue = NewMinMaxValue;
	if (MaxValue < MinValue)
	{
		OpposingValue = TargetValue;
	}

	Slider->SetMinAndMaxValues(MinValue, MaxValue);

	const float NewValue = FMath::Clamp(Value, MinValue, MaxValue);
	if (NewValue != Value)
	{
		Value = NewValue;
		Slider->SetValue(Value);
		OnValueChanged.ExecuteIfBound(NewValue);
	}

	{
		EUnit Unit;
		int32 NumFractionalDigits;
		FVoxelUtilities::DistanceToString(MinValue, 2, Unit, NumFractionalDigits);

		MinValueTypeInterface->UserDisplayUnits = Unit;

		MinValueBox->SetMinFractionalDigits(NumFractionalDigits);
		MinValueBox->SetMaxFractionalDigits(NumFractionalDigits);
	}

	{
		EUnit Unit;
		int32 NumFractionalDigits;
		FVoxelUtilities::DistanceToString(MaxValue, 2, Unit, NumFractionalDigits);

		MaxValueTypeInterface->UserDisplayUnits = Unit;

		MaxValueBox->SetMinFractionalDigits(NumFractionalDigits);
		MaxValueBox->SetMaxFractionalDigits(NumFractionalDigits);
	}
}