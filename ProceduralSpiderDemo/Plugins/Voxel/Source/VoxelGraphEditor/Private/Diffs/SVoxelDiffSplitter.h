// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "AsyncDetailViewDiff.h"
#include "Widgets/SCompoundWidget.h"

template<typename T>
class SVoxelDiffSplitter : public SCompoundWidget
{
public:
	struct FPanel
	{
		TAttribute<bool> IsReadonly;
		TDelegate<void(const TFunction<ETreeTraverseControl(const T&, const T&, ETreeDiffResult)>&)> IterateChildren;
		TDelegate<FSlateRect(const T&, bool)> GetPaintSpaceBounds;
		TDelegate<FSlateRect(const T&, bool)> GetTickSpaceBounds;
	};

	class FSlot : public TSlotBase<FSlot>
	{
	public:
		FSlot() = default;

		SLATE_SLOT_BEGIN_ARGS(FSlot, TSlotBase<FSlot>)
			SLATE_ATTRIBUTE(float, Value)
			SLATE_ARGUMENT(TSharedPtr<SWidget>, Widget)
			SLATE_ATTRIBUTE(bool, IsReadonly)
			SLATE_EVENT(TDelegate<void(const TFunction<ETreeTraverseControl(const T&, const T&, ETreeDiffResult)>&)>, IterateChildren)
			SLATE_EVENT(TDelegate<FSlateRect(const T&, bool)>, GetPaintSpaceBounds)
			SLATE_EVENT(TDelegate<FSlateRect(const T&, bool)>, GetTickSpaceBounds)
		SLATE_SLOT_END_ARGS()
	};
	static typename FSlot::FSlotArguments Slot()
	{
		return typename FSlot::FSlotArguments(MakeUnique<FSlot>());
	}

	VOXEL_SLATE_ARGS()
	{
		SLATE_SLOT_ARGUMENT(FSlot, Slots)
		SLATE_EVENT(TDelegate<bool(const T&, const T&, ETreeDiffResult)>, CanCopyValue)
		SLATE_EVENT(TDelegate<void(const T&, const T&, ETreeDiffResult)>, CopyValue)
	};

	void Construct(const FArguments& InArgs)
	{
		CanCopyValue = InArgs._CanCopyValue;
		CopyValue = InArgs._CopyValue;
		Splitter =
			SNew(SSplitter)
			.PhysicalSplitterHandleSize(5.f);

		for (const typename FSlot::FSlotArguments& SlotArgs : InArgs._Slots)
		{
			AddSlot(SlotArgs);
		}

		ChildSlot
		[
			Splitter.ToSharedRef()
		];
	}

	void AddSlot(const typename FSlot::FSlotArguments& SlotArgs, int32 Index = INDEX_NONE)
	{
		if (Index == -1)
		{
			Index = Panels.Num();
		}

		Splitter->AddSlot(Index)
		.Value(SlotArgs._Value)
		[
			SNew(SBox)
			.Padding(15.f, 0.f)
			[
				SlotArgs._Widget.ToSharedRef()
			]
		];

		Panels.Insert({ SlotArgs._IsReadonly, SlotArgs._IterateChildren, SlotArgs._GetPaintSpaceBounds, SlotArgs._GetTickSpaceBounds }, Index);
	}

