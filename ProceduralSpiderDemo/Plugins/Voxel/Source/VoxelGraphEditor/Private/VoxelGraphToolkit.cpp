// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphToolkit.h"
#include "VoxelEdGraph.h"
#include "VoxelGraphSchema.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphCompiler.h"
#include "VoxelTerminalGraphRuntime.h"
#include "VoxelGraphCommandManager.h"
#include "VoxelGraphEditorSummoner.h"
#include "VoxelGraphEditorSettings.h"
#include "Members/SVoxelGraphMembers.h"
#include "Nodes/VoxelGraphDebugNode.h"
#include "Widgets/SVoxelGraphSearch.h"
#include "Widgets/SVoxelGraphPreview.h"
#include "Widgets/SVoxelGraphMessages.h"
#include "Selection/VoxelGraphSelection.h"
#include "Widgets/SVoxelGraphEditorActionMenu.h"
#include "Customizations/VoxelGraphPreviewSettingsCustomization.h"
#include "SGraphPanel.h"
#include "SourceCodeNavigation.h"
#include "VoxelGraphEditorUtilities.h"

VOXEL_RUN_ON_STARTUP_EDITOR()
{
	FVoxelGraphEditorUtilities::OnSelectNodes.BindLambda([](const TVoxelSet<const UEdGraphNode*>& Nodes)
	{
		TVoxelMap<TSharedPtr<SGraphEditor>, TVoxelArray<const UEdGraphNode*>> GraphEditorToNodes;
		for (const UEdGraphNode* Node : Nodes)
		{
			if (!Node)
			{
				continue;
			}

			const UVoxelGraph* Graph = Node->GetTypedOuter<UVoxelGraph>();
			if (!Graph)
			{
				continue;
			}

			const UEdGraph* EdGraph = Node->GetGraph();
			if (!ensure(EdGraph))
			{
				continue;
			}

			const FVoxelGraphToolkit* Toolkit = FVoxelToolkit::OpenToolkit<FVoxelGraphToolkit>(*Graph);
			if (!ensure(Toolkit))
			{
				continue;
			}

			TSharedPtr<SGraphEditor> GraphEditor = Toolkit->FindGraphEditor(*EdGraph);
			if (!GraphEditor ||
				!GraphEditorToNodes.Contains(GraphEditor))
			{
				GraphEditor = Toolkit->OpenGraphAndBringToFront(*EdGraph, true);
				if (!ensure(GraphEditor))
				{
					continue;
				}

				GraphEditorToNodes.Add_EnsureNew(GraphEditor);
			}

			GraphEditorToNodes[GraphEditor].Add(Node);
		}

		for (const auto& It : GraphEditorToNodes)
		{
			It.Key->ClearSelectionSet();

			for (const UEdGraphNode* Node : It.Value)
			{
				It.Key->SetNodeSelection(ConstCast(Node), true);
			}

			It.Key->ZoomToFit(true);
		}
	});
}

