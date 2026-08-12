// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "SVoxelGraphPreview.h"
#include "ToolMenus.h"
#include "VoxelGraph.h"
#include "VoxelEdGraph.h"
#include "Engine/Texture2D.h"
#include "VoxelGraphToolkit.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "Nodes/VoxelGraphNode.h"
#include "Styling/ToolBarStyle.h"
#include "VoxelTerminalGraphRuntime.h"
#include "Preview/VoxelPreviewHandler.h"
#include "Widgets/SVoxelGraphPreviewImage.h"
#include "Widgets/SVoxelGraphPreviewStats.h"
#include "Widgets/SVoxelGraphPreviewRuler.h"
#include "Widgets/SVoxelGraphPreviewScale.h"
#include "Widgets/SVoxelGraphPreviewDepthSlider.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Preview/VoxelScalarPreviewHandler.h"
#include "Preview/VoxelDoublePreviewHandlers.h"
#include "Preview/VoxelFloatPreviewHandlers.h"
#include "Preview/VoxelDistanceFieldPreviewHandlers.h"
#include "Customizations/VoxelGraphPreviewSettingsCustomization.h"

#if VOXEL_ENGINE_VERSION >= 506
#include "ViewportToolbar/UnrealEdViewportToolbar.h"
#endif

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FToolBarStyle& ToolbarStyle = ConstCast(FAppStyle::GetWidgetStyle<FToolBarStyle>("EditorViewportToolBar"));
	ToolbarStyle.ComboButtonStyle.DownArrowImage = *FAppStyle::GetBrush("EditorViewportToolBar.MenuDropdown");
}