	//~ Begin SCompoundWidget Interface
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		const int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		const bool bParentEnabled) const override
	{
		int32 MaxLayerId = LayerId;

		MaxLayerId = Splitter->Paint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, MaxLayerId, InWidgetStyle, bParentEnabled);
		++MaxLayerId;

		TMap<FSlateRect, FLinearColor> RowHighlights;
		for (int32 Index = 0; Index < Panels.Num(); Index++)
		{
			const FPanel& LeftPanel = Panels[Index];
			if (!LeftPanel.IterateChildren.IsBound())
			{
				continue;
			}

			FSlateRect PrevLeftElementRect;
			FSlateRect PrevRightElementRect;

			T LastSeenRightElement;
			T LastSeenLeftElement;

			LeftPanel.IterateChildren.Execute([&](const T& ValueA, const T& ValueB, ETreeDiffResult DiffResult) -> ETreeTraverseControl
			{
				const FLinearColor Color = FLinearColor(0.f, 1.f, 1.f, .7f);//GetRowHighlightColor.Execute(DiffNode); // TODO:

				FSlateRect LeftElementRect;
				if (ValueA)
				{
					LastSeenLeftElement = ValueA;
					LeftElementRect = LeftPanel.GetPaintSpaceBounds.Execute(ValueA, !ValueB);
				}

				if (!LeftElementRect.IsValid() &&
					PrevLeftElementRect.IsValid())
				{
					LeftElementRect = PrevLeftElementRect;
					LeftElementRect.Top = LeftElementRect.Bottom;
				}

				if (LeftElementRect.IsValid() &&
					DiffResult != ETreeDiffResult::Identical)
				{
					RowHighlights.Add(LeftElementRect, Color);
				}

				const int32 RightIndex = Index + 1;
				FSlateRect RightElementRect;
				if (Panels.IsValidIndex(RightIndex))
				{
					const FPanel& RightPanel = Panels[RightIndex];
					if (ValueB)
					{
						LastSeenRightElement = ValueB;
						RightElementRect = RightPanel.GetPaintSpaceBounds.Execute(ValueB, !ValueA);
					}

					if (!RightElementRect.IsValid() &&
						PrevRightElementRect.IsValid())
					{
						RightElementRect = PrevRightElementRect;
						RightElementRect.Top = RightElementRect.Bottom;
					}

					if (RightElementRect.IsValid() &&
						DiffResult != ETreeDiffResult::Identical)
					{
						RowHighlights.Add(RightElementRect, Color);
					}

					INLINE_LAMBDA
					{
						if (!LeftElementRect.IsValid() ||
							!RightElementRect.IsValid() ||
							DiffResult == ETreeDiffResult::Identical)
						{
							return;
						}

						FLinearColor FillColor = Color.Desaturate(.3f) * FLinearColor(0.053f,0.053f,0.053f);
						FillColor.A = 0.43f;

						const FLinearColor OutlineColor = Color;
						PaintPropertyConnector(OutDrawElements, MaxLayerId, LeftElementRect, RightElementRect, FillColor, OutlineColor);
						++MaxLayerId;

						if (!RightPanel.IsReadonly.Get(true) &&
							CanCopyValue.Execute(LastSeenLeftElement, LastSeenRightElement, DiffResult))
						{
							PaintCopyPropertyButton(
								OutDrawElements,
								MaxLayerId,
								ValueA,
								ValueB,
								LeftElementRect,
								RightElementRect,
								ECopyDirection::CopyLeftToRight);
						}
						if (!LeftPanel.IsReadonly.Get(true) &&
							CanCopyValue.Execute(LastSeenRightElement, LastSeenLeftElement, DiffResult))
						{
							PaintCopyPropertyButton(
								OutDrawElements,
								MaxLayerId,
								ValueA,
								ValueB,
								LeftElementRect,
								RightElementRect,
								ECopyDirection::CopyRightToLeft);
						}
						++MaxLayerId;
					};
				}

				PrevLeftElementRect = MoveTemp(LeftElementRect);
				PrevRightElementRect = MoveTemp(RightElementRect);

				if (!ValueA ||
					!ValueB)
				{
					return ETreeTraverseControl::SkipChildren;
				}

				return ETreeTraverseControl::Continue;
			});
		}

		for (const auto& It : RowHighlights)
		{
			FLinearColor FillColor = It.Value.Desaturate(.3f) * FLinearColor(0.053f,0.053f,0.053f);
			FillColor.A = 0.43f;

			FPaintGeometry Geometry(
				It.Key.GetTopLeft() + FVector2D{0.f,2.f},
				It.Key.GetSize() - FVector2D{0.f,4.f},
				1.f
			);

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 10,
				Geometry,
				FAppStyle::GetBrush("WhiteBrush"),
				ESlateDrawEffect::None,
				FillColor
			);
		}

		return MaxLayerId;
	}
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		HoveredCopyButton = {};

		const FVector2D MousePosition = MouseEvent.GetScreenSpacePosition();

		for (int32 Index = 0; Index < Panels.Num() - 1; Index++)
		{
			const FPanel& LeftPanel = Panels[Index];
			const FPanel& RightPanel = Panels[Index + 1];

			if (LeftPanel.IsReadonly.Get(true) &&
				RightPanel.IsReadonly.Get(true))
			{
				continue;
			}

			T LastSeenRightElement;
			T LastSeenLeftElement;

			LeftPanel.IterateChildren.Execute([&](const T& ValueA, const T& ValueB, ETreeDiffResult DiffResult) -> ETreeTraverseControl
			{
				FSlateRect LeftPropertyRect;
				if (ValueA)
				{
					LastSeenLeftElement = ValueA;
					LeftPropertyRect = LeftPanel.GetTickSpaceBounds.Execute(ValueA, !ValueB);
				}

				FSlateRect RightPropertyRect;
				if (ValueB)
				{
					LastSeenRightElement = ValueB;
					RightPropertyRect = RightPanel.GetTickSpaceBounds.Execute(ValueB, !ValueA);
				}

				if (DiffResult == ETreeDiffResult::Identical)
				{
					return ETreeTraverseControl::Continue;
				}

				if (LeftPropertyRect.IsValid() &&
					!RightPanel.IsReadonly.Get(true))
				{
					const FSlateRect CopyButtonZoneLeftToRight = FSlateRect(
						LeftPropertyRect.Right,
						LeftPropertyRect.Top,
						LeftPropertyRect.Right + 15.f,
						LeftPropertyRect.Bottom
					);

					if (CopyButtonZoneLeftToRight.ContainsPoint(MousePosition))
					{
						if (CanCopyValue.Execute(LastSeenLeftElement, LastSeenRightElement, DiffResult))
						{
							HoveredCopyButton = {
								LastSeenLeftElement,
								LastSeenRightElement,
								DiffResult,
								ECopyDirection::CopyLeftToRight
							};
						}
						return ETreeTraverseControl::Break;
					}
				}

				if (RightPropertyRect.IsValid() &&
					!LeftPanel.IsReadonly.Get(true))
				{
					const FSlateRect CopyButtonZoneRightToLeft = FSlateRect(
						RightPropertyRect.Left - 15.f,
						RightPropertyRect.Top,
						RightPropertyRect.Left,
						RightPropertyRect.Bottom
					);


					if (CopyButtonZoneRightToLeft.ContainsPoint(MousePosition))
					{
						if (CanCopyValue.Execute(LastSeenRightElement, LastSeenLeftElement, DiffResult))
						{
							HoveredCopyButton = {
								LastSeenRightElement,
								LastSeenLeftElement,
								DiffResult,
								ECopyDirection::CopyRightToLeft
							};
						}
						return ETreeTraverseControl::Break;
					}
				}

				return ETreeTraverseControl::Continue;
			});

			if (HoveredCopyButton.CopyDirection != ECopyDirection::None)
			{
				break;
			}
		}

		return SCompoundWidget::OnMouseMove(MyGeometry, MouseEvent);
	}
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		HoveredCopyButton = {};
		SCompoundWidget::OnMouseLeave(MouseEvent);
	}
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
		{
			return FReply::Unhandled();
		}

		if (HoveredCopyButton.CopyDirection == ECopyDirection::None)
		{
			return FReply::Unhandled();
		}

		CopyValue.Execute(HoveredCopyButton.SourceElement, HoveredCopyButton.DestinationElement, HoveredCopyButton.DiffResult);
		return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
	}
	//~ End SCompoundWidget Interface