TSharedPtr<FVoxelGraphToolkit> FVoxelGraphToolkit::Get(const UEdGraph* EdGraph)
{
	if (!EdGraph)
	{
		return nullptr;
	}

	const UVoxelEdGraph* VoxelEdGraph = Cast<UVoxelEdGraph>(EdGraph);
	if (!ensure(VoxelEdGraph))
	{
		return nullptr;
	}

	return VoxelEdGraph->GetGraphToolkit();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphToolkit::Initialize()
{
	DocumentManager = MakeShared<FDocumentTracker>();
	// HostingApp is not used
	DocumentManager->Initialize(nullptr);
	DocumentManager->RegisterDocumentFactory(MakeShared<FVoxelGraphEditorSummoner>(SharedThis(this)));
	DocumentManager->RegisterDocumentFactory(MakeShared<FVoxelGraphReadOnlySummoner>(SharedThis(this)));

	GraphPreview =
		SNew(SVoxelGraphPreview)
		.Toolkit(SharedThis(this));

	Viewport = SNew(SVoxelViewport);

	if (ViewportInterface)
	{
		ViewportInterface->SetupPreview(*Asset, Viewport.ToSharedRef());

		// Make sure to initialize the viewport after SetupPreview so that the component bounds are correct
		Viewport->Initialize(ViewportInterface.ToSharedRef());
	}

	GraphPreviewStats = GraphPreview->GetPreviewStats();

	GraphMembers =
		SNew(SVoxelGraphMembers)
		.Toolkit(SharedThis(this));

	SearchWidget =
		SNew(SVoxelGraphSearch)
		.Toolkit(SharedThis(this));

	GraphMessages =
		SNew(SVoxelGraphMessages)
		.Graph(Asset);

	{
		FDetailsViewArgs Args;
		Args.bAllowSearch = false;
		Args.bShowOptions = false;
		Args.bHideSelectionTip = true;
		Args.bShowPropertyMatrixButton = false;
		Args.NotifyHook = GetNotifyHook();
		Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		DetailsView = PropertyModule.CreateDetailView(Args);
	}

	{
		FDetailsViewArgs Args;
		Args.bAllowSearch = false;
		Args.bShowOptions = false;
		Args.bHideSelectionTip = true;
		Args.bShowPropertyMatrixButton = false;
		Args.NotifyHook = GetNotifyHook();
		Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PreviewDetailsView = PropertyModule.CreateDetailView(Args);
		PreviewDetailsView->SetGenericLayoutDetailsDelegate(MakeLambdaDelegate([]() -> TSharedRef<IDetailCustomization>
		{
			return MakeShared<FVoxelGraphPreviewSettingsCustomization>();
		}));
	}

	Selection = MakeShared<FVoxelGraphSelection>(
		*this,
		DetailsView.ToSharedRef(),
		GraphMembers.ToSharedRef());

	if (!Asset->IsFunctionLibrary())
	{
		Selection->SelectMainGraph();
	}

	CommandManager = MakeShared<FVoxelGraphCommandManager>(*this);
	CommandManager->MapActions();

	// Fixup graphs
	Asset->ForeachTerminalGraph_NoInheritance([&](UVoxelTerminalGraph& TerminalGraph)
	{
		FixupTerminalGraph(&TerminalGraph);
	});

	// Once everything is ready, update errors
	Asset->ForeachTerminalGraph_NoInheritance([&](const UVoxelTerminalGraph& TerminalGraph)
	{
		TerminalGraph.GetRuntime().EnsureIsCompiled();
	});

	// Force a message update if we're loading an already compiled graph
	GraphMessages->UpdateNodes();

	// Force load all pin types
	FVoxelPinTypeSet::GetAllTypes();

	if (Asset->IsFunctionLibrary())
	{
		return;
	}

	GVoxelGraphTracker->OnTerminalGraphChanged(*Asset).Add(FOnVoxelGraphChanged::Make(this, [this]
	{
		if (Asset->HasMainTerminalGraph() ||
			Asset->GetBaseGraph_Unsafe())
		{
			return;
		}

		const FVoxelTransaction Transaction(Asset, "Fix main graph");
		const UVoxelTerminalGraph* Template = nullptr;
		if (UVoxelGraph* TemplateGraph = Asset->GetFactoryInfo().Template)
		{
			Template = &TemplateGraph->GetMainTerminalGraph();
		}

		UVoxelTerminalGraph& MainTerminalGraph = Asset->AddTerminalGraph(GVoxelMainTerminalGraphGuid, Template);

		CloseEmptyTab();

		if (DocumentManager->GetAllDocumentTabs().Num() > 0)
		{
			return;
		}

		OpenGraphAndBringToFront(MainTerminalGraph.GetEdGraph(), true);
	}));
}

void FVoxelGraphToolkit::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	{
		VOXEL_SCOPE_COUNTER("NodesToReconstruct");

		const TSet<TVoxelObjectPtr<UEdGraphNode>> NodesToReconstructCopy = MoveTemp(NodesToReconstruct);
		ensure(NodesToReconstruct.Num() == 0);

		for (const TVoxelObjectPtr<UEdGraphNode>& WeakNode : NodesToReconstructCopy)
		{
			UEdGraphNode* Node = WeakNode.Resolve();
			if (!ensure(Node))
			{
				continue;
			}

			Node->ReconstructNode();
		}
	}

	if (bMessageUpdateQueued)
	{
		bMessageUpdateQueued = false;

		// Focus Messages tab
		const TSharedPtr<FTabManager> TabManager = GetTabManager();
		if (ensure(TabManager))
		{
			const TSharedPtr<SDockTab> MessagesTab = TabManager->FindExistingLiveTab(FTabId(MessagesTabId));
			if (ensure(MessagesTab) &&
				// Only focus if asset is focused
				TabManager->GetOwnerTab()->HasAnyUserFocus())
			{
				MessagesTab->DrawAttention();
			}
		}

		GraphMessages->UpdateNodes();
	}
}

