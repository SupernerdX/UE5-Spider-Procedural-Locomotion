// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphEditorSummoner.h"
#include "VoxelGraphToolkit.h"
#include "VoxelPluginVersion.h"
#include "VoxelTerminalGraph.h"
#include "GraphEditorDragDropAction.h"
#include "Widgets/SVoxelGraphPreview.h"
#include "Members/SVoxelGraphMembers.h"
#include "Selection/VoxelGraphSelection.h"

FVoxelGraphEditorSummoner::FVoxelGraphEditorSummoner(const TSharedRef<FVoxelGraphToolkit>& Toolkit)
	: FDocumentTabFactoryForObjects<UEdGraph>(STATIC_FNAME("FVoxelGraphEditorSummoner"), nullptr)
	, WeakToolkit(Toolkit)
{
}

void FVoxelGraphEditorSummoner::OnTabActivated(const TSharedPtr<SDockTab> Tab) const
{
	Tab->SetOnTabClosed(MakeWeakPtrDelegate(this, [this](const TSharedRef<SDockTab> InTab)
	{
		const TSharedPtr<FVoxelGraphToolkit> Toolkit = WeakToolkit.Pin();
		if (!ensure(Toolkit))
		{
			return;
		}

		if (Toolkit->WeakFocusedGraph == GetGraphEditor(InTab))
		{
			FocusGraphEditor(nullptr);
		}
	}));

	FocusGraphEditor(GetGraphEditor(Tab));
}

void FVoxelGraphEditorSummoner::SaveState(
	const TSharedPtr<SDockTab> Tab,
	const TSharedPtr<FTabPayload> Payload) const
{
	if (!ensure(Payload->IsValid()))
	{
		return;
	}

	UEdGraph* Graph = FTabPayload_UObject::CastChecked<UEdGraph>(Payload);
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = WeakToolkit.Pin();

	if (!ensure(Graph) ||
		!ensure(Toolkit))
	{
		return;
	}

	UE_506_SWITCH(FVector2D, FVector2f) ViewLocation = UE_506_SWITCH(FVector2D, FVector2f)::ZeroVector;
	float ZoomAmount = 0.f;
	if (const TSharedPtr<SGraphEditor> GraphEditor = GetGraphEditor(Tab))
	{
		GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);
	}

	FVoxelEditedDocumentInfo DocumentInfo;
	DocumentInfo.EdGraph = Graph;
	DocumentInfo.ViewLocation = FVector2D(ViewLocation);
	DocumentInfo.ZoomAmount	 = ZoomAmount;
	Toolkit->Asset->LastEditedDocuments.Add(DocumentInfo);
}

TAttribute<FText> FVoxelGraphEditorSummoner::ConstructTabNameForObject(UEdGraph* EdGraph) const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = WeakToolkit.Pin();
	if (!ensure(Toolkit))
	{
		return {};
	}

	return TAttribute<FText>::CreateLambda([=]() -> FText
	{
		if (!ensure(EdGraph))
		{
			return {};
		}

		const UVoxelTerminalGraph* TerminalGraph = EdGraph->GetTypedOuter<UVoxelTerminalGraph>();
		if (!TerminalGraph)
		{
			if (EdGraph->HasAnyFlags(RF_Transient))
			{
				return INVTEXT("Intermediate Graph");
			}

			ensure(false);
			return {};
		}

		if (TerminalGraph->IsMainTerminalGraph())
		{
			return INVTEXT("Main Graph");
		}

		if (TerminalGraph->IsEditorTerminalGraph())
		{
			return INVTEXT("Editor Graph");
		}

		return FText::FromString(TerminalGraph->GetDisplayName());
	});
}