void SVoxelGraphPreview::Construct(const FArguments& Args)
{
	VOXEL_FUNCTION_COUNTER();

	TSharedPtr<SImage> Image;
	TSharedPtr<SScaleBox> PreviewScaleBox;
	TSharedPtr<SVoxelGraphPreviewScale> PreviewScale;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
#if VOXEL_ENGINE_VERSION == 506
			SNew(SBox)
			.Visibility_Lambda([]
			{
				return
					UE::UnrealEd::ShowNewViewportToolbars()
					? EVisibility::Visible
					: EVisibility::Collapsed;
			})
			[
				CreateToolbar(Args._Toolkit, true)
			]
#else
			SNullWidget::NullWidget
#endif
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SAssignNew(PreviewScaleBox, SScaleBox)
					.Stretch(EStretch::ScaleToFill)
					[
						SAssignNew(PreviewImage, SVoxelGraphPreviewImage)
						.Width_Lambda([this]() -> float
						{
							const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
							if (!TerminalGraph)
							{
								return 512;
							}

							return TerminalGraph->PreviewConfig.Resolution;
						})
						.Height_Lambda([this]() -> float
						{
							const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
							if (!TerminalGraph)
							{
								return 512;
							}

							return TerminalGraph->PreviewConfig.Resolution;
						})
						.Cursor(EMouseCursor::Crosshairs)
						[
							SAssignNew(Image, SImage)
							.Image_Lambda([this]
							{
								return Brush ? Brush->GetSlateBrush() : nullptr;
							})
						]
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Bottom)
				[
					SNew(SBox)
					.Visibility(EVisibility::HitTestInvisible)
					[
						SAssignNew(PreviewScale, SVoxelGraphPreviewScale)
						.Resolution_Lambda([this]
						{
							const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
							if (!TerminalGraph)
							{
								return 512;
							}

							return TerminalGraph->PreviewConfig.Resolution;
						})
						.Value_Lambda([this]
						{
							return GetPixelToWorld().GetScaleVector().X;
						})
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SBox)
					.Visibility(EVisibility::HitTestInvisible)
					[
						SAssignNew(PreviewRuler, SVoxelGraphPreviewRuler)
						.Resolution_Lambda([this]
						{
							const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
							if (!TerminalGraph)
							{
								return 512;
							}

							return TerminalGraph->PreviewConfig.Resolution;
						})
						.Value_Lambda([this]
						{
							return GetPixelToWorld().GetScaleVector().X;
						})
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(.0f, .0f, .0f, .3f))
					.Visibility_Lambda([this]
					{
						return
							!FuturePreviewHandler.IsComplete() &&
							FPlatformTime::Seconds() - ProcessingStartTime > 1.f
							? EVisibility::Visible
							: EVisibility::Hidden;
						})
					[
						SNew(SScaleBox)
						.IgnoreInheritedScale(true)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SCircularThrobber)
						]
					]
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.Visibility(EVisibility::HitTestInvisible)
				.Padding(FMargin(20.f, 0.f))
				[
					SNew(STextBlock)
					.Font(FVoxelEditorUtilities::Font())
					.Visibility_Lambda([this]
					{
						return Message.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
					})
					.Text_Lambda([this]
					{
						return FText::FromString(Message);
					})
					.ColorAndOpacity(FLinearColor::White)
					.ShadowOffset(FVector2D(1.f))
					.ShadowColorAndOpacity(FLinearColor::Black)
					.AutoWrapText(true)
					.Justification(ETextJustify::Center)
				]
			]
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.f, 3.f)
				[
					SNew(SBox)
#if VOXEL_ENGINE_VERSION == 506
					.Visibility_Lambda([]
					{
						return
							UE::UnrealEd::ShowOldViewportToolbars()
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
#endif
					[
						CreateToolbar(Args._Toolkit, false)
					]
				]
				+ SVerticalBox::Slot()
				.Padding(5.f, 3.f)
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Left)
					.FillWidth(1.f)
					.Padding(0.f, 0.f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::Get().GetBrush("EditorViewportToolBar.Group"))
						.Padding(5.f)
						[
							SNew(SVoxelDetailText)
							.Text(INVTEXT("Left-click the preview to debug values"))
						]
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.AutoWidth()
					.Padding(0.f, 0.f)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::Get().GetBrush("EditorViewportToolBar.Group"))
						.Padding(1.f, 5.f, 1.f, 5.f)
						.OnMouseButtonDown_Lambda([](const FGeometry&, const FPointerEvent&)
						{
							return FReply::Handled();
						})
						.OnMouseButtonUp_Lambda([](const FGeometry&, const FPointerEvent&)
						{
							return FReply::Handled();
						})
						.OnMouseMove_Lambda([](const FGeometry&, const FPointerEvent&)
						{
							return FReply::Handled();
						})
						[
							SNew(SBox)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Fill)
							.HeightOverride(150.f)
							[
								SAssignNew(DepthSliderContainer, SBox)
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.Padding(5.f, 3.f)
				.FillHeight(1.f)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::Get().GetBrush("EditorViewportToolBar.Group"))
					.Padding(5.f)
					[
						SNew(SBox)
						.MaxDesiredWidth_Lambda([this, PreviewScale]() -> FOptionalSize
						{
							const FVector2D ScaleSize = PreviewScale->GetCachedGeometry().GetLocalSize();
							const FVector2D OverallSize = GetCachedGeometry().GetLocalSize();
							return FMath::Max(OverallSize.X - ScaleSize.X - 10.f, 100.f);
						})
						[
							SAssignNew(PreviewStats, SVoxelGraphPreviewStats)
							.Padding(2.f)
							.BulletPointVisibility(EVisibility::Collapsed)
							.BorderImage(FAppStyle::GetBrush("NoBrush"))
							.RowStyle(&FAppStyle::GetWidgetStyle<FTableRowStyle>("PropertyTable.InViewport.Row"))
							.ListViewStyle(&FAppStyle::GetWidgetStyle<FTableViewStyle>("PropertyTable.InViewport.ListView"))
						]
					]
				]
			]
		]
	];

	ExternalPreviewStats = SNew(SVoxelGraphPreviewStats);

	PreviewRuler->WeakSizeWidget = PreviewImage;
	PreviewScale->SizeWidget = PreviewImage;

	Image->SetOnMouseMove(FPointerEventHandler::CreateLambda([this](const FGeometry&, const FPointerEvent) -> FReply
	{
		if (!bIsCoordinateLocked)
		{
			UpdateStats();
		}

		return FReply::Unhandled();
	}));

	UpdateStats();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelGraphPreview::SetTerminalGraph(UVoxelTerminalGraph* NewTerminalGraph)
{
	bUpdateQueued = true;

	WeakTerminalGraph = NewTerminalGraph;

	if (!NewTerminalGraph)
	{
		OnChangedPtr.Reset();
		DepthSlider.Reset();
		DepthSliderContainer->SetContent(SNullWidget::NullWidget);
		return;
	}

	OnChangedPtr = MakeSharedVoid();

	GVoxelGraphTracker->OnEdGraphChanged(NewTerminalGraph->GetEdGraph()).Add(FOnVoxelGraphChanged::Make(OnChangedPtr, this, [this]
	{
		bUpdateQueued = true;
	}));
	GVoxelGraphTracker->OnParameterValueChanged(NewTerminalGraph->GetGraph()).Add(FOnVoxelGraphChanged::Make(OnChangedPtr, this, [this]
	{
		bUpdateQueued = true;

		if (PreviewHandler)
		{
			PreviewHandler->bIsInvalidated.Set(true);
		}
	}));

	DepthSlider =
		SNew(SVoxelGraphPreviewDepthSlider)
		.ToolTipText(INVTEXT("Depth along the axis being previewed"))
		.ValueText(INVTEXT("Depth"))
		.Value(NewTerminalGraph->PreviewConfig.GetAxisLocation())
		.MinValue(NewTerminalGraph->PreviewConfig.GetAxisLocation() - 100.f)
		.MaxValue(NewTerminalGraph->PreviewConfig.GetAxisLocation() + 100.f)
		.OnValueChanged_Lambda([=, this](const float NewValue)
		{
			UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
			if (!TerminalGraph)
			{
				return;
			}

			// No transaction for preview
			// const FVoxelTransaction Transaction(Graph, "Change depth");
			TerminalGraph->PreviewConfig.SetAxisLocation(NewValue);

			bUpdateQueued = true;
		});

	DepthSliderContainer->SetContent(DepthSlider.ToSharedRef());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> SVoxelGraphPreview::GetPreviewStats() const
{
	return ExternalPreviewStats.ToSharedRef();
}

void SVoxelGraphPreview::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Texture);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelGraphPreview::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	VOXEL_FUNCTION_COUNTER();

	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	const FMatrix PixelToWorld = GetPixelToWorld();
	const FTransform PixelToWorldTransform = FVoxelUtilities::MakeTransformSafe(PixelToWorld);

	if (!LocalToWorld.Equals(PixelToWorldTransform))
	{
		LocalToWorld = PixelToWorldTransform;
		UpdateStats();
	}

	if (bUpdateQueued)
	{
		bUpdateQueued = false;
		Update();
	}

	if (bMousePositionUpdateQueued)
	{
		bMousePositionUpdateQueued = false;
		ComputeMousePositionStats();
	}

	if (!FuturePreviewHandler.IsComplete())
	{
		if (ProcessingStartTime == 0)
		{
			ProcessingStartTime = FPlatformTime::Seconds();
		}
	}
	else
	{
		ProcessingStartTime = 0;
	}
}

FReply SVoxelGraphPreview::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		bLockCoordinatePending = true;
		return FReply::Handled();
	}

	const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);
	if (!bIsMiddleMouseButtonDown)
	{
		return FReply::Unhandled();
	}

	PreviewRuler->StartRuler(MouseEvent.GetScreenSpacePosition(), PreviewImage->GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));

	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