void FVoxelGraphToolkit::BuildMenu(FMenuBarBuilder& MenuBarBuilder)
{
	Super::BuildMenu(MenuBarBuilder);

	MenuBarBuilder.AddPullDownMenu(
		INVTEXT("Graph"),
		INVTEXT(""),
		FNewMenuDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.BeginSection("Search", INVTEXT("Search"));
			{
				MenuBuilder.AddMenuEntry(FVoxelGraphCommands::Get().FindInGraph);
				MenuBuilder.AddMenuEntry(FVoxelGraphCommands::Get().FindInGraphs);
			}
			MenuBuilder.EndSection();

			MenuBuilder.BeginSection("Tools", INVTEXT("Tools"));
			{
				MenuBuilder.AddMenuEntry(FVoxelGraphCommands::Get().Compile);
				MenuBuilder.AddMenuEntry(FVoxelGraphCommands::Get().ReconstructAllNodes);
			}
			MenuBuilder.EndSection();
		}));
}

void FVoxelGraphToolkit::BuildToolbar(FToolBarBuilder& ToolbarBuilder)
{
	Super::BuildToolbar(ToolbarBuilder);

	ToolbarBuilder.BeginSection("Voxel");
	ToolbarBuilder.AddToolBarButton(FVoxelGraphCommands::Get().EnableStats);
	ToolbarBuilder.AddToolBarButton(FVoxelGraphCommands::Get().EnableRangeStats);
	ToolbarBuilder.EndSection();
}

TSharedPtr<FTabManager::FLayout> FVoxelGraphToolkit::GetLayout() const
{
	return FTabManager::NewLayout("FVoxelGraphToolkit_Layout_v8")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.15f)
				->SetHideTabWell(true)
				->AddTab(MembersTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.7f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.8f)
					->SetHideTabWell(true)
					->AddTab("Document", ETabState::ClosedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(MessagesTabId, ETabState::OpenedTab)
					->AddTab(SearchTabId, ETabState::OpenedTab)
					->SetForegroundTab(FTabId(MessagesTabId))
				)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.15f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->SetHideTabWell(true)
					->AddTab(PreviewTabId, ETabState::OpenedTab)
					->AddTab(ViewportTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
					->SetForegroundTab(FTabId(DetailsTabId))
				)
			)
		);
}

void FVoxelGraphToolkit::RegisterTabs(const FRegisterTab RegisterTab)
{
	RegisterTab(PreviewTabId, INVTEXT("Preview"), "LevelEditor.Tabs.Viewports", GraphPreview);
	RegisterTab(ViewportTabId, INVTEXT("Viewport"), "LevelEditor.Tabs.Viewports", Viewport);
	RegisterTab(PreviewStatsTabId, INVTEXT("Stats"), "MaterialEditor.ToggleMaterialStats.Tab", GraphPreviewStats);
	RegisterTab(DetailsTabId, INVTEXT("Details"), "LevelEditor.Tabs.Details", DetailsView);
	RegisterTab(PreviewDetailsTabId, INVTEXT("Preview Settings"), "LevelEditor.Tabs.Details", PreviewDetailsView);
	RegisterTab(MembersTabId, INVTEXT("Members"), "ClassIcon.BlueprintCore", GraphMembers);
	RegisterTab(MessagesTabId, INVTEXT("Messages"), "MessageLog.TabIcon", GraphMessages);
	RegisterTab(SearchTabId, INVTEXT("Find Results"), "Kismet.Tabs.FindResults", SearchWidget);
}

void FVoxelGraphToolkit::SaveDocuments()
{
	Asset->LastEditedDocuments.Empty();
	DocumentManager->SaveAllState();
}

