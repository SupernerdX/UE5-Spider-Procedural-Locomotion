// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphNode_CallParentMainGraph.h"
#include "VoxelGraph.h"
#include "VoxelEdGraph.h"
#include "VoxelGraphToolkit.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphNode_Struct.h"
#include "Nodes/VoxelOutputNode.h"

void UVoxelGraphNode_CallParentMainGraph::AllocateDefaultPins()
{
	VOXEL_FUNCTION_COUNTER();

	ON_SCOPE_EXIT
	{
		Super::AllocateDefaultPins();
	};

	const UVoxelTerminalGraph* BaseTerminalGraph = GetBaseTerminalGraph();

	OnChangedPtr = MakeSharedVoid();
	WeakOutputNode = nullptr;

	// If the main terminal graph is removed or readded, reconstruct the node
	if (const UVoxelGraph* Graph = GetTypedOuter<UVoxelGraph>())
	{
		for (const UVoxelGraph* BaseGraph : Graph->GetBaseGraphs())
		{
			GVoxelGraphTracker->OnTerminalGraphChanged(*BaseGraph).Add(FOnVoxelGraphChanged::Make(OnChangedPtr, this, [this]
			{
				ReconstructNode();
			}));
		}
	}

	if (!BaseTerminalGraph)
	{
		return;
	}

	GVoxelGraphTracker->OnTerminalGraphMetaDataChanged(*BaseTerminalGraph).Add(FOnVoxelGraphChanged::Make(OnChangedPtr, this, [this]
	{
		RefreshNode();
	}));

	TArray<UVoxelGraphNode_Struct*> OutputNodes;
	BaseTerminalGraph->GetEdGraph().GetNodesOfClass<UVoxelGraphNode_Struct>(OutputNodes);

	const UScriptStruct* OutputNodeStruct = BaseTerminalGraph->GetGraph().GetOutputNodeStruct();
	if (!OutputNodeStruct)
	{
		return;
	}

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
	for (const UEdGraphPin* Pin : OutputNode->Pins)
	{
		UEdGraphPin* GraphPin = CreatePin(
			EGPD_Output,
			Pin->PinType,
			Pin->PinName);

		GraphPin->PinFriendlyName = Pin->PinFriendlyName;
		GraphPin->PinToolTip = Pin->PinToolTip;
		GraphPin->bAdvancedView = Pin->bAdvancedView;
	}

	// Reconstruct node if parents output node changes
	GVoxelGraphTracker->OnEdGraphNodeChanged(*OutputNode).Add(FOnVoxelGraphChanged::Make(OnChangedPtr, this, [this]
	{
		ReconstructNode();
	}));

	WeakOutputNode = OutputNode;
}

FText UVoxelGraphNode_CallParentMainGraph::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	const UVoxelTerminalGraph* BaseTerminalGraph = GetBaseTerminalGraph();
	if (!BaseTerminalGraph)
	{
		return INVTEXT("Empty");
	}

	return FText::FromString("Parent: " + BaseTerminalGraph->GetDisplayName());
}

FLinearColor UVoxelGraphNode_CallParentMainGraph::GetNodeTitleColor() const
{
	return GetDefault<UGraphEditorSettings>()->ParentFunctionCallNodeTitleColor;
}

FText UVoxelGraphNode_CallParentMainGraph::GetTooltipText() const
{
	const UVoxelTerminalGraph* BaseTerminalGraph = GetBaseTerminalGraph();
	if (!BaseTerminalGraph)
	{
		return {};
	}

	return FText::FromString(BaseTerminalGraph->GetMetadata().Description);
}

FSlateIcon UVoxelGraphNode_CallParentMainGraph::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon("EditorStyle", "GraphEditor.Function_16x");
}

void UVoxelGraphNode_CallParentMainGraph::JumpToDefinition() const
{
	const UVoxelTerminalGraph* BaseTerminalGraph = GetBaseTerminalGraph();
	if (!BaseTerminalGraph)
	{
		return;
	}

	FVoxelUtilities::FocusObject(BaseTerminalGraph);
}

FName UVoxelGraphNode_CallParentMainGraph::GetPinCategory(const UEdGraphPin& Pin) const
{
	UVoxelGraphNode_Struct* OutputNode = WeakOutputNode.Get();
	if (!OutputNode ||
		!OutputNode->Struct)
	{
		return {};
	}

	const TSharedPtr<const FVoxelPin> StructPin = OutputNode->Struct->FindPin(Pin.PinName);
	if (!ensure(StructPin))
	{
		return {};
	}

	return *StructPin->Metadata.Category;
}

TSharedRef<IVoxelNodeDefinition> UVoxelGraphNode_CallParentMainGraph::GetNodeDefinition()
{
	return MakeShared<FVoxelNodeDefinition_CallParentMainGraph>(*this);
}

bool UVoxelGraphNode_CallParentMainGraph::CanPasteHere(const UEdGraph* TargetGraph) const
{
	return false;
}

const UVoxelTerminalGraph* UVoxelGraphNode_CallParentMainGraph::GetBaseTerminalGraph() const
{
	const UVoxelGraph* Graph = GetTypedOuter<UVoxelGraph>();
	if (!ensure(Graph))
	{
		return nullptr;
	}

	for (const UVoxelGraph* BaseGraph : Graph->GetBaseGraphs())
	{
		if (BaseGraph == Graph ||
			!BaseGraph->HasMainTerminalGraph())
		{
			continue;
		}

		return &BaseGraph->GetMainTerminalGraph();
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<const IVoxelNodeDefinition::FNode> FVoxelNodeDefinition_CallParentMainGraph::GetOutputs() const
{
	UVoxelGraphNode_Struct* OutputNode = Node.WeakOutputNode.Get();
	if (!OutputNode ||
		!OutputNode->Struct)
	{
		return nullptr;
	}

	return OutputNode->GetNodeDefinition()->GetInputs();
}