FReply SVoxelGraphPreview::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bLockCoordinatePending)
		{
			const FMatrix PixelToWorld = GetPixelToWorld();

			FVector2D MouseImagePosition = PreviewImage->GetCachedGeometry().AbsoluteToLocal(MousePosition);
			MouseImagePosition.Y = PreviewImage->GetCachedGeometry().Size.Y - MouseImagePosition.Y;

			if (bIsCoordinateLocked)
			{
				const FVector2D OldPixelPosition = FVector2D(PixelToWorld.InverseTransformPosition(LockedCoordinate_WorldSpace));
				if (FVector2D::Distance(OldPixelPosition, MouseImagePosition) < 20.f)
				{
					PreviewImage->ClearLockedPosition();

					bIsCoordinateLocked = false;
					return FReply::Handled();
				}
			}

			LockedCoordinate_WorldSpace = PixelToWorld.TransformPosition(FVector(MouseImagePosition.X, MouseImagePosition.Y, 0.f));
			bIsCoordinateLocked = true;

			PreviewImage->SetLockedPosition(LockedCoordinate_WorldSpace);

			if (!PreviewImage->UpdateLockedPosition(PixelToWorld, PreviewImage->GetCachedGeometry().Size))
			{
				bIsCoordinateLocked = false;
			}

			UpdateStats();
		}
		return FReply::Handled();
	}

	if (MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton))
	{
		return FReply::Unhandled();
	}

	PreviewRuler->StopRuler();

	return SCompoundWidget::OnMouseButtonUp(MyGeometry, MouseEvent);
}

FReply SVoxelGraphPreview::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const bool bIsMouseButtonDown =
		MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) ||
		MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
	const bool bIsMiddleMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::MiddleMouseButton);

	MousePosition = MouseEvent.GetScreenSpacePosition();

	if (!bIsMouseButtonDown &&
		!bIsMiddleMouseButtonDown)
	{
		return FReply::Unhandled();
	}

	if (bIsMiddleMouseButtonDown)
	{
		if (!MouseEvent.GetCursorDelta().IsNearlyZero())
		{
			PreviewRuler->UpdateRuler(MousePosition, PreviewImage->GetCachedGeometry().AbsoluteToLocal(MousePosition));
		}
		return FReply::Handled();
	}

	const FVector2D PixelDelta = TransformVector(Inverse(PreviewImage->GetCachedGeometry().GetAccumulatedRenderTransform()), MouseEvent.GetCursorDelta());
	const FVector WorldDelta = GetPixelToWorld().TransformVector(FVector(-PixelDelta.X, PixelDelta.Y, 0));

	if (WorldDelta.IsNearlyZero())
	{
		return FReply::Handled();
	}

	bLockCoordinatePending = false;

	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!TerminalGraph)
	{
		return FReply::Handled();
	}

	TerminalGraph->PreviewConfig.Position += WorldDelta;

	bUpdateQueued = true;

	return FReply::Handled();
}

FReply SVoxelGraphPreview::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const double Delta = MouseEvent.GetWheelDelta();
	if (FMath::IsNearlyZero(Delta))
	{
		return FReply::Handled();
	}

	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!TerminalGraph)
	{
		return FReply::Handled();
	}

	const double NewZoom = TerminalGraph->PreviewConfig.Zoom * (1. - FMath::Clamp(Delta, -0.5, 0.5));

	const FVector2D PixelDelta = TransformPoint(Inverse(PreviewImage->GetCachedGeometry().GetAccumulatedRenderTransform()), MouseEvent.GetScreenSpacePosition());
	const FVector OldPosition = GetPixelToWorld().TransformPosition(FVector(PixelDelta, 0));

	TerminalGraph->PreviewConfig.Zoom = FMath::Clamp(NewZoom, 1, 1.e8);

	const FVector NewPosition = GetPixelToWorld().TransformPosition(FVector(PixelDelta, 0));

	FVector PositionDelta = NewPosition - OldPosition;
	switch (TerminalGraph->PreviewConfig.Axis)
	{
	case EVoxelAxis::X: PositionDelta.Z *= -1.f; break;
	case EVoxelAxis::Y: PositionDelta.Z *= -1.f; break;
	case EVoxelAxis::Z: PositionDelta.Y *= -1.f; break;
	}

	TerminalGraph->PreviewConfig.Position -= PositionDelta;

	bUpdateQueued = true;

	return FReply::Handled();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DEFINE_PRIVATE_ACCESS(FToolMenuEntry, Action);