void FVoxelGraphToolkit::LoadDocuments()
{
	Asset->LastEditedDocuments.RemoveAll([](const FVoxelEditedDocumentInfo& Document)
	{
		return !Document.EdGraph.Get();
	});

	if (Asset->LastEditedDocuments.Num() == 0)
	{
		if (Asset->IsFunctionLibrary())
		{
			for (const FGuid& Guid : Asset->GetTerminalGraphs())
			{
				const UVoxelTerminalGraph* TerminalGraph = Asset->FindTerminalGraph(Guid);
				if (!ensure(TerminalGraph) ||
					TerminalGraph->IsMainTerminalGraph() ||
					TerminalGraph->IsEditorTerminalGraph())
				{
					continue;
				}

				FVoxelEditedDocumentInfo DocumentInfo;
				DocumentInfo.EdGraph = &ConstCast(TerminalGraph->GetEdGraph());
				Asset->LastEditedDocuments.Add(DocumentInfo);
				break;
			}

			if (Asset->LastEditedDocuments.Num() == 0)
			{
				FVoxelEditedDocumentInfo DocumentInfo;
				Asset->LastEditedDocuments.Add(DocumentInfo);
			}
		}
		else
		{
			FVoxelEditedDocumentInfo DocumentInfo;
			if (Asset->HasMainTerminalGraph())
			{
				DocumentInfo.EdGraph = &Asset->GetMainTerminalGraph().GetEdGraph();
			}
			else if (
				!Asset->GetBaseGraph_Unsafe() &&
				!Asset->IsFunctionLibrary())
			{
				// If the main graph is missing and it's not inherited, create it
				UVoxelTerminalGraph* Template = nullptr;
				if (UVoxelGraph* TemplateGraph = Asset->GetFactoryInfo().Template)
				{
					Template = &TemplateGraph->GetMainTerminalGraph();
				}

				Asset->AddTerminalGraph(GVoxelMainTerminalGraphGuid, Template);

				DocumentInfo.EdGraph = &Asset->GetMainTerminalGraph().GetEdGraph();
			}
			Asset->LastEditedDocuments.Add(DocumentInfo);
		}
	}

	for (const FVoxelEditedDocumentInfo& Document : Asset->LastEditedDocuments)
	{
		const UObject* TabOwner = Document.EdGraph.Get();
		if (!TabOwner)
		{
			TabOwner = Asset;
		}

		const TSharedPtr<SDockTab> Tab = DocumentManager->OpenDocument(FTabPayload_UObject::Make(TabOwner), FDocumentTracker::RestorePreviousDocument);
		if (!ensure(Tab))
		{
			continue;
		}

		if (const TSharedPtr<SGraphEditor> GraphEditor = FVoxelGraphEditorSummoner::GetGraphEditor(Tab))
		{
			GraphEditor->SetViewLocation(UE_506_ONLY((FVector2f))Document.ViewLocation, Document.ZoomAmount);
		}
	}
}

void FVoxelGraphToolkit::PostUndo()
{
	Super::PostUndo();

	DocumentManager->RefreshAllTabs();
	DocumentManager->CleanInvalidTabs();

	// Clear selection, to avoid holding refs to nodes that go away

	Asset->ForeachTerminalGraph_NoInheritance([&](const UVoxelTerminalGraph& TerminalGraph)
	{
		if (const TSharedPtr<SGraphEditor> GraphEditor = FindGraphEditor(TerminalGraph.GetEdGraph()))
		{
			GraphEditor->ClearSelectionSet();
			GraphEditor->NotifyGraphChanged();
		}
	});
}

void FVoxelGraphToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);

	GraphPreview->AddReferencedObjects(Collector);
}

TSharedPtr<SWidget> FVoxelGraphToolkit::GetMenuOverlay() const
{
	return MakeMenuOverlay(Asset);
}

void FVoxelGraphToolkit::SetTabManager(const TSharedRef<FTabManager>& TabManager)
{
	Super::SetTabManager(TabManager);

	DocumentManager->SetTabManager(TabManager);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<SGraphEditor> FVoxelGraphToolkit::GetActiveGraphEditor() const
{
	return WeakFocusedGraph.Pin();
}

UEdGraph* FVoxelGraphToolkit::GetActiveEdGraph() const
{
	const TSharedPtr<SGraphEditor> GraphEditor = GetActiveGraphEditor();
	if (!GraphEditor)
	{
		return nullptr;
	}
	return GraphEditor->GetCurrentGraph();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphToolkit::FixupTerminalGraph(UVoxelTerminalGraph* TerminalGraph)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(TerminalGraph))
	{
		return;
	}

	UVoxelEdGraph& EdGraph = TerminalGraph->GetTypedEdGraph();
	EdGraph.Schema = UVoxelGraphSchema::StaticClass();
	EdGraph.SetToolkit(SharedThis(this));
	EdGraph.MigrateAndReconstructAll();

	TerminalGraph->GetRuntime().OnMessagesChanged.RemoveAll(this);
	TerminalGraph->GetRuntime().OnMessagesChanged.Add(MakeWeakPtrDelegate(this, [=, this]
	{
		bMessageUpdateQueued = true;
	}));
}