TSharedRef<SWidget> FVoxelGraphEditorSummoner::CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* EdGraph) const
{
	if (!ensure(EdGraph))
	{
		return SNullWidget::NullWidget;
	}

	const TSharedPtr<FVoxelGraphToolkit> Toolkit = WeakToolkit.Pin();
	check(Toolkit);

	const TSharedRef<SVerticalBox> OverlayBox =
		SNew(SVerticalBox)
		.Visibility(EVisibility::HitTestInvisible)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.f, 0.f, 0.f, -10.f)
		[
			SNew(STextBlock)
			.TextStyle(FAppStyle::Get(), "Graph.CornerText")
			.Text(INVTEXT("VOXEL"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(FAppStyle::Get(), "Graph.CornerText")
			.Font(FCoreStyle::GetDefaultFontStyle("BoldCondensed", 16))
			.Text(FText::FromString(FVoxelUtilities::GetPluginVersion().ToString_UserFacing()))
		];

	if (EdGraph->HasAllFlags(RF_Transient))
	{
		OverlayBox->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("BoldCondensed", 12))
			.ColorAndOpacity(FStyleColors::AccentBlue)
			.RenderOpacity(0.2f)
			.Text_Lambda([Toolkit]
			{
				return FText::FromString(Toolkit->IntermediateGraphName);
			})
		];
		OverlayBox->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("BoldCondensed", 12))
			.ColorAndOpacity(FStyleColors::Warning)
			.RenderOpacity(0.2f)
			.Text(INVTEXT("Intermediate graphs do not auto refresh.\nAfter doing changes, try opening it again."))
		];
	}

	return
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			Toolkit->CreateGraphEditor(*EdGraph, true)
		]
		+ SOverlay::Slot()
		.Padding(10)
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		[
			OverlayBox
		];
}

const FSlateBrush* FVoxelGraphEditorSummoner::GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* EdGraph) const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = WeakToolkit.Pin();
	if (!ensure(EdGraph) ||
		!ensure(Toolkit))
	{
		return nullptr;
	}

	const UVoxelTerminalGraph* TerminalGraph = EdGraph->GetTypedOuter<UVoxelTerminalGraph>();
	if (!TerminalGraph)
	{
		if (EdGraph->HasAnyFlags(RF_Transient))
		{
			return FAppStyle::GetBrush("BlueprintDebugger.TabIcon");
		}

		return nullptr;
	}

	if (TerminalGraph->IsMainTerminalGraph())
	{
		return FVoxelEditorStyle::GetBrush("VoxelGraph.Execute");
	}

	if (TerminalGraph->IsEditorTerminalGraph())
	{
		return FAppStyle::GetBrush("ClassIcon.EditorUtilityBlueprint");
	}

	return FAppStyle::GetBrush("GraphEditor.Function_16x");
}

TSharedRef<FGenericTabHistory> FVoxelGraphEditorSummoner::CreateTabHistoryNode(TSharedPtr<FTabPayload> Payload)
{
	return MakeShared<FVoxelGraphTabHistory>(SharedThis(this), Payload);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<SGraphEditor> FVoxelGraphEditorSummoner::GetGraphEditor(const TSharedPtr<SDockTab>& Tab)
{
	if (Tab->GetContent()->GetType() != STATIC_FNAME("SOverlay"))
	{
		return nullptr;
	}

	SOverlay& Overlay = static_cast<SOverlay&>(*Tab->GetContent());
	const TSharedRef<SWidget> OverlayChild = Overlay.GetChildren()->GetChildAt(0);
	if (OverlayChild->GetType() != STATIC_FNAME("SGraphEditor"))
	{
		return nullptr;
	}

	return StaticCastSharedPtr<SGraphEditor>(OverlayChild.ToSharedPtr());
}

void FVoxelGraphEditorSummoner::FocusGraphEditor(const TSharedPtr<SGraphEditor>& GraphEditor) const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = WeakToolkit.Pin();
	if (!ensure(Toolkit))
	{
		return;
	}

	if (Toolkit->WeakFocusedGraph == GraphEditor)
	{
		return;
	}

	Toolkit->WeakFocusedGraph = GraphEditor;

	if (!GraphEditor)
	{
		Toolkit->GraphPreview->SetTerminalGraph(nullptr);
		Toolkit->GraphMembers->SetTerminalGraph(nullptr);
		return;
	}

	if (Toolkit->GraphBeingClosed == GraphEditor->GetCurrentGraph())
	{
		return;
	}

	const UEdGraph* EdGraph = GraphEditor->GetCurrentGraph();
	if (!ensure(EdGraph))
	{
		return;
	}

	UVoxelTerminalGraph* TerminalGraph = EdGraph->GetTypedOuter<UVoxelTerminalGraph>();
	if (!TerminalGraph)
	{
		return;
	}

	Toolkit->GraphPreview->SetTerminalGraph(TerminalGraph);
	Toolkit->GraphMembers->SetTerminalGraph(TerminalGraph);
	Toolkit->GetSelection().Update();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelGraphReadOnlySummoner::FVoxelGraphReadOnlySummoner(const TSharedRef<FVoxelGraphToolkit>& Toolkit)
	: FDocumentTabFactoryForObjects<UVoxelGraph>(STATIC_FNAME("FVoxelGraphReadOnlySummoner"), nullptr)
	, WeakToolkit(Toolkit)
{
}

void FVoxelGraphReadOnlySummoner::OnTabActivated(TSharedPtr<SDockTab> Tab) const
{
	FocusReadOnlyPreview();
}

void FVoxelGraphReadOnlySummoner::SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = WeakToolkit.Pin();
	if (!ensure(Toolkit))
	{
		return;
	}

	FVoxelEditedDocumentInfo DocumentInfo;
	DocumentInfo.EdGraph = nullptr;
	Toolkit->Asset->LastEditedDocuments.Add(DocumentInfo);
}

