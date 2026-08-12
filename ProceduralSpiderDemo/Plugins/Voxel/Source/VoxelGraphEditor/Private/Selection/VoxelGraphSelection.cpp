// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Selection/VoxelGraphSelection.h"
#include "VoxelGraph.h"
#include "VoxelGraphToolkit.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "VoxelTerminalGraphRuntime.h"
#include "Members/SVoxelGraphMembers.h"
#include "Customizations/VoxelGraphNode_Struct_Customization.h"
#include "Nodes/VoxelGraphNode_Struct.h"
#include "Nodes/VoxelGraphNode_Parameter.h"
#include "Nodes/VoxelGraphNode_FunctionInput.h"
#include "Nodes/VoxelGraphNode_FunctionOutput.h"
#include "Nodes/VoxelGraphNode_LocalVariable.h"
#include "Nodes/VoxelGraphNode_CallFunction.h"
#include "Selection/VoxelGraphSelectionCustomization_Parameter.h"
#include "Selection/VoxelGraphSelectionCustomization_FunctionInput.h"
#include "Selection/VoxelGraphSelectionCustomization_FunctionOutput.h"
#include "Selection/VoxelGraphSelectionCustomization_LocalVariable.h"

void FVoxelGraphSelection::SelectNone()
{
	if (Toolkit.Asset->IsFunctionLibrary())
	{
		Select(FNone{});
	}
	else
	{
		SelectMainGraph();
	}
}

void FVoxelGraphSelection::SelectCategory(
	const int32 SectionId,
	const FName DisplayName)
{
	Select(FCategory
	{
		SectionId,
		DisplayName
	});
}

void FVoxelGraphSelection::SelectMainGraph()
{
	ensure(!Toolkit.Asset->IsFunctionLibrary());
	Select(FMainGraph{});
}

void FVoxelGraphSelection::SelectEditorGraph()
{
	ensure(!Toolkit.Asset->IsFunctionLibrary());
	Select(FEditorGraph{});
}

void FVoxelGraphSelection::SelectFunction(const FGuid& Guid)
{
	ensure(Guid.IsValid());
	ensure(Guid != GVoxelMainTerminalGraphGuid);
	ensure(Guid != GVoxelEditorTerminalGraphGuid);

	Select(FFunction
	{
		Guid
	});
}

void FVoxelGraphSelection::SelectParameter(const FGuid& Guid)
{
	Select(FParameter
	{
		Guid
	});
}

void FVoxelGraphSelection::SelectFunctionInput(
	UVoxelTerminalGraph& TerminalGraph,
	const FGuid& Guid)
{
	Select(FFunctionInput
	{
		&TerminalGraph,
		Guid
	});
}

void FVoxelGraphSelection::SelectOutput(
	UVoxelTerminalGraph& TerminalGraph,
	const FGuid& Guid)
{
	Select(FOutput
	{
		&TerminalGraph,
		Guid
	});
}

void FVoxelGraphSelection::SelectLocalVariable(
	UVoxelTerminalGraph& TerminalGraph,
	const FGuid& Guid)
{
	Select(FLocalVariable
	{
		&TerminalGraph,
		Guid
	});
}

void FVoxelGraphSelection::SelectNode(UEdGraphNode* Node)
{
	Select(FNode
	{
		Node
	});
}