TSharedPtr<SGraphEditor> FVoxelGraphToolkit::FindGraphEditor(const UEdGraph& EdGraph) const
{
	for (const TSharedPtr<SDockTab>& Tab : DocumentManager->GetAllDocumentTabs())
	{
		if (!ensure(Tab))
		{
			continue;
		}

		if (const TSharedPtr<SGraphEditor> GraphEditor = FVoxelGraphEditorSummoner::GetGraphEditor(Tab))
		{
			if (GraphEditor->GetCurrentGraph() == &EdGraph)
			{
				return GraphEditor;
			}
		}
	}

	return nullptr;
}

TSharedRef<SGraphEditor> FVoxelGraphToolkit::CreateGraphEditor(UEdGraph& EdGraph, const bool bBindOnSelect)
{
	VOXEL_FUNCTION_COUNTER();

	// Make sure to fixup the graph before opening it
	if (!EdGraph.HasAnyFlags(RF_Transient))
	{
		FixupTerminalGraph(EdGraph.GetTypedOuter<UVoxelTerminalGraph>());
	}

	const FGraphAppearanceInfo AppearanceInfo;

	SGraphEditor::FGraphEditorEvents Events;
	if (bBindOnSelect &&
		!EdGraph.HasAnyFlags(RF_Transient))
	{
		Events.OnSelectionChanged = MakeLambdaDelegate([
			=,
			this,
			WeakTerminalGraph = MakeWeakObjectPtr(EdGraph.GetTypedOuter<UVoxelTerminalGraph>())
		](const TSet<UObject*>& NewSelection)
		{
			const UVoxelTerminalGraph* TerminalGraph = WeakTerminalGraph.Get();
			if (!ensure(TerminalGraph))
			{
				return;
			}

			if (NewSelection.Num() == 0)
			{
				Selection->SelectTerminalGraph(*TerminalGraph);
			}
			else if (NewSelection.Num() == 1)
			{
				Selection->SelectNode(CastChecked<UEdGraphNode>(NewSelection.Array()[0]));
			}
			else
			{
				Selection->SelectNone();
			}

			if (NewSelection.Num() == 1)
			{
				PreviewDetailsView->SetObject(NewSelection.Array()[0]);
			}
			else
			{
				PreviewDetailsView->SetObject(nullptr);
			}
		});
	}
	Events.OnTextCommitted = MakeLambdaDelegate([](const FText& NewText, ETextCommit::Type, UEdGraphNode* NodeBeingChanged)
	{
		if (!ensure(NodeBeingChanged))
		{
			return;
		}

		const FVoxelTransaction Transaction(NodeBeingChanged, "Rename Node");
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	});
	Events.OnNodeDoubleClicked = MakeLambdaDelegate([](UEdGraphNode* Node)
	{
		if (Node->CanJumpToDefinition())
		{
			Node->JumpToDefinition();
		}
	});

#if VOXEL_ENGINE_VERSION >= 506
	Events.OnSpawnNodeByShortcutAtLocation = MakeWeakPtrDelegate(this, [this](FInputChord Chord, const FVector2f& Position)
	{
		for (const FVoxelGraphEditorInputBinding& Input : GetDefault<UVoxelGraphEditorSettings>()->Hotkeys)
		{
			if (Input.InputChord == Chord)
			{
				if (const TSharedPtr<FEdGraphSchemaAction> Action = Input.CreateAction())
				{
					Action->PerformAction(GetActiveEdGraph(), nullptr, Position);
				}
			}
		}

		return FReply::Handled();
	});

	Events.OnCreateActionMenuAtLocation = MakeLambdaDelegate([](
		UEdGraph* InGraph,
		const FVector2f& InNodePosition,
		const TArray<UEdGraphPin*>& InDraggedPins,
		const bool bAutoExpand,
		SGraphEditor::FActionMenuClosed InOnMenuClosed)
	{
		const TSharedRef<SVoxelGraphEditorActionMenu> Menu =
			SNew(SVoxelGraphEditorActionMenu)
			.GraphObj(InGraph)
			.NewNodePosition(InNodePosition)
			.DraggedFromPins(InDraggedPins)
			.AutoExpandActionMenu(bAutoExpand)
			.OnClosedCallback(InOnMenuClosed)
			.OnActionSelected_Lambda([](const TSharedPtr<FEdGraphSchemaAction>& Action, UEdGraph* Graph, TArray<UEdGraphPin*>& DraggedFromPins, const UE_506_SWITCH(FVector2D, FVector2f)& NewNodePosition)
			{
				if (!Graph)
				{
					return;
				}

				FSlateApplication::Get().DismissAllMenus();
				Action->PerformAction(Graph, DraggedFromPins, NewNodePosition);
			});

		return FActionMenuContent(Menu, Menu->GetFilterTextBox());
	});
#else
	Events.OnSpawnNodeByShortcut = MakeWeakPtrDelegate(this, [this](FInputChord Chord, const FVector2D& Position)
	{
		for (const FVoxelGraphEditorInputBinding& Input : GetDefault<UVoxelGraphEditorSettings>()->Hotkeys)
		{
			if (Input.InputChord == Chord)
			{
				if (const TSharedPtr<FEdGraphSchemaAction> Action = Input.CreateAction())
				{
					Action->PerformAction(GetActiveEdGraph(), nullptr, Position);
				}
			}
		}

		return FReply::Handled();
	});

	Events.OnCreateActionMenu = MakeLambdaDelegate([](
		UEdGraph* InGraph,
		const FVector2D& InNodePosition,
		const TArray<UEdGraphPin*>& InDraggedPins,
		const bool bAutoExpand,
		SGraphEditor::FActionMenuClosed InOnMenuClosed)
	{
		const TSharedRef<SVoxelGraphEditorActionMenu> Menu =
			SNew(SVoxelGraphEditorActionMenu)
			.GraphObj(InGraph)
			.NewNodePosition(InNodePosition)
			.DraggedFromPins(InDraggedPins)
			.AutoExpandActionMenu(bAutoExpand)
			.OnClosedCallback(InOnMenuClosed)
			.OnActionSelected_Lambda([](const TSharedPtr<FEdGraphSchemaAction>& Action, UEdGraph* Graph, TArray<UEdGraphPin*>& DraggedFromPins, const UE_506_SWITCH(FVector2D, FVector2f)& NewNodePosition)
			{
				if (!Graph)
				{
					return;
				}

				FSlateApplication::Get().DismissAllMenus();
				Action->PerformAction(Graph, DraggedFromPins, NewNodePosition);
			});

		return FActionMenuContent(Menu, Menu->GetFilterTextBox());
	});
#endif

	return
		SNew(SGraphEditor)
		.AdditionalCommands(GetCommands())
		.Appearance(AppearanceInfo)
		.GraphToEdit(&EdGraph)
		.GraphEvents(Events)
		.AutoExpandActionMenu(false)
		.ShowGraphStateOverlay(false);
}

