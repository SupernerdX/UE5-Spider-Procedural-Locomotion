// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMembersSchemaAction_MainGraph.h"
#include "VoxelGraph.h"
#include "VoxelGraphToolkit.h"
#include "VoxelGraphCompiler.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphSchemaAction.h"
#include "Nodes/VoxelGraphNode_Struct.h"
#include "Selection/VoxelGraphSelection.h"

VOXEL_INITIALIZE_STYLE(VoxelMainGraphButtonStyle)
{
	Set("Voxel.Members.MainGraph.Implement", FAppStyle::GetWidgetStyle<FComboButtonStyle>("ComboButton").ButtonStyle);
}

UObject* FVoxelMembersSchemaAction_MainGraph::GetOuter() const
{
	return WeakGraph.Resolve();
}

TSharedRef<SWidget> FVoxelMembersSchemaAction_MainGraph::CreatePaletteWidget(const FCreateWidgetForActionData& CreateData) const
{
	return SNew(SVoxelMembersPaletteItem_MainGraph, CreateData);
}

void FVoxelMembersSchemaAction_MainGraph::BuildContextMenu(FMenuBuilder& MenuBuilder)
{
	const UVoxelGraph* Graph = WeakGraph.Resolve();
	if (!ensure(Graph))
	{
		return;
	}

	if (Graph->HasMainTerminalGraph())
	{
		MenuBuilder.AddSubMenu(
			INVTEXT("Intermediate Graph"),
			INVTEXT("Open graph after selected compiler pass"),
			MakeLambdaDelegate([this](FMenuBuilder& Builder)
			{
				for (const FVoxelGraphCompilerPass& Pass : GVoxelGraphCompilerPasses)
				{
					Builder.AddMenuEntry(
						FText::FromString("Open " + Pass.Name.ToString() + " Pass"),
						{},
						{},
						FUIAction
						{
							MakeLambdaDelegate([this, PassName = Pass.Name]
							{
								const UVoxelGraph* LocalGraph = WeakGraph.Resolve();
								const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
								if (!ensure(LocalGraph) ||
									!ensure(Toolkit))
								{
									return;
								}

								Toolkit->OpenIntermediateGraph(LocalGraph->GetMainTerminalGraph(), PassName);
							})
						});
				}
			}));
	}

	if (!Graph->GetBaseGraph_Unsafe())
	{
		return;
	}

	MenuBuilder.AddMenuEntry(
		INVTEXT("Go to parent"),
		INVTEXT("Go to the parent graph"),
		{},
		FUIAction
		{
			MakeLambdaDelegate([this]
			{
				const UVoxelGraph* LocalGraph = WeakGraph.Resolve();
				if (!ensure(LocalGraph))
				{
					return;
				}

				UVoxelGraph* BaseGraph = LocalGraph->GetBaseGraph_Unsafe();
				if (!BaseGraph)
				{
					return;
				}

				if (BaseGraph->HasMainTerminalGraph())
				{
					FVoxelUtilities::FocusObject(BaseGraph->GetMainTerminalGraph());
				}
				else
				{
					FVoxelUtilities::FocusObject(BaseGraph);
				}
			})
		});
}

void FVoxelMembersSchemaAction_MainGraph::ApplyNewGuids(const TArray<FGuid>& NewGuids) const
{
	ensure(NewGuids.Num() == 1);
}

void FVoxelMembersSchemaAction_MainGraph::OnActionDoubleClick() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return;
	}

	for (UVoxelGraph* Graph : Toolkit->Asset->GetBaseGraphs())
	{
		if (Graph->HasMainTerminalGraph())
		{
			FVoxelUtilities::FocusObject(Graph->GetMainTerminalGraph());
			break;
		}
	}
}

void FVoxelMembersSchemaAction_MainGraph::OnSelected() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return;
	}

	Toolkit->GetSelection().SelectMainGraph();
}

void FVoxelMembersSchemaAction_MainGraph::OnDelete() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit) ||
		!ensure(Toolkit->Asset->GetBaseGraph_Unsafe()) ||
		!Toolkit->Asset->HasMainTerminalGraph())
	{
		return;
	}

	UVoxelTerminalGraph& MainTerminalGraph = Toolkit->Asset->GetMainTerminalGraph();
	Toolkit->CloseGraph(MainTerminalGraph.GetEdGraph());

	{
		UVoxelGraph& Graph = MainTerminalGraph.GetGraph();
		const FVoxelTransaction Transaction(Graph, "Delete main graph");
		MainTerminalGraph.Modify();

		Graph.RemoveTerminalGraph(MemberGuid);
		MainTerminalGraph.MarkAsGarbage();
	}

	// Open an empty tab if there's no opened tabs
	Toolkit->OpenEmptyTab();
}

FString FVoxelMembersSchemaAction_MainGraph::GetName() const
{
	return "Main Graph";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelMembersSchemaAction_MainGraph::CanBeDeleted() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return false;
	}

	return
		Toolkit->Asset->GetBaseGraph_Unsafe() &&
		Toolkit->Asset->HasMainTerminalGraph();
}

bool FVoxelMembersSchemaAction_MainGraph::IsOverriden() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return false;
	}

	return !Toolkit->Asset->HasMainTerminalGraph();
}

