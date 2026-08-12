// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphEditorInterface.h"
#include "VoxelGraph.h"
#include "VoxelEdGraph.h"
#include "VoxelGraphSchema.h"
#include "VoxelNodeLibrary.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "Nodes/VoxelGraphNode_Knot.h"
#include "Nodes/VoxelGraphNode_FunctionInput.h"
#include "Nodes/VoxelGraphNode_FunctionOutput.h"
#include "Nodes/VoxelGraphNode_Struct.h"
#include "Nodes/VoxelGraphNode_Parameter.h"
#include "Nodes/VoxelGraphNode_CallFunction.h"
#include "Nodes/VoxelGraphNode_LocalVariable.h"
#include "Nodes/VoxelGraphNode_CustomizeParameter.h"
#include "Nodes/VoxelFunctionInputNodes.h"
#include "Nodes/VoxelNode_FunctionOutput.h"
#include "Nodes/VoxelNode_Parameter.h"
#include "Nodes/VoxelNode_CustomizeParameter.h"
#include "Nodes/VoxelCallFunctionNodes.h"
#include "Nodes/VoxelLocalVariableNodes.h"
#include "Serialization/ObjectReader.h"
#include "Serialization/ObjectWriter.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Nodes/VoxelGraphNode_CallParentMainGraph.h"