TAttribute<FText> FVoxelGraphReadOnlySummoner::ConstructTabNameForObject(UVoxelGraph* VoxelGraph) const
{
	if (!ensure(VoxelGraph))
	{
		return {};
	}

	if (VoxelGraph->IsFunctionLibrary())
	{
		return INVTEXT("Empty");
	}

	return INVTEXT("Main Graph (Read Only)");
}

class SVoxelReadOnlyGraph : public SNodePanel
{
public:
	VOXEL_SLATE_ARGS()
	{
	};

	void Construct(const FArguments& Args)
	{
		SNodePanel::Construct();
	}

	//~ Begin SNodePanel Interface
	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override
	{
		int32 MaxLayerId = LayerId + 1;
		const FSlateBrush* DefaultBackground = FAppStyle::GetBrush(TEXT("Graph.Panel.SolidBackground"));
		PaintBackgroundAsLines(DefaultBackground, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
		MaxLayerId++;

		// Draw a shadow overlay around the edges of the graph
		PaintSurroundSunkenShadow(FAppStyle::GetBrush(TEXT("Graph.Shadow")), AllottedGeometry, MyCullingRect, OutDrawElements, MaxLayerId);
		MaxLayerId++;

		PaintSoftwareCursor(AllottedGeometry, MyCullingRect, OutDrawElements, MaxLayerId);

		return MaxLayerId;
	}
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		const TSharedPtr<FGraphEditorDragDropAction> GraphDragDropOp = DragDropEvent.GetOperationAs<FGraphEditorDragDropAction>();
		if (!GraphDragDropOp)
		{
			return;
		}

		GraphDragDropOp->SetHoveredGraph(nullptr);

		const FSlateBrush* StatusSymbol = FAppStyle::GetBrush("Graph.ConnectorFeedback.Error");
		GraphDragDropOp->SetSimpleFeedbackMessage(StatusSymbol, FLinearColor::White, INVTEXT("Graph is Read-Only"));
	}
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override
	{
		const TSharedPtr<FGraphEditorDragDropAction> GraphDragDropOp = DragDropEvent.GetOperationAs<FGraphEditorDragDropAction>();
		if (!GraphDragDropOp)
		{
			return;
		}

		GraphDragDropOp->HoverTargetChanged();
	}
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		return FReply::Unhandled();
	}
	//~ End SNodePanel Interface
};