TSharedRef<SWidget> SVoxelGraphPreview::CreateToolbar(const TSharedPtr<FVoxelGraphToolkit>& Toolkit, const bool bNewToolbar)
{
#if VOXEL_ENGINE_VERSION >= 506
	if (bNewToolbar)
	{
		const FName ToolbarName = FName("GraphPreview.Toolbar");

		if (!UToolMenus::Get()->IsMenuRegistered(ToolbarName))
		{
			UToolMenu* ToolbarMenu = UToolMenus::Get()->RegisterMenu(ToolbarName, NAME_None, EMultiBoxType::SlimHorizontalToolBar);
			ToolbarMenu->StyleName = "ViewportToolbar";

			{
				ToolbarMenu->AddSection("Axis", INVTEXT("Axis"))
				.AddDynamicEntry("Axis", MakeLambdaDelegate([](FToolMenuSection& Section)
				{
					const UVoxelGraphPreviewContext* Context = Section.FindContext<UVoxelGraphPreviewContext>();
					if (!Context)
					{
						return;
					}

					const TSharedPtr<SVoxelGraphPreview> Preview = Context->WeakPreview.Pin();
					if (!Preview)
					{
						return;
					}

					FToolMenuEntry& EntryX = Section.AddEntry(Preview->MakeAxisEntry(EVoxelAxis::X));
					EntryX.SetShowInToolbarTopLevel(true);

					FToolMenuEntry& EntryY = Section.AddEntry(Preview->MakeAxisEntry(EVoxelAxis::Y));
					EntryY.SetShowInToolbarTopLevel(true);

					FToolMenuEntry& EntryZ = Section.AddEntry(Preview->MakeAxisEntry(EVoxelAxis::Z));
					EntryZ.SetShowInToolbarTopLevel(true);
				}));
			}

			{
				ToolbarMenu->AddSection("ResetView", INVTEXT("Reset View"))
				.AddDynamicEntry("ResetView", MakeLambdaDelegate([](FToolMenuSection& Section)
				{
					const UVoxelGraphPreviewContext* Context = Section.FindContext<UVoxelGraphPreviewContext>();
					if (!Context)
					{
						return;
					}

					const TSharedPtr<SVoxelGraphPreview> Preview = Context->WeakPreview.Pin();
					if (!Preview)
					{
						return;
					}

					FToolMenuEntry& Entry = Section.AddEntry(Preview->MakeResetViewEntry());
					Entry.SetShowInToolbarTopLevel(true);
				}));
			}

			{
				ToolbarMenu->AddSection("Settings", INVTEXT("Settings"))
				.AddDynamicEntry("Settings", MakeLambdaDelegate([](FToolMenuSection& Section)
				{
					const UVoxelGraphPreviewContext* Context = Section.FindContext<UVoxelGraphPreviewContext>();
					if (!Context)
					{
						return;
					}

					const TSharedPtr<SVoxelGraphPreview> Preview = Context->WeakPreview.Pin();
					if (!Preview)
					{
						return;
					}

					FToolMenuEntry& PropertiesEntry = Section.AddEntry(Preview->MakePreviewPropertiesEntry(Context->WeakToolkit.Pin()));
					PropertiesEntry.SetShowInToolbarTopLevel(true);

					FToolMenuEntry& NormalizeEntry = Section.AddEntry(Preview->MakeNormalizeEntry());
					NormalizeEntry.SetShowInToolbarTopLevel(true);

					FToolMenuEntry& GrayscaleEntry = Section.AddEntry(Preview->MakeGrayscaleEntry());
					GrayscaleEntry.SetShowInToolbarTopLevel(true);
				}));
			}

			{
				FToolMenuSection& ResolutionSection = ToolbarMenu->AddSection("Resolution", INVTEXT("Resolution"));
				ResolutionSection.Alignment = EToolMenuSectionAlign::Last;
				ResolutionSection.AddDynamicEntry("Resolution", MakeLambdaDelegate([](FToolMenuSection& Section)
				{
					const UVoxelGraphPreviewContext* Context = Section.FindContext<UVoxelGraphPreviewContext>();
					if (!Context)
					{
						return;
					}

					const TSharedPtr<SVoxelGraphPreview> Preview = Context->WeakPreview.Pin();
					if (!Preview)
					{
						return;
					}

					Section.AddEntry(Preview->MakeResolutionEntry());
				}));
			}
		}

		UVoxelGraphPreviewContext* Context = NewObject<UVoxelGraphPreviewContext>();
		Context->WeakToolkit = Toolkit;
		Context->WeakPreview = SharedThis(this);
		return UToolMenus::Get()->GenerateWidget(ToolbarName, Context);
	}
#endif

	FSlimHorizontalToolBarBuilder LeftToolbarBuilder(nullptr, FMultiBoxCustomization::None);
	LeftToolbarBuilder.SetStyle(&FAppStyle::Get(), "EditorViewportToolBar");
	LeftToolbarBuilder.SetLabelVisibility(EVisibility::Collapsed);

	const auto AddToolbarButton = [&](const FToolMenuEntry& Entry)
	{
		LeftToolbarBuilder.AddToolBarButton(
			*PrivateAccess::Action(Entry).GetUIAction(),
			Entry.Name,
			Entry.Label,
			Entry.ToolTip,
			Entry.Icon,
			EUserInterfaceActionType::ToggleButton);
	};
	const auto AddSubmenu = [&](const FToolMenuEntry& Entry, const bool bLast)
	{
		FOnGetContent OnGetContent = MakeLambdaDelegate([Delegate = Entry.SubMenuData.ConstructMenu]() -> TSharedRef<SWidget>
		{
			if (ensure(Delegate.NewToolMenuWidget.IsBound()))
			{
				const FToolMenuContext Context;
				return Delegate.NewToolMenuWidget.Execute(Context);
			}
			return SNullWidget::NullWidget;
		});

		FUIAction Action;
		if (const FUIAction* EntryAction = PrivateAccess::Action(Entry).GetUIAction())
		{
			Action = *EntryAction;
		}

		LeftToolbarBuilder.AddWidget(
			SNew(SComboButton)
			.ButtonStyle(FAppStyle::Get(), bLast ? "EditorViewportToolBar.Button" : "EditorViewportToolBar.Button.Start")
			.ComboButtonStyle(FAppStyle::Get(), "ViewportPinnedCommandList.ComboButton")
			.ContentPadding(FMargin(4.f, 0.f))
			.ToolTipText(Entry.ToolTip)
			.IsEnabled_Lambda([Delegate = Action.CanExecuteAction]
			{
				if (Delegate.IsBound())
				{
					return Delegate.Execute();
				}
				return true;
			})
			.OnGetMenuContent(OnGetContent)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Clipping(EWidgetClipping::ClipToBounds)
			.ButtonContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SImage)
					.Image_Lambda([GetIcon = Entry.Icon]
					{
						if (GetIcon.IsSet())
						{
							return GetIcon.Get().GetSmallIcon();
						}
						return FAppStyle::GetNoBrush();
					})
					.DesiredSizeOverride(FVector2D(16.f, 16.f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.f, 0.f)
				.VAlign(VAlign_Center)
				[
					SNew(SVoxelDetailText)
					.Text(Entry.ToolBarData.LabelOverride)
				]
			],
			{},
			false,
			bLast ? HAlign_Right : HAlign_Fill,
			MakeLambdaDelegate([this, OnGetContent, Entry](FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddWrapperSubMenu(
					Entry.Label.Get(),
					Entry.ToolTip.Get(),
					OnGetContent,
					Entry.Icon.Get());
			}));
	};

	LeftToolbarBuilder.BeginSection("Preview");
	{
		LeftToolbarBuilder.BeginBlockGroup();
		AddToolbarButton(MakeAxisEntry(EVoxelAxis::X));
		AddToolbarButton(MakeAxisEntry(EVoxelAxis::Y));
		AddToolbarButton(MakeAxisEntry(EVoxelAxis::Z));
		LeftToolbarBuilder.EndBlockGroup();
	}

	LeftToolbarBuilder.AddSeparator();

	AddToolbarButton(MakeResetViewEntry());

	LeftToolbarBuilder.AddSeparator();

	{
		LeftToolbarBuilder.BeginBlockGroup();
		AddSubmenu(MakePreviewPropertiesEntry(Toolkit), false);
		AddToolbarButton(MakeNormalizeEntry());
		AddToolbarButton(MakeGrayscaleEntry());
		LeftToolbarBuilder.EndBlockGroup();
	}

	LeftToolbarBuilder.EndSection();

	LeftToolbarBuilder.AddSeparator();

	AddSubmenu(MakeResolutionEntry(), true);

	return LeftToolbarBuilder.MakeWidget();
}