private:
	enum class ECopyDirection
	{
		None,
		CopyLeftToRight,
		CopyRightToLeft,
	};

	struct FCopyPropertyButton
	{
		T SourceElement;
		T DestinationElement;
		ETreeDiffResult DiffResult = ETreeDiffResult::Invalid;
		ECopyDirection CopyDirection = ECopyDirection::None;
	};

	void PaintPropertyConnector(
		FSlateWindowElementList& OutDrawElements,
		const int32 LayerId,
		const FSlateRect& LeftPropertyRect,
		const FSlateRect& RightPropertyRect,
		const FLinearColor& FillColor,
		const FLinearColor& OutlineColor) const
	{
		FVector2D TopLeft = LeftPropertyRect.GetTopRight();
		FVector2D BottomLeft = LeftPropertyRect.GetBottomRight();

		FVector2D TopRight = RightPropertyRect.GetTopLeft();
		FVector2D BottomRight = RightPropertyRect.GetBottomLeft();

		{
			constexpr float YPadding = 2.f;
			if (BottomLeft.Y - TopLeft.Y > YPadding * 2.f)
			{
				BottomLeft.Y -= YPadding;
				TopLeft.Y += YPadding;
			}
			if (BottomRight.Y - TopRight.Y > YPadding * 2.f)
			{
				BottomRight.Y -= YPadding;
				TopRight.Y += YPadding;
			}
		}

		TArray<FSlateVertex> FillVertices;
		TArray<SlateIndex> FillIndices;
		TArray<FVector2D> TopBoarderLine;
		TArray<FVector2D> BottomBoarderLine;

		const auto AddVertex = [&FillVertices, &FillIndices](const FVector2D& Position, const FColor& VertColor)
		{
			FillVertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
				FSlateRenderTransform(),
				FVector2f(Position),
				{ 0.f,0.f },
				VertColor
			));

			if (FillVertices.Num() >= 3)
			{
				FillIndices.Add(FillVertices.Num() - 3);
				FillIndices.Add(FillVertices.Num() - 2);
				FillIndices.Add(FillVertices.Num() - 1);
			}
		};

		constexpr float StepSize = 1.f / 30.f;
		constexpr float InterpBorder = .3f;

		float Alpha = 0.f;
		while (true)
		{
			constexpr float Exp = 3.f;
			const float TopAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
			const float BottomAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
			const double TopX = FMath::Lerp(TopLeft.X, TopRight.X, TopAlpha);
			const double BottomX = FMath::Lerp(TopLeft.X, TopRight.X, BottomAlpha);

			constexpr float InterpolationRange = 1.f - 2.f * InterpBorder;

			const float TopTransformedAlpha = FMath::Clamp((TopAlpha - InterpBorder) / InterpolationRange, 0.f, 1.f);
			const float BottomTransformedAlpha = FMath::Clamp((BottomAlpha - InterpBorder) / InterpolationRange, 0.f, 1.f);

			const double TopY = FMath::InterpEaseInOut(TopLeft.Y, TopRight.Y, TopTransformedAlpha, Exp);
			const double BottomY = FMath::InterpEaseInOut(BottomLeft.Y, BottomRight.Y, BottomTransformedAlpha, Exp);

			FLinearColor ColumnColor = FillColor;
			if (Alpha <= 0.5f)
			{
				ColumnColor.A = FMath::InterpEaseOut(FillColor.A, 1.f, Alpha * 2.f, 2.f);
			}
			else
			{
				ColumnColor.A = FMath::InterpEaseIn( 1.f,FillColor.A, (Alpha - 0.5f) * 2.f, 2.f);
			}

			TopBoarderLine.Add({ TopX, TopY });
			FLinearColor TopColor = ColumnColor.Desaturate(0.5f);
			AddVertex(TopBoarderLine.Last(), TopColor.ToFColorSRGB());

			BottomBoarderLine.Add({ BottomX, BottomY });
			FLinearColor BottomColor = ColumnColor;
			AddVertex(BottomBoarderLine.Last(), BottomColor.ToFColorSRGB());

			if (Alpha >= 1.f)
			{
				break;
			}

			Alpha = FMath::Clamp(Alpha + StepSize, 0.f, 1.f);
		}

		const FSlateResourceHandle ResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*FAppStyle::GetBrush("WhiteBrush"));
		FSlateDrawElement::MakeCustomVerts(
			OutDrawElements,
			LayerId,
			ResourceHandle,
			FillVertices,
			FillIndices,
			nullptr,
			0,
			0,
			ESlateDrawEffect::None
		);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			FPaintGeometry(),
			TopBoarderLine,
			ESlateDrawEffect::None,
			OutlineColor,
			true,
			0.5f
		);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			FPaintGeometry(),
			BottomBoarderLine,
			ESlateDrawEffect::None,
			OutlineColor,
			true,
			0.5f
		);
	}

	void PaintCopyPropertyButton(
		FSlateWindowElementList& OutDrawElements,
		const int32 LayerId,
		const T& ValueA,
		const T& ValueB,
		const FSlateRect& LeftPropertyRect,
		const FSlateRect& RightPropertyRect,
		const ECopyDirection CopyDirection) const
	{
		constexpr float ButtonScale = 15.f;
		FPaintGeometry Geometry;
		const FSlateBrush* Brush;
		FLinearColor ButtonColor = FStyleColors::Foreground.GetSpecifiedColor();

		switch (CopyDirection)
		{
		case ECopyDirection::CopyLeftToRight:
		{
			if (LeftPropertyRect.GetSize().Y < UE_SMALL_NUMBER)
			{
				return;
			}

			Geometry = FPaintGeometry(
				FVector2D(LeftPropertyRect.Right, LeftPropertyRect.GetCenter().Y - 0.5f * ButtonScale),
				FVector2D(ButtonScale),
				1.f
			);

			Brush = FAppStyle::GetBrush("BlueprintDif.CopyPropertyRight");

			if (HoveredCopyButton.CopyDirection == ECopyDirection::CopyLeftToRight &&
				HoveredCopyButton.SourceElement == ValueA)
			{
				ButtonColor = FStyleColors::ForegroundHover.GetSpecifiedColor();
			}
			break;
		}
		case ECopyDirection::CopyRightToLeft:
		{
			if (RightPropertyRect.GetSize().Y < UE_SMALL_NUMBER)
			{
				return;
			}

			Geometry = FPaintGeometry(
				FVector2D(RightPropertyRect.Left - ButtonScale, RightPropertyRect.GetCenter().Y - 0.5f * ButtonScale),
				FVector2D(ButtonScale),
				1.f
			);

			Brush = FAppStyle::GetBrush("BlueprintDif.CopyPropertyLeft");

			if (HoveredCopyButton.CopyDirection == ECopyDirection::CopyRightToLeft &&
				HoveredCopyButton.SourceElement == ValueB)
			{
				ButtonColor = FStyleColors::ForegroundHover.GetSpecifiedColor();
			}
			break;
		}
		default: return;
		}

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			Geometry,
			Brush,
			ESlateDrawEffect::None,
			ButtonColor
		);
	}

private:
	TSharedPtr<SSplitter> Splitter;

	TArray<FPanel> Panels;
	FCopyPropertyButton HoveredCopyButton;

private:
	TDelegate<bool(const T&, const T&, ETreeDiffResult)> CanCopyValue;
	TDelegate<void(const T&, const T&, ETreeDiffResult)> CopyValue;
};