void FVoxelMembersSchemaAction_MainGraph::CreateMainGraph() const
{
	const TSharedPtr<FVoxelGraphToolkit> Toolkit = GetToolkit();
	if (!ensure(Toolkit))
	{
		return;
	}

	UVoxelGraph* Graph = Toolkit->Asset;
	if (!ensure(Graph) ||
		Graph->HasMainTerminalGraph())
	{
		return;
	}

	{
		const FVoxelTransaction Transaction(Graph, "Create new function");

		const UVoxelTerminalGraph* Template = nullptr;
		if (UVoxelGraph* TemplateGraph = Graph->GetFactoryInfo().Template)
		{
			Template = &TemplateGraph->GetMainTerminalGraph();
		}

		UVoxelTerminalGraph& TerminalGraph = Graph->AddTerminalGraph(GVoxelMainTerminalGraphGuid, Template);

		const TVoxelInstancedStruct<FVoxelNode> ParentOutputNode = INLINE_LAMBDA -> TVoxelInstancedStruct<FVoxelNode>
		{
			const UScriptStruct* OutputNodeStruct = Graph->GetOutputNodeStruct();
			if (!OutputNodeStruct)
			{
				return {};
			}

			for (const UVoxelGraph* BaseGraph : Graph->GetBaseGraphs())
			{
				if (BaseGraph == Graph ||
					!BaseGraph->HasMainTerminalGraph())
				{
					continue;
				}

				const UEdGraph& EdGraph = BaseGraph->GetMainTerminalGraph().GetEdGraph();

				TArray<UVoxelGraphNode_Struct*> OutputNodes;
				EdGraph.GetNodesOfClass<UVoxelGraphNode_Struct>(OutputNodes);

				OutputNodes.RemoveAll([&](UVoxelGraphNode_Struct* Node)
				{
					return
						!Node ||
						!Node->Struct ||
						!Node->Struct->IsA(OutputNodeStruct);
				});

				if (!ensure(OutputNodes.Num() == 1))
				{
					continue;
				}

				return OutputNodes[0]->Struct;
			}
			return {};
		};

		INLINE_LAMBDA
		{
			if (!ParentOutputNode.IsValid())
			{
				return;
			}

			const UScriptStruct* OutputNodeStruct = Graph->GetOutputNodeStruct();
			if (!OutputNodeStruct)
			{
				return;
			}

			UEdGraph& EdGraph = TerminalGraph.GetEdGraph();
			TArray<UVoxelGraphNode_Struct*> OutputNodes;
			EdGraph.GetNodesOfClass<UVoxelGraphNode_Struct>(OutputNodes);

			OutputNodes.RemoveAll([&](UVoxelGraphNode_Struct* Node)
			{
				return
					!Node ||
					!Node->Struct ||
					!Node->Struct->IsA(OutputNodeStruct);
			});

			if (!ensure(OutputNodes.Num() == 1))
			{
				return;
			}

			UVoxelGraphNode_Struct* OutputNode = OutputNodes[0];

			const FVoxelTransaction NodeTransaction(OutputNode, "Create new function");
			OutputNode->Struct = ParentOutputNode.MakeSharedCopy<FVoxelNode>();
			OutputNode->ReconstructNode();

			UE_506_SWITCH(FVector2D, FVector2f) Position;
			Position.X = OutputNode->NodePosX - 300;
			Position.Y = OutputNode->NodePosY;

			FVoxelGraphSchemaAction_NewCallParentMainGraphNode Action;
			const UEdGraphNode* NewNode = Action.PerformAction(&EdGraph, nullptr, Position, true);

			const UEdGraphSchema* Schema = NewNode->GetSchema();
			for (UEdGraphPin* Pin : NewNode->GetAllPins())
			{
				UEdGraphPin* OutputNodePin = OutputNode->FindPin(Pin->PinName, EGPD_Input);
				if (!ensure(OutputNodePin))
				{
					continue;
				}

				Schema->TryCreateConnection(Pin, OutputNodePin);
			}
		};
	}

	Toolkit->OpenGraphAndBringToFront(Graph->GetMainTerminalGraph().GetEdGraph(), false);
	Toolkit->CloseEmptyTab();
	Toolkit->GetSelection().SelectMainGraph();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelMembersPaletteItem_MainGraph::Construct(const FArguments& InArgs, const FCreateWidgetForActionData& CreateData)
{
	ActionPtr = CreateData.Action;

	ChildSlot
	[
		SNew(SBox)
		.Padding(FMargin(0.f, -2.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.f, 2.f, 4.f, 2.f))
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				SNew(SImage)
				.Image(FVoxelEditorStyle::GetBrush("VoxelGraph.Execute"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f)
			[
				CreateTextSlotWidget(CreateData)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 1.f)
			[
				SNew(SWidgetSwitcher)
				.Visibility_Lambda([this]
				{
					const TSharedPtr<FEdGraphSchemaAction> Action = ActionPtr.Pin();
					if (!Action)
					{
						return EVisibility::Collapsed;
					}

					return
						static_cast<FVoxelMembersSchemaAction_MainGraph&>(*Action).IsOverriden()
						? EVisibility::Visible
						: EVisibility::Collapsed;
				})
				.WidgetIndex_Lambda([this]
				{
					return IsHovered() ? 0 : 1;
				})
				+ SWidgetSwitcher::Slot()
				[
					SNew(SButton)
					.ButtonStyle(FVoxelEditorStyle::Get(), "Voxel.Members.MainGraph.Implement")
					.ContentPadding(2.f)
					.ForegroundColor(FSlateColor::UseForeground())
					.Text(INVTEXT("Override"))
					.ToolTipText(INVTEXT("Override Main Graph"))
					.OnClicked_Lambda([this]
					{
						const TSharedPtr<FEdGraphSchemaAction> Action = ActionPtr.Pin();
						if (!Action)
						{
							return FReply::Handled();
						}

						static_cast<FVoxelMembersSchemaAction_MainGraph&>(*Action).CreateMainGraph();
						return FReply::Handled();
					})
				]
				+ SWidgetSwitcher::Slot()
				.Padding(10.f, 2.f)
				[
					SNew(STextBlock)
					.ColorAndOpacity(FSlateColor::UseForeground())
					.Text(INVTEXT("Override"))
				]
			]
		]
	];
}