FToolMenuEntry SVoxelGraphPreview::MakeAxisEntry(EVoxelAxis Axis)
{
	const UEnum* Enum = StaticEnum<EVoxelAxis>();
	const int32 Index = Enum->GetIndexByValue(int32(Axis));
	FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
		Enum->GetNameByIndex(Index),
		Enum->GetDisplayNameTextByIndex(Index),
		Enum->GetToolTipTextByIndex(Index),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), *Enum->GetMetaData(TEXT("Icon"), Index)),
		FUIAction(
			MakeLambdaDelegate([this, Axis]
			{
				UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
				if (!TerminalGraph)
				{
					return;
				}

				// No transaction for preview
				// const FVoxelTransaction Transaction(Graph, "Set preview axis");
				TerminalGraph->PreviewConfig.Axis = Axis;

				if (DepthSlider)
				{
					DepthSlider->ResetValue(TerminalGraph->PreviewConfig.GetAxisLocation(), false);
				}

				bUpdateQueued = true;
			}),
			MakeLambdaDelegate([]
			{
				return true;
			}),
			MakeLambdaDelegate([this, Axis]
			{
				const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
				if (!TerminalGraph)
				{
					return ECheckBoxState::Unchecked;
				}

				return TerminalGraph->PreviewConfig.Axis == Axis ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})),
			EUserInterfaceActionType::RadioButton);

#if VOXEL_ENGINE_VERSION >= 506
	Entry.ToolBarData.BlockGroupName = "Axis";
	Entry.ToolBarData.ResizeParams.ClippingPriority = 2000;
#endif
	Entry.ToolBarData.LabelOverride = FText();

	return Entry;
}

FToolMenuEntry SVoxelGraphPreview::MakeResetViewEntry()
{
	FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
		"ResetView",
		INVTEXT("Reset view"),
		INVTEXT("Reset view"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Undo"),
		FUIAction(
		MakeLambdaDelegate([this]
		{
			UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
			if (!TerminalGraph)
			{
				return;
			}

			// No transaction for preview
			// const FVoxelTransaction Transaction(Graph, "Reset view");

			TerminalGraph->PreviewConfig.Position = FVector::ZeroVector;
			TerminalGraph->PreviewConfig.Zoom = 1.f;

			if (DepthSlider)
			{
				DepthSlider->ResetValue(TerminalGraph->PreviewConfig.GetAxisLocation(), false);
			}

			bUpdateQueued = true;
		}),
		MakeLambdaDelegate([this]
		{
			const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
			if (!TerminalGraph)
			{
				return false;
			}

			return
				!TerminalGraph->PreviewConfig.Position.IsZero() ||
				TerminalGraph->PreviewConfig.Zoom != 1.f;
		})),
		EUserInterfaceActionType::Button
	);

#if VOXEL_ENGINE_VERSION >= 506
	Entry.ToolBarData.BlockGroupName = "Reset View";
	Entry.ToolBarData.ResizeParams.ClippingPriority = 1000;
#endif
	Entry.ToolBarData.LabelOverride = FText();

	return Entry;
}