TSharedRef<SWidget> FVoxelGraphReadOnlySummoner::CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UVoxelGraph* VoxelGraph) const
{
	return
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVoxelReadOnlyGraph)
		]
		+ SOverlay::Slot()
		.Padding(20)
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Right)
		[
			SNew(STextBlock)
			.TextStyle(FAppStyle::Get(), "Graph.CornerText")
			.Text(
				VoxelGraph->IsFunctionLibrary()
				? INVTEXT("Empty Function Library")
				: INVTEXT("Main Graph is inherited"))
		]
		+ SOverlay::Slot()
		.Padding(10)
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		[
			SNew(SVerticalBox)
			.Visibility(EVisibility::HitTestInvisible)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, -10.f)
			[
				SNew(STextBlock)
				.TextStyle(FAppStyle::Get(), "Graph.CornerText")
				.Text(INVTEXT("VOXEL"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(FAppStyle::Get(), "Graph.CornerText")
				.Font(FCoreStyle::GetDefaultFontStyle("BoldCondensed", 16))
				.Text(FText::FromString(FVoxelUtilities::GetPluginVersion().ToString_UserFacing()))
			]
		];
}

const FSlateBrush* FVoxelGraphReadOnlySummoner::GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UVoxelGraph* VoxelGraph) const
{
	if (VoxelGraph->IsFunctionLibrary())
	{
		return FAppStyle::GetBrush("GraphEditor.Function_16x");
	}

	return FVoxelEditorStyle::GetBrush("VoxelGraph.Execute");
}

TSharedRef<FGenericTabHistory> FVoxelGraphReadOnlySummoner::CreateTabHistoryNode(TSharedPtr<FTabPayload> Payload)
{
	return MakeShared<FVoxelGraphTabHistory>(SharedThis(this), Payload);
}

void FVoxelGraphReadOnlySummoner::FocusReadOnlyPreview() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = WeakToolkit.Pin();
	if (!ensure(Toolkit))
	{
		return;
	}

	if (!Toolkit->WeakFocusedGraph.IsValid())
	{
		return;
	}

	Toolkit->WeakFocusedGraph = nullptr;
	Toolkit->GraphPreview->SetTerminalGraph(nullptr);
	Toolkit->GraphMembers->SetTerminalGraph(nullptr);
	Toolkit->GetSelection().Update();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphTabHistory::EvokeHistory(const TSharedPtr<FTabInfo> InTabInfo, const bool bPrevTabMatches)
{
	const TSharedPtr<SDockTab> DockTab = InTabInfo->GetTab().Pin();
	if (!ensure(DockTab))
	{
		return;
	}

	if (bPrevTabMatches)
	{
		WeakGraphEditor = FVoxelGraphEditorSummoner::GetGraphEditor(DockTab);
		return;
	}

	const TSharedPtr<FDocumentTabFactory> Factory = FactoryPtr.Pin();
	if (!ensure(Factory))
	{
		return;
	}

	FWorkflowTabSpawnInfo SpawnInfo;
	SpawnInfo.Payload = Payload;
	SpawnInfo.TabInfo = InTabInfo;

	const TSharedRef<SWidget> Widget = Factory->CreateTabBody(SpawnInfo);
	ON_SCOPE_EXIT
	{
		Factory->UpdateTab(DockTab, SpawnInfo, Widget);
	};

	if (Widget->GetType() != STATIC_FNAME("SOverlay"))
	{
		return;
	}

	const TSharedRef<SWidget> Child = Widget->GetChildren()->GetChildAt(0);
	if (Child->GetType() != STATIC_FNAME("SGraphEditor"))
	{
		return;
	}

	const TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Child);
	WeakGraphEditor = GraphEditor;
}

void FVoxelGraphTabHistory::SaveHistory()
{
	if (!IsHistoryValid())
	{
		return;
	}

	const TSharedPtr<SGraphEditor> GraphEditor = WeakGraphEditor.Pin();
	if (!GraphEditor)
	{
		return;
	}

	GraphEditor->GetViewLocation(SavedLocation, SavedZoomAmount);
	GraphEditor->GetViewBookmark(SavedBookmarkId);
}

void FVoxelGraphTabHistory::RestoreHistory()
{
	if (!IsHistoryValid())
	{
		return;
	}

	const TSharedPtr<SGraphEditor> GraphEditor = WeakGraphEditor.Pin();
	if (!GraphEditor)
	{
		return;
	}

	GraphEditor->SetViewLocation(SavedLocation, SavedZoomAmount, SavedBookmarkId);
}