VOXEL_RUN_ON_STARTUP_EDITOR_COMMANDLET()
{
	check(!GVoxelGraphEditorInterface);
	GVoxelGraphEditorInterface = new FVoxelGraphEditorInterface();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorInterface::CompileAll()
{
	VOXEL_FUNCTION_COUNTER();

	UVoxelGraph::LoadAllGraphs();

	TArray<FSoftObjectPath> AssetsToOpen;
	TVoxelChunkedArray<UVoxelGraph*> Objects;
	ForEachObjectOfClass<UVoxelGraph>([&](UVoxelGraph& Object)
	{
		Objects.Add(&Object);
	});

	{
		FScopedSlowTask SlowTask(Objects.Num(), INVTEXT("Recompiling graphs..."));
		SlowTask.MakeDialog();

		for (UVoxelGraph* Graph : Objects)
		{
			bool bShouldOpen = false;

			Graph->ForeachTerminalGraph_NoInheritance([&](const UVoxelTerminalGraph& TerminalGraph)
			{
				UVoxelTerminalGraphRuntime& Runtime = TerminalGraph.GetRuntime();
				Runtime.EnsureIsCompiled();

				if (Runtime.HasCompileMessages())
				{
					bShouldOpen = true;
				}
			});

			if (bShouldOpen)
			{
				AssetsToOpen.Add(Graph);
			}
		}
	}

	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorsForAssets(AssetsToOpen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorInterface::ReconstructAllNodes(UVoxelTerminalGraph& TerminalGraph)
{
	VOXEL_FUNCTION_COUNTER();

	TerminalGraph.GetTypedEdGraph().MigrateAndReconstructAll();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelGraphEditorInterface::HasNode(const UVoxelTerminalGraph& TerminalGraph, const UScriptStruct* Struct)
{
	VOXEL_FUNCTION_COUNTER();

	// Important: don't migrate, we can end up calling HasExecNodes recursively otherwise

	for (const UEdGraphNode* EdGraphNode : TerminalGraph.GetEdGraph().Nodes)
	{
		const UVoxelGraphNode* Node = Cast<UVoxelGraphNode>(EdGraphNode);
		if (!Node ||
			Node->IsA<UVoxelGraphNode_Knot>())
		{
			continue;
		}

		if (const UVoxelGraphNode_Struct* GraphNode = Cast<UVoxelGraphNode_Struct>(EdGraphNode))
		{
			if (GraphNode->Struct.IsA(Struct))
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_CallMemberFunction>())
		{
			if (Struct == StaticStructFast<FVoxelNode_CallMemberFunction>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_CallExternalFunction>())
		{
			if (Struct == StaticStructFast<FVoxelNode_CallExternalFunction>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_CallParentMainGraph>())
		{
			if (Struct == StaticStructFast<FVoxelNode_CallParentMainGraph>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_Parameter>())
		{
			if (Struct == StaticStructFast<FVoxelNode_Parameter>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_CustomizeParameter>())
		{
			if (Struct == StaticStructFast<FVoxelNode_CustomizeParameter>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_FunctionInput>())
		{
			if (Struct == StaticStructFast<FVoxelNode_FunctionInput>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_FunctionInputDefault>())
		{
			if (Struct == StaticStructFast<FVoxelNode_FunctionInputDefault>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_FunctionInputPreview>())
		{
			if (Struct == StaticStructFast<FVoxelNode_FunctionInputPreview>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_FunctionOutput>())
		{
			if (Struct == StaticStructFast<FVoxelNode_FunctionOutput>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_LocalVariableDeclaration>())
		{
			if (Struct == StaticStructFast<FVoxelNode_LocalVariableDeclaration>())
			{
				return true;
			}
			continue;
		}

		if (EdGraphNode->IsA<UVoxelGraphNode_LocalVariableUsage>())
		{
			if (Struct == StaticStructFast<FVoxelNode_LocalVariableUsage>())
			{
				return true;
			}
			continue;
		}

		ensure(false);
	}

	return false;
}

bool FVoxelGraphEditorInterface::HasFunctionInputDefault(const UVoxelTerminalGraph& TerminalGraph, const FGuid& Guid)
{
	VOXEL_FUNCTION_COUNTER();

	for (const UEdGraphNode* EdGraphNode : TerminalGraph.GetEdGraph().Nodes)
	{
		const UVoxelGraphNode_FunctionInputDefault* InputDefaultNode = Cast<UVoxelGraphNode_FunctionInputDefault>(EdGraphNode);
		if (!InputDefaultNode)
		{
			continue;
		}

		if (InputDefaultNode->Guid == Guid)
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UEdGraph* FVoxelGraphEditorInterface::CreateEdGraph(UVoxelTerminalGraph& TerminalGraph)
{
	UVoxelEdGraph* EdGraph = NewObject<UVoxelEdGraph>(&TerminalGraph, NAME_None, RF_Transactional);
	EdGraph->SetLatestVersion();
	EdGraph->Schema = UVoxelGraphSchema::StaticClass();
	return EdGraph;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSerializedGraph FVoxelGraphEditorInterface::Translate(UVoxelTerminalGraph& TerminalGraph)
{
	VOXEL_FUNCTION_COUNTER();

	UVoxelEdGraph& EdGraph = TerminalGraph.GetTypedEdGraph();
	EdGraph.MigrateIfNeeded();

	// Fixup reroute nodes
	for (UEdGraphNode* GraphNode : EdGraph.Nodes)
	{
		if (UVoxelGraphNode_Knot* Node = Cast<UVoxelGraphNode_Knot>(GraphNode))
		{
			Node->PropagatePinType();
		}
	}

	// Sanitize nodes, we've had cases where some nodes are null
	EdGraph.Nodes.RemoveAllSwap([](const UEdGraphNode* Node)
	{
		return !IsValid(Node);
	});

	TVoxelArray<const UVoxelGraphNode*> Nodes;
	for (const UEdGraphNode* EdGraphNode : EdGraph.Nodes)
	{
		const UVoxelGraphNode* Node = Cast<UVoxelGraphNode>(EdGraphNode);
		if (!Node ||
			Node->IsA<UVoxelGraphNode_Knot>())
		{
			continue;
		}
		Nodes.Add(Node);

		for (const UEdGraphPin* Pin : Node->Pins)
		{
			const auto GetError = [&]
			{
				return
					Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString() +
					" " +
					Node->GetPathName();
			};

			if (!ensureAlwaysMsgf(Pin, TEXT("Invalid pin on %s"), *GetError()))
			{
				return {};
			}

			if (Pin->ParentPin)
			{
				if (!ensureAlwaysMsgf(Pin->ParentPin->SubPins.Contains(Pin), TEXT("Invalid sub-pin on %s"), *GetError()))
				{
					return {};
				}
			}

			for (const UEdGraphPin* SubPin : Pin->SubPins)
			{
				if (!ensureAlwaysMsgf(SubPin->ParentPin == Pin, TEXT("Invalid parent pin on %s"), *GetError()))
				{
					return {};
				}
			}

			for (const UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				if (!ensureAlwaysMsgf(LinkedPin, TEXT("Invalid pin linked to %s"), *GetError()))
				{
					return {};
				}

				if (!ensureAlwaysMsgf(LinkedPin->LinkedTo.Contains(Pin), TEXT("Invalid link on %s"), *GetError()))
				{
					return {};
				}
			}
		}
	}

	TVoxelSet<FGuid> VisitedNodesGuids;

	FVoxelSerializedGraph SerializedGraph;
	SerializedGraph.Version = FVoxelSerializedGraph::FVersion::LatestVersion;

	for (const UEdGraphNode* EdGraphNode : Nodes)
	{
		FVoxelSerializedNode SerializedNode;
		SerializedNode.VoxelNode = INLINE_LAMBDA -> TVoxelInstancedStruct<FVoxelNode>
		{
			if (const UVoxelGraphNode_Struct* GraphNode = Cast<UVoxelGraphNode_Struct>(EdGraphNode))
			{
				if (!GraphNode->Struct)
				{
					return {};
				}

				if (!GraphNode->Struct->NodeGuid.IsValid())
				{
					ConstCast(GraphNode)->Modify();
					ConstCast(GraphNode)->Struct->NodeGuid = FGuid::NewGuid();
				}

				while (!VisitedNodesGuids.TryAdd(GraphNode->Struct->NodeGuid))
				{
					ConstCast(GraphNode)->Modify();
					ConstCast(GraphNode)->Struct->NodeGuid = FGuid::NewGuid();
				}

				return GraphNode->Struct;
			}

			if (const UVoxelGraphNode_CallMemberFunction* GraphNode = Cast<UVoxelGraphNode_CallMemberFunction>(EdGraphNode))
			{
				FVoxelNode_CallMemberFunction Node;
				Node.Guid = GraphNode->Guid;

				if (GraphNode->bCallParent)
				{
					UVoxelGraph* BaseGraph = TerminalGraph.GetGraph().GetBaseGraph_Unsafe();
					if (!BaseGraph)
					{
						SerializedNode.Errors.Add("Invalid function call: Base Graph is null");
					}
					Node.ContextOverride = BaseGraph;
				}

				Node.FixupPins(TerminalGraph.GetGraph(), FOnVoxelGraphChanged::Null(), FOnVoxelGraphChanged::Null());
				return Node;
			}

			if (const UVoxelGraphNode_CallExternalFunction* GraphNode = Cast<UVoxelGraphNode_CallExternalFunction>(EdGraphNode))
			{
				FVoxelNode_CallExternalFunction Node;
				Node.FunctionLibrary = GraphNode->FunctionLibrary;
				Node.Guid = GraphNode->Guid;
				Node.FixupPins(TerminalGraph.GetGraph(), FOnVoxelGraphChanged::Null(), FOnVoxelGraphChanged::Null());
				return Node;
			}

			if (EdGraphNode->IsA<UVoxelGraphNode_CallParentMainGraph>())
			{
				FVoxelNode_CallParentMainGraph Node;

				UVoxelGraph* CurrentGraph = &TerminalGraph.GetGraph();
				UVoxelGraph* BaseGraph = nullptr;
				for (UVoxelGraph* Graph : TerminalGraph.GetGraph().GetBaseGraphs())
				{
					if (Graph == CurrentGraph ||
						!Graph->HasMainTerminalGraph())
					{
						continue;
					}

					BaseGraph = Graph;
					break;
				}

				if (!BaseGraph)
				{
					SerializedNode.Errors.Add("Invalid call to parent main graph: Base Graph is null");
				}
				Node.ContextOverride = BaseGraph;

				Node.FixupPins(TerminalGraph.GetGraph(), FOnVoxelGraphChanged::Null(), FOnVoxelGraphChanged::Null());
				return Node;
			}

			if (const UVoxelGraphNode_Parameter* GraphNode = Cast<UVoxelGraphNode_Parameter>(EdGraphNode))
			{
				const FVoxelParameter Parameter = GraphNode->GetParameterSafe();

				FVoxelNode_Parameter Node;
				Node.ParameterGuid = GraphNode->Guid;
				Node.ParameterName = Parameter.Name;
				Node.GetPin(Node.ValuePin).SetType(Parameter.Type);
				return Node;
			}

			if (const UVoxelGraphNode_CustomizeParameter* GraphNode = Cast<UVoxelGraphNode_CustomizeParameter>(EdGraphNode))
			{
				const FVoxelParameter Parameter = GraphNode->GetParameterSafe();

				FVoxelNode_CustomizeParameter Node;
				Node.ParameterGuid = GraphNode->Guid;
				Node.ParameterName = Parameter.Name;
				return Node;
			}

			if (const UVoxelGraphNode_FunctionInput* GraphNode = Cast<UVoxelGraphNode_FunctionInput>(EdGraphNode))
			{
				const FVoxelGraphFunctionInput Input = GraphNode->GetInputSafe();

				FVoxelNode_FunctionInput Node;
				Node.Guid = GraphNode->Guid;
				Node.GetPin(Node.ValuePin).SetType(Input.Type);
				return Node;
			}

			if (const UVoxelGraphNode_FunctionInputDefault* GraphNode = Cast<UVoxelGraphNode_FunctionInputDefault>(EdGraphNode))
			{
				const FVoxelGraphFunctionInput Input = GraphNode->GetInputSafe();

				FVoxelNode_FunctionInputDefault Node;
				Node.Guid = GraphNode->Guid;
				Node.GetPin(Node.DefaultPin).SetType(Input.Type);
				return Node;
			}

			if (const UVoxelGraphNode_FunctionInputPreview* GraphNode = Cast<UVoxelGraphNode_FunctionInputPreview>(EdGraphNode))
			{
				const FVoxelGraphFunctionInput Input = GraphNode->GetInputSafe();

				FVoxelNode_FunctionInputPreview Node;
				Node.Guid = GraphNode->Guid;
				Node.GetPin(Node.PreviewPin).SetType(Input.Type);
				return Node;
			}

			if (const UVoxelGraphNode_FunctionOutput* GraphNode = Cast<UVoxelGraphNode_FunctionOutput>(EdGraphNode))
			{
				// Add that output to the graph so we know to query it
				SerializedGraph.OutputGuids.Add(GraphNode->Guid);

				const FVoxelGraphFunctionOutput Output = GraphNode->GetOutputSafe();

				FVoxelNode_FunctionOutput Node;
				Node.Guid = GraphNode->Guid;
				Node.GetPin(Node.ValuePin).SetType(Output.Type);
				return Node;
			}

			if (const UVoxelGraphNode_LocalVariableDeclaration* GraphNode = Cast<UVoxelGraphNode_LocalVariableDeclaration>(EdGraphNode))
			{
				const FVoxelGraphLocalVariable LocalVariable = GraphNode->GetLocalVariableSafe();

				FVoxelNode_LocalVariableDeclaration Node;
				Node.Guid = GraphNode->Guid;
				Node.GetPin(Node.InputPinPin).SetType(LocalVariable.Type);
				Node.GetPin(Node.OutputPinPin).SetType(LocalVariable.Type);
				return Node;
			}

			if (const UVoxelGraphNode_LocalVariableUsage* GraphNode = Cast<UVoxelGraphNode_LocalVariableUsage>(EdGraphNode))
			{
				const FVoxelGraphLocalVariable LocalVariable = GraphNode->GetLocalVariableSafe();

				FVoxelNode_LocalVariableUsage Node;
				Node.Guid = GraphNode->Guid;
				Node.GetPin(Node.OutputPinPin).SetType(LocalVariable.Type);
				return Node;
			}

			ensure(false);
			return {};
		};

		if (SerializedNode.VoxelNode)
		{
			SerializedNode.StructName = SerializedNode.VoxelNode.GetScriptStruct()->GetFName();
			if (!TerminalGraph.IsFunction() &&
				TerminalGraph.IsMainTerminalGraph() &&
				TerminalGraph.GetGraph().GetOutputNodeStruct() &&
				SerializedNode.VoxelNode.IsA(TerminalGraph.GetGraph().GetOutputNodeStruct()))
			{
				ensure(SerializedGraph.OutputNodeName.IsNone());
				SerializedGraph.OutputNodeName = EdGraphNode->GetFName();
			}
		}

		SerializedNode.EdGraphNodeName = EdGraphNode->GetFName();
		SerializedNode.EdGraphNodeTitle = *EdGraphNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString();

		ensure(!SerializedGraph.NodeNameToNode.Contains(SerializedNode.EdGraphNodeName));
		SerializedGraph.NodeNameToNode.Add(SerializedNode.EdGraphNodeName, SerializedNode);
	}

	TMap<const UEdGraphPin*, FVoxelSerializedPinRef> EdGraphPinToPinRef;
	for (const UVoxelGraphNode* EdGraphNode : Nodes)
	{
		FVoxelSerializedNode& SerializedNode = SerializedGraph.NodeNameToNode[EdGraphNode->GetFName()];

		const auto MakePin = [&](const UEdGraphPin* EdGraphPin)
		{
			ensure(!EdGraphPin->bOrphanedPin);

			FVoxelSerializedPin SerializedPin;
			SerializedPin.Type = EdGraphPin->PinType;
			SerializedPin.PinName = EdGraphPin->PinName;

			for (const UEdGraphPin* Pin = EdGraphPin->ParentPin; Pin; Pin = Pin->ParentPin)
			{
				SerializedPin.PinName = Pin->PinName.ToString() + "|" + SerializedPin.PinName;
			}

			if (const UEdGraphPin* ParentPin = EdGraphPin->ParentPin)
			{
				const FVoxelPinType Type(ParentPin->PinType);
				if (ParentPin->Direction == EGPD_Input)
				{
					SerializedGraph.TypeToMakeNode.Add(Type);
				}
				else
				{
					check(ParentPin->Direction == EGPD_Output);
					SerializedGraph.TypeToBreakNode.Add(Type);
				}

				SerializedPin.ParentPinName = ParentPin->PinName;
			}

			if (SerializedPin.Type.HasPinDefaultValue())
			{
				SerializedPin.DefaultValue = FVoxelPinValue::MakeFromPinDefaultValue(*EdGraphPin);
			}
			else
			{
				ensureVoxelSlow(!EdGraphPin->DefaultObject);
				ensureVoxelSlow(EdGraphPin->DefaultValue.IsEmpty());
				ensureVoxelSlow(EdGraphPin->AutogeneratedDefaultValue.IsEmpty());
			}

			return SerializedPin;
		};

		for (const UEdGraphPin* Pin : EdGraphNode->GetInputPins())
		{
			if (Pin->bOrphanedPin)
			{
				SerializedNode.Errors.Add("Orphaned pin " + Pin->GetDisplayName().ToString());
				continue;
			}
			if (!ensureVoxelSlow(FVoxelPinType(Pin->PinType).IsValid()))
			{
				SerializedNode.Errors.Add("Pin with invalid type " + Pin->GetDisplayName().ToString());
				continue;
			}

			if (!ensure(!SerializedNode.InputPins.Contains(Pin->PinName)))
			{
				continue;
			}

			FVoxelSerializedPin NewPin = MakePin(Pin);
			SerializedNode.InputPins.Add(NewPin.PinName, NewPin);
			EdGraphPinToPinRef.Add(Pin, FVoxelSerializedPinRef{ EdGraphNode->GetFName(), NewPin.PinName, true });
		}
		for (const UEdGraphPin* Pin : EdGraphNode->GetOutputPins())
		{
			if (Pin->bOrphanedPin)
			{
				SerializedNode.Errors.Add("Orphaned pin " + Pin->GetDisplayName().ToString());
				continue;
			}
			if (!ensureVoxelSlow(FVoxelPinType(Pin->PinType).IsValid()))
			{
				SerializedNode.Errors.Add("Pin with invalid type " + Pin->GetDisplayName().ToString());
				continue;
			}

			if (!ensure(!SerializedNode.OutputPins.Contains(Pin->PinName)))
			{
				continue;
			}

			FVoxelSerializedPin NewPin = MakePin(Pin);
			SerializedNode.OutputPins.Add(NewPin.PinName, NewPin);
			EdGraphPinToPinRef.Add(Pin, FVoxelSerializedPinRef{ EdGraphNode->GetFName(), NewPin.PinName, false });
		}
	}

	for (auto& It : SerializedGraph.TypeToMakeNode)
	{
		const TSharedPtr<const FVoxelNode> Node = GVoxelNodeLibrary->FindMakeNode(It.Key);
		if (!ensure(Node))
		{
			continue;
		}

		It.Value = FVoxelInstancedStruct::Make(*Node);
	}
	for (auto& It : SerializedGraph.TypeToBreakNode)
	{
		const TSharedPtr<const FVoxelNode> Node = GVoxelNodeLibrary->FindBreakNode(It.Key);
		if (!ensure(Node))
		{
			continue;
		}

		It.Value = FVoxelInstancedStruct::Make(*Node);
	}

	// Link the pins once they're all allocated
	for (const UVoxelGraphNode* EdGraphNode : Nodes)
	{
		FVoxelSerializedNode* SerializedNode = SerializedGraph.NodeNameToNode.Find(EdGraphNode->GetFName());
		if (!ensure(SerializedNode))
		{
			continue;
		}

		const auto AddPin = [&](const UEdGraphPin* Pin)
		{
			if (!ensure(EdGraphPinToPinRef.Contains(Pin)))
			{
				return false;
			}

			FVoxelSerializedPin* SerializedPin = SerializedGraph.FindPin(EdGraphPinToPinRef[Pin]);
			if (!ensure(SerializedPin))
			{
				return false;
			}

			for (const UEdGraphPin* LinkedTo : Pin->LinkedTo)
			{
				if (LinkedTo->bOrphanedPin)
				{
					if (Pin->Direction == EGPD_Input)
					{
						SerializedNode->Errors.Add(Pin->GetDisplayName().ToString() + " is connected to an orphaned pin");
					}
					continue;
				}

				if (UVoxelGraphNode_Knot* Knot = Cast<UVoxelGraphNode_Knot>(LinkedTo->GetOwningNode()))
				{
					for (const UEdGraphPin* LinkedToKnot : Knot->GetLinkedPins(Pin->Direction))
					{
						if (LinkedToKnot->bOrphanedPin)
						{
							continue;
						}

						if (ensure(EdGraphPinToPinRef.Contains(LinkedToKnot)))
						{
							SerializedPin->LinkedTo.Add(EdGraphPinToPinRef[LinkedToKnot]);
						}
					}
					continue;
				}

				if (ensure(EdGraphPinToPinRef.Contains(LinkedTo)))
				{
					SerializedPin->LinkedTo.Add(EdGraphPinToPinRef[LinkedTo]);
				}
			}

			return true;
		};

		for (const UEdGraphPin* Pin : EdGraphNode->GetInputPins())
		{
			if (Pin->bOrphanedPin ||
				!FVoxelPinType(Pin->PinType).IsValid())
			{
				continue;
			}

			if (!ensure(AddPin(Pin)))
			{
				return {};
			}
		}
		for (const UEdGraphPin* Pin : EdGraphNode->GetOutputPins())
		{
			if (Pin->bOrphanedPin ||
				!FVoxelPinType(Pin->PinType).IsValid())
			{
				continue;
			}

			if (!ensure(AddPin(Pin)))
			{
				return {};
			}
		}
	}

	INLINE_LAMBDA
	{
		VOXEL_SCOPE_COUNTER("Setup preview");

		TVoxelArray<UEdGraphPin*> PreviewedPins;
		for (const UVoxelGraphNode* Node : Nodes)
		{
			if (!Node->bEnablePreview)
			{
				continue;
			}

			UEdGraphPin* PreviewedPin = Node->FindPin(Node->PreviewedPin);
			if (!PreviewedPin)
			{
				continue;
			}

			PreviewedPins.Add(PreviewedPin);
		}

		if (PreviewedPins.Num() > 1)
		{
			for (const UEdGraphPin* PreviewedPin : PreviewedPins)
			{
				CastChecked<UVoxelGraphNode>(PreviewedPin->GetOwningNode())->bEnablePreview = false;
			}
			return;
		}

		if (PreviewedPins.Num() == 0)
		{
			return;
		}

		const UEdGraphPin& PreviewedPin = *PreviewedPins[0];
		UVoxelGraphNode* PreviewedNode = CastChecked<UVoxelGraphNode>(PreviewedPin.GetOwningNode());

		FVoxelInstancedStruct PreviewHandler;
		for (const FVoxelPinPreviewSettings& PreviewSettings : PreviewedNode->PreviewSettings)
		{
			if (PreviewSettings.PinName == PreviewedPin.PinName)
			{
				ensure(!PreviewHandler.IsValid());
				PreviewHandler = PreviewSettings.PreviewHandler;
			}
		}
		if (!PreviewHandler.IsValid())
		{
			return;
		}

		SerializedGraph.PreviewedPin = FVoxelSerializedPinRef
		{
			PreviewedNode->GetFName(),
			PreviewedPin.PinName,
			PreviewedPin.Direction == EGPD_Input
		};

		SerializedGraph.PreviewHandler = PreviewHandler;
	};

	SerializedGraph.bIsValid = true;
	return SerializedGraph;
}