FToolMenuEntry SVoxelGraphPreview::MakePreviewPropertiesEntry(const TSharedPtr<FVoxelGraphToolkit>& Toolkit)
{
	FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
		"Settings",
		INVTEXT("Settings"),
		INVTEXT("Edit Preview Settings"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"),
		FUIAction(
			{},
			MakeLambdaDelegate([this]() -> bool
			{
				UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
				if (!TerminalGraph)
				{
					return false;
				}

				const UVoxelEdGraph& EdGraph = TerminalGraph->GetTypedEdGraph<UVoxelEdGraph>();
				return EdGraph.WeakDebugNode.IsValid();
			})));

	Entry.SubMenuData.bIsSubMenu = true;
	Entry.SubMenuData.ConstructMenu =
		MakeLambdaDelegate([this, WeakToolkit = MakeWeakPtr(Toolkit)](const FToolMenuContext&) -> TSharedRef<SWidget>
		{
			TSharedPtr<FVoxelGraphToolkit> PinnedToolkit = WeakToolkit.Pin();
			if (!PinnedToolkit)
			{
				return SNullWidget::NullWidget;
			}

			UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
			if (!TerminalGraph)
			{
				return SNullWidget::NullWidget;
			}

			const UVoxelEdGraph& EdGraph = TerminalGraph->GetTypedEdGraph<UVoxelEdGraph>();
			UEdGraphNode* DebugNode = EdGraph.WeakDebugNode.Get();
			if (!DebugNode)
			{
				return SNullWidget::NullWidget;
			}

			FDetailsViewArgs Args;
			Args.bAllowSearch = false;
			Args.bShowOptions = false;
			Args.bHideSelectionTip = true;
			Args.bShowPropertyMatrixButton = false;
			Args.NotifyHook = PinnedToolkit->GetNotifyHook();
			Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

			FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
			TSharedRef<IDetailsView> PreviewDetailsView = PropertyModule.CreateDetailView(Args);
			PreviewDetailsView->SetGenericLayoutDetailsDelegate(MakeLambdaDelegate([]() -> TSharedRef<IDetailCustomization>
			{
				return MakeShared<FVoxelGraphPreviewSettingsCustomization>();
			}));
			PreviewDetailsView->SetObject(DebugNode);

			return PreviewDetailsView;
		});
	Entry.SubMenuData.bOpenSubMenuOnClick = false;

#if VOXEL_ENGINE_VERSION >= 506
	Entry.ToolBarData.BlockGroupName = "Settings";
	Entry.ToolBarData.ResizeParams.ClippingPriority = 500;
#endif
	Entry.ToolBarData.LabelOverride = FText();

	return Entry;
}

FToolMenuEntry SVoxelGraphPreview::MakeNormalizeEntry()
{
	FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
		"Normalize",
		INVTEXT("Normalize"),
		INVTEXT("Normalize"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.DistributeNodesHorizontally"),
		FUIAction(
			MakeLambdaDelegate([this]
			{
				UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
				if (!TerminalGraph)
				{
					return;
				}

				const UVoxelEdGraph& EdGraph = TerminalGraph->GetTypedEdGraph<UVoxelEdGraph>();
				UVoxelGraphNode* DebugNode = Cast<UVoxelGraphNode>(EdGraph.WeakDebugNode.Get());
				if (!DebugNode)
				{
					return;
				}

				for (FVoxelPinPreviewSettings& PinPreviewSettings : DebugNode->PreviewSettings)
				{
					if (PinPreviewSettings.PinName == DebugNode->PreviewedPin)
					{
						if (FVoxelScalarPreviewHandler* ScalarPreviewHandler = PinPreviewSettings.PreviewHandler.GetPtr<FVoxelScalarPreviewHandler>())
						{
							FVoxelTransaction Transaction(DebugNode, "Set Normalize");
							ScalarPreviewHandler->bNormalize = !ScalarPreviewHandler->bNormalize;
						}
						return;
					}
				}
			}),
			MakeLambdaDelegate([this]
			{
				UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
				if (!TerminalGraph ||
					!PreviewHandler)
				{
					return false;
				}

				const UVoxelEdGraph& EdGraph = TerminalGraph->GetTypedEdGraph<UVoxelEdGraph>();
				if (!EdGraph.WeakDebugNode.IsValid())
				{
					return false;
				}

				return PreviewHandler->IsA<FVoxelScalarPreviewHandler>();
			}),
			MakeLambdaDelegate([this]
			{
				UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
				if (!TerminalGraph ||
					!PreviewHandler ||
					!PreviewHandler->IsA<FVoxelScalarPreviewHandler>())
				{
					return false;
				}

				return PreviewHandler->As<FVoxelScalarPreviewHandler>()->bNormalize;
			})),
			EUserInterfaceActionType::ToggleButton);

#if VOXEL_ENGINE_VERSION >= 506
	Entry.ToolBarData.BlockGroupName = "Settings";
	Entry.ToolBarData.ResizeParams.ClippingPriority = 500;
#endif
	Entry.ToolBarData.LabelOverride = FText();

	return Entry;
}

FToolMenuEntry SVoxelGraphPreview::MakeGrayscaleEntry()
{
	const auto GetTypeString = [this]() -> FString
	{
		const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
		if (!TerminalGraph ||
			!PreviewHandler)
		{
			return "None";
		}

		return PreviewHandler->IsA<FVoxelGrayscalePreviewHandler>() ? "Grayscale" : "Distance Field";
	};

	FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
		"GrayscaleDistanceField",
		MakeAttributeLambda([GetTypeString]
		{
			return FText::FromString("Scalar type " + GetTypeString());
		}),
		MakeAttributeLambda([GetTypeString]
		{
			return FText::FromString("Scalar preview representation type "+ GetTypeString());
		}),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "SpinBox.Arrows"),
		FUIAction(
			MakeLambdaDelegate([this]
			{
				UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
				if (!TerminalGraph)
				{
					return;
				}

				const UVoxelEdGraph& EdGraph = TerminalGraph->GetTypedEdGraph<UVoxelEdGraph>();
				UVoxelGraphNode* DebugNode = Cast<UVoxelGraphNode>(EdGraph.WeakDebugNode.Get());
				if (!DebugNode)
				{
					return;
				}

				for (FVoxelPinPreviewSettings& PinPreviewSettings : DebugNode->PreviewSettings)
				{
					if (PinPreviewSettings.PinName == DebugNode->PreviewedPin)
					{
						FVoxelTransaction Transaction(DebugNode, "Switch Grayscale Distance Field");
						if (PinPreviewSettings.PreviewHandler.IsA<FVoxelPreviewHandler_Grayscale_Float>())
						{
							PinPreviewSettings.PreviewHandler = FVoxelInstancedStruct(FVoxelPreviewHandler_DistanceField_Float::StaticStruct());
						}
						else if (PinPreviewSettings.PreviewHandler.IsA<FVoxelPreviewHandler_DistanceField_Float>())
						{
							PinPreviewSettings.PreviewHandler = FVoxelInstancedStruct(FVoxelPreviewHandler_Grayscale_Float::StaticStruct());
						}
						else if (PinPreviewSettings.PreviewHandler.IsA<FVoxelPreviewHandler_Grayscale_Double>())
						{
							PinPreviewSettings.PreviewHandler = FVoxelInstancedStruct(FVoxelPreviewHandler_DistanceField_Double::StaticStruct());
						}
						else if (PinPreviewSettings.PreviewHandler.IsA<FVoxelPreviewHandler_DistanceField_Double>())
						{
							PinPreviewSettings.PreviewHandler = FVoxelInstancedStruct(FVoxelPreviewHandler_Grayscale_Double::StaticStruct());
						}
						else
						{
							ensure(false);
						}
						return;
					}
				}
			}),
			MakeLambdaDelegate([this]
			{
				UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
				if (!TerminalGraph ||
					!PreviewHandler)
				{
					return false;
				}

				const UVoxelEdGraph& EdGraph = TerminalGraph->GetTypedEdGraph<UVoxelEdGraph>();
				if (!EdGraph.WeakDebugNode.IsValid())
				{
					return false;
				}

				return
					PreviewHandler->IsA<FVoxelGrayscalePreviewHandler>() ||
					PreviewHandler->IsA<FVoxelPreviewHandler_DistanceField_Base>();
			}),
			MakeLambdaDelegate([this]
			{
				const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
				if (!TerminalGraph ||
					!PreviewHandler)
				{
					return false;
				}

				return PreviewHandler->IsA<FVoxelGrayscalePreviewHandler>();
			})),
			EUserInterfaceActionType::ToggleButton);