void FVoxelGraphSelection::SelectTerminalGraph(const UVoxelTerminalGraph& TerminalGraph)
{
	if (TerminalGraph.IsMainTerminalGraph())
	{
		SelectMainGraph();
	}
	else if (TerminalGraph.IsEditorTerminalGraph())
	{
		SelectEditorGraph();
	}
	else
	{
		SelectFunction(TerminalGraph.GetGuid());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphSelection::RequestRename() const
{
	GraphMembers->RequestRename();
}

void FVoxelGraphSelection::Update()
{
	VOXEL_FUNCTION_COUNTER();

	if (bIsUpdating)
	{
		return;
	}
	ensure(!bIsUpdating);
	bIsUpdating = true;
	ON_SCOPE_EXIT
	{
		ensure(bIsUpdating);
		bIsUpdating = false;
	};

	// Ensure Refresh is called before selecting any new member
	GVoxelGraphTracker->Flush();

	GraphMembers->SelectNone();
	SetDetailObject(nullptr);

	Visit(
		[&](const auto& Data)
		{
			this->Update(Data);
		},
		Selection);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphSelection::Update(const FNone& Data)
{
	// Nothing to do
}

void FVoxelGraphSelection::Update(const FCategory& Data)
{
	GraphMembers->SelectCategory(
		Data.SectionID,
		Data.DisplayName);
}

void FVoxelGraphSelection::Update(const FMainGraph& Data)
{
	GraphMembers->SelectMember(
		EVoxelGraphMemberSection::Graphs,
		GVoxelMainTerminalGraphGuid);

	SetDetailObject(Toolkit.Asset);
}

void FVoxelGraphSelection::Update(const FEditorGraph& Data)
{
	GraphMembers->SelectMember(
		EVoxelGraphMemberSection::Graphs,
		GVoxelEditorTerminalGraphGuid);

	SetDetailObject(&Toolkit.Asset->GetEditorTerminalGraph());
}

void FVoxelGraphSelection::Update(const FFunction& Data)
{
	const FGuid Guid = Data.Guid;
	ensure(Guid.IsValid());
	ensure(Guid != GVoxelMainTerminalGraphGuid);
	ensure(Guid != GVoxelEditorTerminalGraphGuid);

	UVoxelTerminalGraph* TerminalGraph = Toolkit.Asset->FindTerminalGraph_NoInheritance(Guid);
	if (!ensureVoxelSlow(TerminalGraph))
	{
		// Function was deleted
		return;
	}

	GraphMembers->SelectMember(
		EVoxelGraphMemberSection::Functions,
		Guid);

	SetDetailObject(TerminalGraph);
}

void FVoxelGraphSelection::Update(const FParameter& Data)
{
	UVoxelGraph* Graph = Toolkit.Asset;
	if (!ensure(Graph))
	{
		return;
	}

	const FGuid Guid = Data.Guid;
	ensure(Guid.IsValid());

	if (!ensureVoxelSlow(Graph->FindParameter(Guid)))
	{
		// Parameter was deleted
		return;
	}

	GraphMembers->SelectMember(
		EVoxelGraphMemberSection::Parameters,
		Guid);

	SetDetailObject(
		Graph,
		Graph->IsInheritedParameter(Guid),
		FVoxelGraphSelectionCustomization_Parameter(Guid));

}

void FVoxelGraphSelection::Update(const FFunctionInput& Data)
{
	UVoxelTerminalGraph* TerminalGraph = Data.TerminalGraph.Resolve();
	if (!ensureVoxelSlow(TerminalGraph))
	{
		// Graph was deleted
		return;
	}

	const FGuid Guid = Data.Guid;
	ensure(Guid.IsValid());

	if (!ensureVoxelSlow(TerminalGraph->FindInput(Guid)))
	{
		// Function input was deleted
		return;
	}

	if (Toolkit.GetActiveEdGraph() != &TerminalGraph->GetEdGraph())
	{
		return;
	}

	GraphMembers->SelectMember(
		EVoxelGraphMemberSection::FunctionInputs,
		Guid);

	SetDetailObject(
		TerminalGraph,
		TerminalGraph->IsInheritedInput(Guid),
		FVoxelGraphSelectionCustomization_FunctionInput(Guid, TerminalGraph->GetRuntime().HasFunctionInputDefault_EditorOnly(Guid)));

}

void FVoxelGraphSelection::Update(const FOutput& Data)
{
	UVoxelTerminalGraph* TerminalGraph = Data.TerminalGraph.Resolve();
	if (!ensureVoxelSlow(TerminalGraph))
	{
		// Graph was deleted
		return;
	}

	const FGuid Guid = Data.Guid;
	ensure(Guid.IsValid());

	if (!ensureVoxelSlow(TerminalGraph->FindOutput(Guid)))
	{
		// Output was deleted
		return;
	}

	if (Toolkit.GetActiveEdGraph() != &TerminalGraph->GetEdGraph())
	{
		return;
	}

	GraphMembers->SelectMember(
		EVoxelGraphMemberSection::FunctionOutputs,
		Guid);

	SetDetailObject(
		TerminalGraph,
		TerminalGraph->IsInheritedOutput(Guid),
		FVoxelGraphSelectionCustomization_FunctionOutput(Guid));

}

void FVoxelGraphSelection::Update(const FLocalVariable& Data)
{
	UVoxelTerminalGraph* TerminalGraph = Data.TerminalGraph.Resolve();
	if (!ensureVoxelSlow(TerminalGraph))
	{
		// Graph was deleted
		return;
	}

	const FGuid Guid = Data.Guid;
	ensure(Guid.IsValid());

	if (!ensureVoxelSlow(TerminalGraph->FindLocalVariable(Guid)))
	{
		// Local variable was deleted
		return;
	}

	if (Toolkit.GetActiveEdGraph() != &TerminalGraph->GetEdGraph())
	{
		return;
	}

	GraphMembers->SelectMember(
		EVoxelGraphMemberSection::LocalVariables,
		Guid);

	SetDetailObject(
		TerminalGraph,
		false,
		FVoxelGraphSelectionCustomization_LocalVariable(Guid));
}

void FVoxelGraphSelection::Update(const FNode& Data)
{
	UVoxelGraph* Graph = Toolkit.Asset;
	if (!ensure(Graph))
	{
		return;
	}

	UEdGraphNode* Node = Data.Node.Resolve();
	if (!ensureVoxelSlow(Node))
	{
		// Node was deleted
		return;
	}

	if (Toolkit.GetActiveEdGraph() != Node->GetGraph())
	{
		return;
	}

	if (Node->IsA<UVoxelGraphNode_Struct>())
	{
		SetDetailObject(
			Node,
			false,
			FVoxelGraphNode_Struct_Customization());

		return;
	}

	if (const UVoxelGraphNode_Parameter* ParameterNode = Cast<UVoxelGraphNode_Parameter>(Node))
	{
		const FGuid Guid = ParameterNode->Guid;
		if (!Guid.IsValid() ||
			!ensure(Graph->FindParameter(Guid)))
		{
			return;
		}

		GraphMembers->SelectMember(EVoxelGraphMemberSection::Parameters, Guid);

		SetDetailObject(
			Graph,
			Graph->IsInheritedParameter(Guid),
			FVoxelGraphSelectionCustomization_Parameter(Guid));

		return;
	}

	if (UVoxelGraphNode_FunctionInputBase* InputNode = Cast<UVoxelGraphNode_FunctionInputBase>(Node))
	{
		UVoxelTerminalGraph* TerminalGraph = InputNode->GetTerminalGraph();
		if (!ensure(TerminalGraph))
		{
			return;
		}

		const FGuid Guid = InputNode->Guid;
		if (!Guid.IsValid() ||
			!ensure(TerminalGraph->FindInput(Guid)))
		{
			return;
		}

		GraphMembers->SelectMember(EVoxelGraphMemberSection::FunctionInputs, Guid);

		SetDetailObject(
			TerminalGraph,
			TerminalGraph->IsInheritedInput(Guid),
			FVoxelGraphSelectionCustomization_FunctionInput(Guid, TerminalGraph->GetRuntime().HasFunctionInputDefault_EditorOnly(Guid)));

		return;
	}

	if (const UVoxelGraphNode_FunctionOutput* OutputNode = Cast<UVoxelGraphNode_FunctionOutput>(Node))
	{
		UVoxelTerminalGraph* TerminalGraph = OutputNode->GetTypedOuter<UVoxelTerminalGraph>();
		if (!ensure(TerminalGraph))
		{
			return;
		}

		const FGuid Guid = OutputNode->Guid;
		if (!Guid.IsValid() ||
			!ensure(TerminalGraph->FindOutput(Guid)))
		{
			return;
		}

		GraphMembers->SelectMember(EVoxelGraphMemberSection::FunctionOutputs, Guid);

		SetDetailObject(
			TerminalGraph,
			TerminalGraph->IsInheritedOutput(Guid),
			FVoxelGraphSelectionCustomization_FunctionOutput(Guid));

		return;
	}

	if (const UVoxelGraphNode_LocalVariable* LocalVariable = Cast<UVoxelGraphNode_LocalVariable>(Node))
	{
		UVoxelTerminalGraph* TerminalGraph = LocalVariable->GetTypedOuter<UVoxelTerminalGraph>();
		if (!ensure(TerminalGraph))
		{
			return;
		}

		const FGuid Guid = LocalVariable->Guid;
		if (!Guid.IsValid() ||
			!ensure(TerminalGraph->FindLocalVariable(Guid)))
		{
			return;
		}

		GraphMembers->SelectMember(EVoxelGraphMemberSection::LocalVariables, Guid);

		SetDetailObject(
			TerminalGraph,
			false,
			FVoxelGraphSelectionCustomization_LocalVariable(Guid));

		return;
	}

	if (const UVoxelGraphNode_CallMemberFunction* FunctionNode = Cast<UVoxelGraphNode_CallMemberFunction>(Node))
	{
		if (!ensure(FunctionNode->GetTypedOuter<UVoxelGraph>() == Graph))
		{
			return;
		}

		const FGuid Guid = FunctionNode->Guid;
		if (!Guid.IsValid())
		{
			return;
		}

		UVoxelTerminalGraph* FunctionGraph = Graph->FindTerminalGraph(Guid);
		if (!ensure(FunctionGraph))
		{
			return;
		}

		if (Graph->FindTerminalGraph_NoInheritance(Guid))
		{
			GraphMembers->SelectMember(EVoxelGraphMemberSection::Functions, Guid);
		}

		bool bIsReadOnly = false;
		if (Graph->GetBaseGraph_Unsafe() &&
			Graph->GetBaseGraph_Unsafe()->FindTerminalGraph(Guid))
		{
			// Inherited
			bIsReadOnly = true;
		}

		SetDetailObject(FunctionGraph, bIsReadOnly);

		return;
	}

	SetDetailObject(Node);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void FVoxelGraphSelection::Select(const T& Value)
{
	if (bIsSelecting)
	{
		// Flushing graph tracker might trigger additional selections through SVoxelGraphMembers::Refresh
		return;
	}

	// Ensure there's no leftover values
	Selection = {};
	Selection.Set<T>(Value);

	if (FVoxelUtilities::Equal(
		MakeByteVoxelArrayView(Selection),
		MakeByteVoxelArrayView(LastSelection)))
	{
		return;
	}
	LastSelection = Selection;

	ensure(!bIsSelecting);
	bIsSelecting = true;
	{
		Update();
	}
	ensure(bIsSelecting);
	bIsSelecting = false;
}

template<typename T>
void FVoxelGraphSelection::SetDetailObject(
	UObject* Object,
	const bool bIsReadOnly,
	T Customization)
{
	DetailsView->SetIsPropertyEditingEnabledDelegate(MakeLambdaDelegate([=]
	{
		return !bIsReadOnly;
	}));

	if constexpr (std::is_same_v<decltype(Customization), decltype(nullptr)>)
	{
		DetailsView->SetGenericLayoutDetailsDelegate(nullptr);
	}
	else
	{
		DetailsView->SetGenericLayoutDetailsDelegate(MakeLambdaDelegate([=]() -> TSharedRef<IDetailCustomization>
		{
			return MakeSharedCopy(Customization);
		}));
	}

	DetailsView->SetObject(Object, true);
}