TSharedPtr<SGraphEditor> FVoxelGraphToolkit::OpenGraphAndBringToFront(const UEdGraph& EdGraph, const bool bSetFocus) const
{
	const TSharedPtr<SDockTab> Tab = DocumentManager->OpenDocument(FTabPayload_UObject::Make(&EdGraph), FDocumentTracker::OpenNewDocument);
	if (!ensure(Tab))
	{
		return nullptr;
	}

	const TSharedPtr<SGraphEditor> GraphEditor = FVoxelGraphEditorSummoner::GetGraphEditor(Tab);
	if (!ensure(GraphEditor))
	{
		return nullptr;
	}

	if (bSetFocus)
	{
		GraphEditor->CaptureKeyboard();
	}
	return GraphEditor;
}

void FVoxelGraphToolkit::OpenEmptyTab() const
{
	if (DocumentManager->GetAllDocumentTabs().Num() > 0)
	{
		return;
	}

	DocumentManager->OpenDocument(FTabPayload_UObject::Make(Asset), FDocumentTracker::OpenNewDocument);
}

void FVoxelGraphToolkit::CloseGraph(UEdGraph& EdGraph)
{
	ensure(!GraphBeingClosed.IsValid_Slow());
	GraphBeingClosed = &EdGraph;
	{
		DocumentManager->CloseTab(FTabPayload_UObject::Make(&EdGraph));
	}
	ensure(GraphBeingClosed == &EdGraph);
	GraphBeingClosed = nullptr;
}