#if VOXEL_ENGINE_VERSION >= 506
	Entry.ToolBarData.BlockGroupName = "Settings";
	Entry.ToolBarData.ResizeParams.ClippingPriority = 500;
#endif
	Entry.ToolBarData.LabelOverride = FText();

	return Entry;
}

FToolMenuEntry SVoxelGraphPreview::MakeResolutionEntry()
{
	FToolMenuEntry Entry = FToolMenuEntry::InitSubMenu(
		"Resolution",
		MakeAttributeLambda([this]
		{
			const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
			if (!TerminalGraph)
			{
				return INVTEXT("Resolution 512x512");
			}

			return FText::FromString("Resolution " + FString::Printf(TEXT("%dx%d"), TerminalGraph->PreviewConfig.Resolution, TerminalGraph->PreviewConfig.Resolution));
		}),
		INVTEXT("Texture resolution for preview. The higher the resolution, the longer it will take to compute preview"),
		MakeLambdaDelegate([this](const FToolMenuContext&)
		{
			FMenuBuilder MenuBuilder(true, nullptr);

			MenuBuilder.BeginSection({}, INVTEXT("Resolutions"));

			static const TArray<int32> PreviewSizes =
			{
				256,
				512,
				1024,
				2048,
				4096
			};

			for (const int32 NewPreviewSize : PreviewSizes)
			{
				const FString Text = FString::Printf(TEXT("%dx%d"), NewPreviewSize, NewPreviewSize);

				MenuBuilder.AddMenuEntry(
					FText::FromString(Text),
					FText::FromString(Text),
					FSlateIcon(),
					FUIAction(
						MakeLambdaDelegate([=, this]
						{
							UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
							if (!TerminalGraph)
							{
								return;
							}

							const FVoxelTransaction Transaction(TerminalGraph, "Change preview size");
							TerminalGraph->PreviewConfig.Resolution = NewPreviewSize;

							bUpdateQueued = true;
						}),
						{},
						MakeLambdaDelegate([this, NewPreviewSize]
						{
							const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
							if (!TerminalGraph)
							{
								return false;
							}

							return TerminalGraph->PreviewConfig.Resolution == NewPreviewSize;
						})
					),
					FName(Text),
					EUserInterfaceActionType::ToggleButton);
			}

			MenuBuilder.EndSection();

			return MenuBuilder.MakeWidget();
		}),
		false,
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "CurveEditorTools.ActivateTransformTool"));

	Entry.ToolBarData.LabelOverride = MakeAttributeLambda([this]
	{
		const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
		if (!TerminalGraph)
		{
			return INVTEXT("512x512");
		}

		return FText::FromString(FString::Printf(TEXT("%dx%d"), TerminalGraph->PreviewConfig.Resolution, TerminalGraph->PreviewConfig.Resolution));
	});

	return Entry;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelGraphPreview::Update()
{
	VOXEL_FUNCTION_COUNTER();

	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!TerminalGraph)
	{
		Message = {};
		return;
	}

	const int32 PreviewSize = TerminalGraph->PreviewConfig.Resolution;
	if (!Texture ||
		Texture->GetSizeX() != PreviewSize ||
		Texture->GetSizeY() != PreviewSize)
	{
		Texture = FVoxelTextureUtilities::CreateTexture2D(
			"Preview",
			PreviewSize,
			PreviewSize,
			false,
			TF_Bilinear,
			PF_B8G8R8A8);

		Brush = FDeferredCleanupSlateBrush::CreateBrush(
			Texture,
			FVector2D(PreviewSize, PreviewSize));

		PreviewHandler.Reset();
		FuturePreviewHandler = {};
	}

	const FVoxelSerializedGraph& SerializedGraph = TerminalGraph->GetRuntime().GetSerializedGraph();

	if (!SerializedGraph.PreviewHandler.IsValid())
	{
		if (PreviewHandler.IsValid())
		{
			const TSharedRef<TVoxelArray<uint8>> Bytes = MakeShared<TVoxelArray<uint8>>();
			FVoxelUtilities::SetNumZeroed(*Bytes, PreviewSize * PreviewSize * sizeof(FColor));
			FVoxelUtilities::AsyncCopyTexture(Texture, Bytes);
		}

		PreviewHandler.Reset();

		Message = "Press D to preview the selected node";
		return;
	}

	Message = {};

	const TSharedRef<FVoxelPreviewHandler> NewPreviewHandler = SerializedGraph.PreviewHandler.Get<FVoxelPreviewHandler>().MakeSharedCopy();

	const bool bIsInvalidated = INLINE_LAMBDA
	{
		if (!PreviewHandler)
		{
			return true;
		}

		if (PreviewHandler->PreviewSize != TerminalGraph->PreviewConfig.Resolution ||
			!PreviewHandler->LocalToWorld.Equals(LocalToWorld) ||
			PreviewHandler->bIsInvalidated.Get() ||
			!PreviewHandler->Equals_UPropertyOnly(*NewPreviewHandler))
		{
			return true;
		}

		return false;
	};

	if (!bIsInvalidated)
	{
		return;
	}

	if (!FuturePreviewHandler.IsComplete())
	{
		bUpdateQueued = true;
		return;
	}

	NewPreviewHandler->PreviewSize = TerminalGraph->PreviewConfig.Resolution;
	NewPreviewHandler->LocalToWorld = LocalToWorld;
	NewPreviewHandler->OnInvalidated = MakeWeakPtrLambda(this, MakeWeakPtrLambda(NewPreviewHandler, [this, &LocalPreviewHandler = *NewPreviewHandler]
	{
		bUpdateQueued = true;
		LocalPreviewHandler.bIsInvalidated.Set(true);
	}));

	ensure(FuturePreviewHandler.IsComplete());
	FuturePreviewHandler =
		NewPreviewHandler->Compute(*TerminalGraph)
		.Then_GameThread(MakeWeakPtrLambda(this, [this, NewPreviewHandler](const TSharedRef<TVoxelArray<uint8>>& Bytes)
		{
			if (Bytes->Num() > 0)
			{
				FVoxelUtilities::AsyncCopyTexture(Texture, Bytes);
			}

			PreviewHandler = NewPreviewHandler;

			PreviewStats->Reset();
			ExternalPreviewStats->Reset();
			PreviewHandler->BuildStats([&](
				const FString& Name,
				const bool bGlobalSpacing,
				const TFunction<FString()>& GetValueTooltip,
				const TFunction<TArray<FString>()>& GetValues)
				{
					const TSharedRef<SVoxelGraphPreviewStats::FRow> StatsRow = MakeShared<SVoxelGraphPreviewStats::FRow>();
					StatsRow->bGlobalSpacing = bGlobalSpacing;
					StatsRow->Header = FText::FromString(Name);
					StatsRow->Tooltip = MakeAttributeLambda([=]
					{
						return FText::FromString(GetValueTooltip());
					});
					StatsRow->Values = MakeAttributeLambda([=]
					{
						return GetValues();
					});
					PreviewStats->Rows.Add(StatsRow);
					ExternalPreviewStats->Rows.Add(StatsRow);
				});

			PreviewStats->RowsView->RequestListRefresh();
			ExternalPreviewStats->RowsView->RequestListRefresh();

			UpdateStats();
		}));
}

void SVoxelGraphPreview::UpdateStats()
{
	VOXEL_FUNCTION_COUNTER();

	if (!PreviewHandler)
	{
		return;
	}

	const FMatrix PixelToWorld = GetPixelToWorld();

	if (!PreviewImage->UpdateLockedPosition(PixelToWorld, PreviewImage->GetCachedGeometry().Size))
	{
		bIsCoordinateLocked = false;
	}

	FVector2D LocalMousePosition;
	if (bIsCoordinateLocked)
	{
		LocalMousePosition = FVector2D(PixelToWorld.InverseTransformPosition(LockedCoordinate_WorldSpace));
	}
	else
	{
		LocalMousePosition = PreviewImage->GetCachedGeometry().AbsoluteToLocal(MousePosition);
		LocalMousePosition.Y = PreviewImage->GetCachedGeometry().Size.Y - LocalMousePosition.Y;
	}

	PreviewHandler->SetMousePosition(LocalMousePosition);

	ComputeMousePositionStats();
}

void SVoxelGraphPreview::ComputeMousePositionStats()
{
	UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!TerminalGraph)
	{
		return;
	}

	if (!FutureMousePositionComputeHandler.IsComplete())
	{
		bMousePositionUpdateQueued = true;
		return;
	}

	TerminalGraph->GetGraph().bEnableNodeValueStats = bIsCoordinateLocked;

	if (bIsCoordinateLocked)
	{
		FutureMousePositionComputeHandler = PreviewHandler->ComputeAtMousePosition(*TerminalGraph);
	}
}

FMatrix SVoxelGraphPreview::GetPixelToWorld() const
{
	const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Resolve();
	if (!TerminalGraph)
	{
		return FMatrix::Identity;
	}

	const FMatrix Matrix = INLINE_LAMBDA -> FMatrix
	{
		const FVector X = FVector::UnitX();
		const FVector Y = FVector::UnitY();
		const FVector Z = FVector::UnitZ();

		switch (TerminalGraph->PreviewConfig.Axis)
		{
		default: ensure(false);
		case EVoxelAxis::X: return FMatrix(Y, Z, X, FVector::ZeroVector);
		case EVoxelAxis::Y: return FMatrix(X, Z, -Y, FVector::ZeroVector);
		case EVoxelAxis::Z: return FMatrix(X, Y, Z, FVector::ZeroVector);
		}
	};

	return
		FScaleMatrix(1. / TerminalGraph->PreviewConfig.Resolution) *
		FScaleMatrix(2.) *
		FTranslationMatrix(FVector(-1.f, -1.f, 0.f)) *
		FScaleMatrix(FVector(TerminalGraph->PreviewConfig.Zoom, TerminalGraph->PreviewConfig.Zoom, 1.f)) *
		FRotationMatrix(Matrix.Rotator()) *
		FTranslationMatrix(TerminalGraph->PreviewConfig.Position);
}