void FVoxelGraphToolkit::CloseEmptyTab() const
{
	DocumentManager->CloseTab(FTabPayload_UObject::Make(Asset));
}

TSet<UEdGraphNode*> FVoxelGraphToolkit::GetSelectedNodes() const
{
	const TSharedPtr<SGraphEditor> GraphEditor = GetActiveGraphEditor();
	if (!GraphEditor)
	{
		return {};
	}

	// Removed nodes might still be selected & might only be unselected on the next frame as
	// removal might still be pending from GraphEditor
	const TSet<UEdGraphNode*> ValidNodes(GraphEditor->GetCurrentGraph()->Nodes);

	TSet<UEdGraphNode*> SelectedNodes;
	for (UObject* Object : GraphEditor->GetSelectedNodes())
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (!ensure(Node) ||
			!ValidNodes.Contains(Node))
		{
			continue;
		}

		SelectedNodes.Add(Node);
	}
	return SelectedNodes;
}

void FVoxelGraphToolkit::AddNodeToReconstruct(UEdGraphNode* Node)
{
	NodesToReconstruct.Add(Node);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelGraphToolkit::IsGraphSensitive() const
{
	return bSensitiveContext;
}

void FVoxelGraphToolkit::SetIsGraphSensitive(const bool bValue)
{
	bSensitiveContext = bValue;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphToolkit::OpenIntermediateGraph(const UVoxelTerminalGraph& TerminalGraph, const FName PassName)
{
	TerminalGraph.GetRuntime().EnsureIsCompiled(
		true,
		[&](const FName Pass, FVoxelGraphCompiler& Compiler)
		{
			if (Pass != PassName)
			{
				return;
			}

			const TSharedRef<Voxel::Graph::FGraph> NewGraph = MakeShared<Voxel::Graph::FGraph>();
			Compiler.CloneInto(*NewGraph);

			UEdGraph* EdGraph = WeakIntermediateGraph.Resolve();
			if (!EdGraph)
			{
				UVoxelEdGraph* VoxelEdGraph = NewObject<UVoxelEdGraph>(GetTransientPackage(), NAME_None, RF_Transient);
				VoxelEdGraph->SetLatestVersion();
				VoxelEdGraph->Schema = UVoxelGraphSchema::StaticClass();
				VoxelEdGraph->SetToolkit(SharedThis(this));

				WeakIntermediateGraph = VoxelEdGraph;
				EdGraph = VoxelEdGraph;
			}

			if (TerminalGraph.IsMainTerminalGraph())
			{
				IntermediateGraphName = "Main Graph";
			}
			else if (TerminalGraph.IsEditorTerminalGraph())
			{
				IntermediateGraphName = "Editor Graph";
			}
			else
			{
				IntermediateGraphName = TerminalGraph.GetDisplayName();
			}
			IntermediateGraphName += " | " + FName::NameToDisplayString(PassName.ToString(), false);

			// Remove existing nodes
			{
				TArray<TObjectPtr<UEdGraphNode>> CopiedNodes = EdGraph->Nodes;
				for (UEdGraphNode* Node : CopiedNodes)
				{
					EdGraph->RemoveNode(Node);
				}
			}

			const FVoxelIntBox2D Bounds = INLINE_LAMBDA
			{
				TVoxelArray<FIntPoint> Positions;
				for (const Voxel::Graph::FNode& Node : NewGraph->GetNodes())
				{
					const UEdGraphNode* GraphNode = Node.NodeRef.GetGraphNode_EditorOnly();
					if (!GraphNode)
					{
						continue;
					}
					Positions.Add(FIntPoint(GraphNode->NodePosX * 2, GraphNode->NodePosY * 2));
				}
				return FVoxelIntBox2D::FromPositions(Positions);
			};

			TMap<const Voxel::Graph::FNode*, UVoxelGraphDebugNode*> NodeMap;
			for (const Voxel::Graph::FNode& Node : NewGraph->GetNodes())
			{
				FGraphNodeCreator<UVoxelGraphDebugNode> NodeCreator(*EdGraph);
				UVoxelGraphDebugNode* DebugNode = NodeCreator.CreateNode(false);
				DebugNode->Node = &Node;
				DebugNode->Graph = NewGraph;
				NodeCreator.Finalize();

				NodeMap.Add(&Node, DebugNode);

				const UEdGraphNode* GraphNode = Node.NodeRef.GetGraphNode_EditorOnly();
				if (!GraphNode)
				{
					DebugNode->NodePosX = Bounds.Max.X;
					DebugNode->NodePosY = Bounds.Max.Y;
					continue;
				}

				DebugNode->NodePosX = GraphNode->NodePosX * 2;
				DebugNode->NodePosY = GraphNode->NodePosY * 2;
			}
	
			for (const Voxel::Graph::FNode& Node : NewGraph->GetNodes())
			{
				UVoxelGraphDebugNode* DebugNode = NodeMap[&Node];

				for (const Voxel::Graph::FPin& Pin : Node.GetPins())
				{
					UEdGraphPin* DebugPin = DebugNode->PinMap[&Pin];

					for (Voxel::Graph::FPin& OtherPin : Pin.GetLinkedTo())
					{
						UVoxelGraphDebugNode* OtherNode = NodeMap[&OtherPin.Node];
						UEdGraphPin* OtherDebugPin = OtherNode->PinMap[&OtherPin];
						DebugPin->MakeLinkTo(OtherDebugPin);
					}
				}
			}

			OpenGraphAndBringToFront(*EdGraph, true);
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SWidget> FVoxelGraphToolkit::MakeMenuOverlay(UVoxelGraph* Graph)
{
	return
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 2.f, 0.f)
		[
			SNew(STextBlock)
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			.ShadowOffset(FVector2D::UnitVector)
			.Text_Lambda([Graph]
			{
				return Graph->GetBaseGraph_Unsafe() ? INVTEXT("Parent: ") : INVTEXT("Type: ");
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SHyperlink)
			.Style(FAppStyle::Get(), "Common.GotoNativeCodeHyperlink")
			.Visibility_Lambda([Graph]
			{
				return Graph->GetBaseGraph_Unsafe() ? EVisibility::Collapsed : EVisibility::Visible;
			})
			.OnNavigate_Lambda([Graph]
			{
				FSourceCodeNavigation::NavigateToClass(Graph->GetClass());
			})
			.Text_Lambda([Graph]
			{
				return Graph->GetClass()->GetDisplayNameText();
			})
			.ToolTipText(FText::Format(INVTEXT("Click to open this source file in {0}"), FSourceCodeNavigation::GetSelectedSourceCodeIDE()))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			.Visibility_Lambda([Graph]
			{
				return Graph->GetBaseGraph_Unsafe() ? EVisibility::Visible : EVisibility::Collapsed;
			})
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.ShadowOffset(FVector2D::UnitVector)
				.TextStyle(FAppStyle::Get(), "Common.InheritedFromBlueprintTextStyle")
				.Text_Lambda([Graph]
				{
					const UVoxelGraph* BaseGraph = Graph->GetBaseGraph_Unsafe();
					if (!BaseGraph)
					{
						return INVTEXT("None");
					}

					return FText::FromString(BaseGraph->GetMetadata().DisplayName);
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.OnClicked_Lambda([Graph]
				{
					const UVoxelGraph* BaseGraph = Graph->GetBaseGraph_Unsafe();
					if (!BaseGraph)
					{
						return FReply::Handled();
					}

					GEditor->SyncBrowserToObject(BaseGraph);
					return FReply::Handled();
				})
				.ToolTipText(INVTEXT("Find parent in Content Browser"))
				.ContentPadding(4.f)
				.ForegroundColor(FSlateColor::UseForeground())
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Search"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.OnClicked_Lambda([Graph]
				{
					const UVoxelGraph* BaseGraph = Graph->GetBaseGraph_Unsafe();
					if (!BaseGraph)
					{
						return FReply::Handled();
					}

					FVoxelUtilities::FocusObject(*BaseGraph);
					return FReply::Handled();
				})
				.ToolTipText(INVTEXT("Open parent in editor"))
				.ContentPadding(4.f)
				.ForegroundColor(FSlateColor::UseForeground())
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Edit"))
				]
			]
		];
}