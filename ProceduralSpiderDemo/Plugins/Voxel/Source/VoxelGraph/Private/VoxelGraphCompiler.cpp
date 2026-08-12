// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelGraphCompiler.h"
#include "VoxelGraph.h"
#include "VoxelTemplateNode.h"
#include "VoxelTerminalGraph.h"
#include "VoxelGraphCompileScope.h"
#include "Preview/VoxelNode_Preview.h"
#include "Nodes/VoxelNode_UFunction.h"
#include "Nodes/VoxelNode_Parameter.h"
#include "Nodes/VoxelNode_RangeDebug.h"
#include "Nodes/VoxelNode_ValueDebug.h"
#include "Nodes/VoxelCallFunctionNodes.h"
#include "Nodes/VoxelFunctionInputNodes.h"
#include "Nodes/VoxelLocalVariableNodes.h"
#include "Nodes/VoxelNode_FunctionOutput.h"
#include "Nodes/VoxelNode_CustomizeParameter.h"
#include "FunctionLibrary/VoxelBasicFunctionLibrary.h"
#include "FunctionLibrary/VoxelPositionFunctionLibrary.h"

#define GRAPH_PASS(Name) { #Name, &FVoxelGraphCompiler::Name }
TVoxelArray<FVoxelGraphCompilerPass> GVoxelGraphCompilerPasses = 
{
	GRAPH_PASS(AddPreviewNode),
	GRAPH_PASS(AddRangeNodes),
	GRAPH_PASS(AddPreviewValueNodes),
	GRAPH_PASS(RemoveSplitPins),
	GRAPH_PASS(FixPositionPins),
	GRAPH_PASS(FixSplineKeyPins),
	GRAPH_PASS(AddWildcardErrors),
	GRAPH_PASS(AddNoDefaultErrors),
	GRAPH_PASS(CheckParameters),
	GRAPH_PASS(CheckFunctionInputs),
	GRAPH_PASS(CheckFunctionOutputs),
	GRAPH_PASS(AddToBuffer),
	GRAPH_PASS(RemoveLocalVariables),
	GRAPH_PASS(CollapseInputs),
	GRAPH_PASS(ReplaceTemplates),
	GRAPH_PASS(RemovePassthroughs),
	GRAPH_PASS(RemoveNodesNotLinkedToQueryableNodes),
	GRAPH_PASS(MergeSameSubTrees),
	GRAPH_PASS(CheckForLoops),
	GRAPH_PASS(CheckNodeGuids),
};
#undef GRAPH_PASS

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelGraphCompiler::FVoxelGraphCompiler(const UVoxelTerminalGraph& TerminalGraph)
	: TerminalGraph(TerminalGraph)
	, SerializedGraph(TerminalGraph.GetRuntime().GetSerializedGraph())
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelGraphCompiler::LoadSerializedGraph(
	const FOnVoxelGraphChanged& OnTranslated,
	const FOnVoxelGraphChanged& OnForceRecompile)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	check(GVoxelGraphCompileScope);

	// Manually call FixupPins on graph calls
	// This needs to happen as late as possible as it requires graphs to be loaded
	{
		VOXEL_SCOPE_COUNTER("Fixup graph calls");

		for (const auto& It : SerializedGraph.NodeNameToNode)
		{
			if (!It.Value.VoxelNode)
			{
				continue;
			}

			const FVoxelNode_CallFunction* CallFunctionNode = It.Value.VoxelNode->As<FVoxelNode_CallFunction>();
			if (!CallFunctionNode)
			{
				continue;
			}

			ConstCast(CallFunctionNode)->FixupPins(TerminalGraph.GetGraph(), OnTranslated, OnForceRecompile);
		}
	}

	TVoxelMap<const FVoxelSerializedNode*, FNode*> SerializedNodeToNode;

	for (const auto& NodeIt : SerializedGraph.NodeNameToNode)
	{
		const FVoxelSerializedNode& SerializedNode = NodeIt.Value;
		VOXEL_SCOPE_COUNTER_FNAME(SerializedNode.EdGraphNodeTitle);

		if (!SerializedNode.VoxelNode.IsValid())
		{
			VOXEL_MESSAGE(Error, "Invalid struct on {0}", SerializedNode);
			return false;
		}

		const FVoxelGraphNodeRef NodeRef
		{
			TerminalGraph,
			SerializedNode.GetNodeId(),
			SerializedNode.EdGraphNodeTitle,
			SerializedNode.EdGraphNodeName
		};

		FNode& Node = NewNode(NodeRef);
		{
			VOXEL_SCOPE_COUNTER_FORMAT("Copy node %s", *SerializedNode.VoxelNode->GetStruct()->GetName());
			Node.SetVoxelNode(SerializedNode.VoxelNode->MakeSharedCopy());
		}
		SerializedNodeToNode.Add_EnsureNew(&SerializedNode, &Node);

		for (const FString& Error : SerializedNode.Errors)
		{
			Node.AddError(Error);
		}

		const FVoxelNode& VoxelNode = Node.GetVoxelNode();
		for (const FVoxelPin& Pin : VoxelNode.GetPins())
		{
			if (!(Pin.bIsInput ? SerializedNode.InputPins : SerializedNode.OutputPins).Contains(Pin.Name))
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: missing pin {1}", SerializedNode, Pin.Name);
				return false;
			}
		}
		for (const auto& It : SerializedNode.InputPins)
		{
			const FVoxelSerializedPin& SerializedPin = It.Value;
			if (!SerializedPin.ParentPinName.IsNone())
			{
				// Sub-pinS
				continue;
			}

			const TSharedPtr<const FVoxelPin> Pin = VoxelNode.FindPin(SerializedPin.PinName);
			if (!Pin ||
				!Pin->bIsInput)
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: unknown pin {1}", SerializedNode, SerializedPin.PinName);
				return false;
			}

			if (SerializedPin.Type != Pin->GetType())
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: type mismatch for pin {1}", SerializedNode, SerializedPin.PinName);
				return false;
			}
		}
		for (const auto& It : SerializedNode.OutputPins)
		{
			const FVoxelSerializedPin& SerializedPin = It.Value;
			if (!SerializedPin.ParentPinName.IsNone())
			{
				// Sub-pinS
				continue;
			}

			const TSharedPtr<const FVoxelPin> Pin = VoxelNode.FindPin(SerializedPin.PinName);
			if (!Pin ||
				Pin->bIsInput)
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: unknown pin {1}", SerializedNode, SerializedPin.PinName);
				return false;
			}

			if (SerializedPin.Type != Pin->GetType())
			{
				VOXEL_MESSAGE(Error, "Outdated node {0}: type mismatch for pin {1}", SerializedNode, SerializedPin.PinName);
				return false;
			}
		}

		for (const auto& It : SerializedNode.InputPins)
		{
			const FVoxelSerializedPin& SerializedPin = It.Value;
			if (!SerializedPin.Type.IsValid())
			{
				VOXEL_MESSAGE(Error, "Invalid pin {0}.{1}", SerializedNode, SerializedPin.PinName);
				return false;
			}

			FVoxelPinValue DefaultValue;
			if (SerializedPin.Type.HasPinDefaultValue())
			{
				DefaultValue = SerializedPin.DefaultValue;

				if (!DefaultValue.IsValid() ||
					!DefaultValue.GetType().CanBeCastedTo(SerializedPin.Type.GetPinDefaultValueType()))
				{
					VOXEL_MESSAGE(Error, "{0}.{1}: Invalid default value", SerializedNode, SerializedPin.PinName);
					return false;
				}
			}
			else
			{
				ensureVoxelSlow(!SerializedPin.DefaultValue.IsValid());
			}

			Node.NewInputPin(SerializedPin.PinName, SerializedPin.Type, DefaultValue).SetParentName(SerializedPin.ParentPinName);
		}

		for (const auto& It : SerializedNode.OutputPins)
		{
			const FVoxelSerializedPin& SerializedPin = It.Value;
			if (!SerializedPin.Type.IsValid())
			{
				VOXEL_MESSAGE(Error, "Invalid pin {0}.{1}", SerializedNode, SerializedPin.PinName);
				return false;
			}

			Node.NewOutputPin(SerializedPin.PinName, SerializedPin.Type).SetParentName(SerializedPin.ParentPinName);
		}
	}

	VOXEL_SCOPE_COUNTER("Fixup links");

	// Fixup links after all nodes are created
	for (const auto& SerializedNodeIt : SerializedGraph.NodeNameToNode)
	{
		const FVoxelSerializedNode& SerializedNode = SerializedNodeIt.Value;
		FNode& Node = *SerializedNodeToNode[&SerializedNode];

		for (const auto& It : SerializedNode.InputPins)
		{
			const FVoxelSerializedPin& InputPin = It.Value;
			if (InputPin.LinkedTo.Num() > 1)
			{
				VOXEL_MESSAGE(Error, "Too many pins linked to {0}.{1}", SerializedNode, InputPin.PinName);
				return false;
			}

			for (const FVoxelSerializedPinRef& OutputPinRef : InputPin.LinkedTo)
			{
				check(!OutputPinRef.bIsInput);

				const FVoxelSerializedPin* OutputPin = SerializedGraph.FindPin(OutputPinRef);
				if (!ensure(OutputPin))
				{
					VOXEL_MESSAGE(Error, "Invalid pin ref on {0}", SerializedNode);
					continue;
				}

				const FVoxelSerializedNode& OtherSerializedNode = SerializedGraph.NodeNameToNode[OutputPinRef.NodeName];
				FNode& OtherNode = *SerializedNodeToNode[&OtherSerializedNode];

				if (!OutputPin->Type.CanBeCastedTo_Schema(InputPin.Type))
				{
					VOXEL_MESSAGE(Error, "Invalid pin link from {0}.{1} to {2}.{3}: type mismatch: {4} vs {5}",
						SerializedNode,
						OutputPin->PinName,
						OtherSerializedNode,
						InputPin.PinName,
						OutputPin->Type.ToString(),
						InputPin.Type.ToString());

					continue;
				}

				Node.FindInputChecked(InputPin.PinName).MakeLinkTo(OtherNode.FindOutputChecked(OutputPinRef.PinName));
			}
		}
	}

	// Input links are used to populate all links, check they're correct with output links
	for (const auto& SerializedNodeIt : SerializedGraph.NodeNameToNode)
	{
		const FVoxelSerializedNode& SerializedNode = SerializedNodeIt.Value;
		FNode& Node = *SerializedNodeToNode[&SerializedNode];

		for (const auto& It : SerializedNode.OutputPins)
		{
			const FVoxelSerializedPin& OutputPin = It.Value;
			for (const FVoxelSerializedPinRef& InputPinRef : OutputPin.LinkedTo)
			{
				check(InputPinRef.bIsInput);

				const FVoxelSerializedPin* InputPin = SerializedGraph.FindPin(InputPinRef);
				if (!ensure(InputPin))
				{
					VOXEL_MESSAGE(Error, "Invalid pin ref on {0}", SerializedNode);
					continue;
				}

				const FVoxelSerializedNode& OtherSerializedNode = SerializedGraph.NodeNameToNode[InputPinRef.NodeName];
				FNode& OtherNode = *SerializedNodeToNode[&OtherSerializedNode];

				if (!OutputPin.Type.CanBeCastedTo_Schema(InputPin->Type))
				{
					VOXEL_MESSAGE(Error, "Invalid pin link from {0}.{1} to {2}.{3}: type mismatch: {4} vs {5}",
						SerializedNode,
						OutputPin.PinName,
						OtherSerializedNode,
						InputPin->PinName,
						OutputPin.Type.ToString(),
						InputPin->Type.ToString());

					return false;
				}

				if (!ensure(Node.FindOutputChecked(OutputPin.PinName).IsLinkedTo(OtherNode.FindInputChecked(InputPinRef.PinName))))
				{
					VOXEL_MESSAGE(Error, "Translation error: {0} -> {1}", Node, OtherNode);
					return false;
				}
			}
		}
	}

	Check();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::AddPreviewNode()
{
	VOXEL_FUNCTION_COUNTER();

	// Make sure to run AddPreviewNode before any new node is spawned, otherwise the find below will fail
	// and find compile nodes instead

	const FVoxelSerializedPinRef PreviewedSerializedPin = SerializedGraph.PreviewedPin;
	if (PreviewedSerializedPin.NodeName.IsNone())
	{
		ensure(PreviewedSerializedPin.PinName.IsNone());
		ensure(!PreviewedSerializedPin.bIsInput);
		return;
	}

	FPin* PreviewedPin = nullptr;
	for (FNode& Node : GetNodes())
	{
		if (Node.NodeRef.EdGraphNodeName != PreviewedSerializedPin.NodeName)
		{
			continue;
		}

		ensure(!PreviewedPin);
		PreviewedPin = Node.FindPin(PreviewedSerializedPin.PinName);
		ensure(PreviewedPin);
	}

	if (!PreviewedPin)
	{
		VOXEL_MESSAGE(Warning, "Invalid preview pin");
		return;
	}

	const FVoxelGraphNodeRef NodeRef
	{
		TerminalGraph,
		"Preview",
		"Preview Node",
		{},
		true
	};

	FNode& PreviewNode = NewNode(NodeRef);

	const TSharedRef<FVoxelNode_Preview> PreviewVoxelNode = MakeShared<FVoxelNode_Preview>();
	PreviewVoxelNode->PromotePin_Runtime(PreviewVoxelNode->GetUniqueInputPin(), PreviewedPin->Type);

	PreviewNode.SetVoxelNode(PreviewVoxelNode);
	PreviewNode.NewInputPin(VOXEL_PIN_NAME(FVoxelNode_Preview, ValuePin), PreviewedPin->Type);

	if (PreviewedPin->Direction == EPinDirection::Input)
	{
		// TODO Is previewing an input allowed?
		PreviewedPin->CopyInputPinTo(PreviewNode.GetInputPin(0));
	}
	else
	{
		check(PreviewedPin->Direction == EPinDirection::Output);
		PreviewedPin->MakeLinkTo(PreviewNode.GetInputPin(0));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::AddRangeNodes()
{
#if WITH_EDITOR
	if (!TerminalGraph.GetGraph().bEnableNodeRangeStats)
	{
		return;
	}

	VOXEL_FUNCTION_COUNTER();

	for (FNode& Node : GetNodesCopy())
	{
		for (FPin& Pin : Node.GetOutputPins())
		{
			if (Pin.Type.IsWildcard())
			{
				continue;
			}

			FNode& DebugNode = NewGeneratedNode(Node.NodeRef.WithSuffix("DebugRangeNode").WithSuffix(Pin.Name.ToString()));

			const TSharedRef<FVoxelNode_RangeDebug> DebugVoxelNode = MakeShared<FVoxelNode_RangeDebug>();
			DebugVoxelNode->PromotePin_Runtime(DebugVoxelNode->GetUniqueInputPin(), Pin.Type);
			DebugVoxelNode->PromotePin_Runtime(DebugVoxelNode->GetUniqueOutputPin(), Pin.Type);
			DebugVoxelNode->RefPin = Pin.Name;

			DebugNode.SetVoxelNode(DebugVoxelNode);
			DebugNode.NewInputPin(VOXEL_PIN_NAME(FVoxelNode_RangeDebug, InPin), Pin.Type);
			DebugNode.NewOutputPin(VOXEL_PIN_NAME(FVoxelNode_RangeDebug, OutPin), Pin.Type);

			Pin.CopyOutputPinTo(DebugNode.GetOutputPin(0));
			Pin.BreakAllLinks();

			Pin.MakeLinkTo(DebugNode.GetInputPin(0));
		}
	}
#endif
}

void FVoxelGraphCompiler::AddPreviewValueNodes()
{
#if WITH_EDITOR
	if (SerializedGraph.PreviewedPin.NodeName.IsNone())
	{
		return;
	}

	VOXEL_FUNCTION_COUNTER();

	for (FNode& Node : GetNodesCopy())
	{
		for (FPin& Pin : Node.GetOutputPins())
		{
			if (Pin.Type.IsWildcard())
			{
				continue;
			}

			FNode& DebugNode = NewGeneratedNode(Node.NodeRef.WithSuffix("DebugValueNode").WithSuffix(Pin.Name.ToString()));

			const TSharedRef<FVoxelNode_ValueDebug> DebugVoxelNode = MakeShared<FVoxelNode_ValueDebug>();
			DebugVoxelNode->PromotePin_Runtime(DebugVoxelNode->GetUniqueInputPin(), Pin.Type);
			DebugVoxelNode->PromotePin_Runtime(DebugVoxelNode->GetUniqueOutputPin(), Pin.Type);
			DebugVoxelNode->RefPin = Pin.Name;

			DebugNode.SetVoxelNode(DebugVoxelNode);
			DebugNode.NewInputPin(VOXEL_PIN_NAME(FVoxelNode_ValueDebug, InPin), Pin.Type);
			DebugNode.NewOutputPin(VOXEL_PIN_NAME(FVoxelNode_ValueDebug, OutPin), Pin.Type);

			Pin.CopyOutputPinTo(DebugNode.GetOutputPin(0));
			Pin.BreakAllLinks();

			Pin.MakeLinkTo(DebugNode.GetInputPin(0));
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::RemoveSplitPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (FNode& Node : GetNodesCopy())
	{
		TVoxelMap<FPin*, FNode*> ParentPinToMakeBreakNodes;
		TVoxelArray<FPin*> PinsToRemove;
		for (FPin& SubPin : Node.GetPins())
		{
			if (SubPin.GetParentName().IsNone())
			{
				continue;
			}

			FPin* ParentPin = Node.FindPin(SubPin.GetParentName());
			if (!ensure(ParentPin))
			{
				VOXEL_MESSAGE(Error, "{0}: Invalid parent pin", SubPin);
				return;
			}

			FNode*& MakeBreakNode = ParentPinToMakeBreakNodes.FindOrAdd(ParentPin);
			if (!MakeBreakNode)
			{
				if (ParentPin->Direction == EPinDirection::Input)
				{
					FNode& MakeNode = NewGeneratedNode(Node.NodeRef.WithSuffix("Make_" + ParentPin->Name.ToString()));
					MakeBreakNode = &MakeNode;

					const FVoxelNode* MakeVoxelNodeTemplate = SerializedGraph.FindMakeNode(ParentPin->Type);
					if (!ensure(MakeVoxelNodeTemplate))
					{
						VOXEL_MESSAGE(Error, "{0}: No make node for type {1}", ParentPin, ParentPin->Type.ToString());
						return;
					}

					const TSharedRef<FVoxelNode> MakeVoxelNode = MakeVoxelNodeTemplate->MakeSharedCopy();
					MakeVoxelNode->PromotePin_Runtime(MakeVoxelNode->GetUniqueOutputPin(), ParentPin->Type);
					MakeNode.SetVoxelNode(MakeVoxelNode);

					MakeNode.NewOutputPin(
						MakeVoxelNode->GetUniqueOutputPin().Name,
						MakeVoxelNode->GetUniqueOutputPin().GetType()).MakeLinkTo(*ParentPin);

					for (const FVoxelPin& Pin : MakeVoxelNode->GetPins())
					{
						if (!Pin.bIsInput)
						{
							continue;
						}

						MakeNode.NewInputPin(Pin.Name, Pin.GetType());
					}
				}
				else
				{
					check(ParentPin->Direction == EPinDirection::Output);

					FNode& BreakNode = NewGeneratedNode(Node.NodeRef.WithSuffix("Break_" + ParentPin->Name.ToString()));
					MakeBreakNode = &BreakNode;

					const FVoxelNode* BreakVoxelNodeTemplate = SerializedGraph.FindBreakNode(ParentPin->Type);
					if (!ensure(BreakVoxelNodeTemplate))
					{
						VOXEL_MESSAGE(Error, "{0}: No break node for type {1}", ParentPin, ParentPin->Type.ToString());
						return;
					}

					const TSharedRef<FVoxelNode> BreakVoxelNode = BreakVoxelNodeTemplate->MakeSharedCopy();
					BreakVoxelNode->PromotePin_Runtime(BreakVoxelNode->GetUniqueInputPin(), ParentPin->Type);
					BreakNode.SetVoxelNode(BreakVoxelNode);

					BreakNode.NewInputPin(
						BreakVoxelNode->GetUniqueInputPin().Name,
						BreakVoxelNode->GetUniqueInputPin().GetType()).MakeLinkTo(*ParentPin);

					for (const FVoxelPin& Pin : BreakVoxelNode->GetPins())
					{
						if (Pin.bIsInput)
						{
							continue;
						}

						BreakNode.NewOutputPin(Pin.Name, Pin.GetType());
					}
				}
			}
			check(MakeBreakNode);

			TArray<FString> Parts;
			SubPin.Name.ToString().ParseIntoArray(Parts, TEXT("|"));

			FName SubPinName = SubPin.Name;
			if (Parts.Num() > 1)
			{
				SubPinName = FName(Parts.Last());
			}

			FPin* NewPin = MakeBreakNode->FindPin(SubPinName);
			if (!ensure(NewPin))
			{
				VOXEL_MESSAGE(Error, "{0}: Invalid sub-pin", SubPin);
				return;
			}
			ensure(NewPin->GetLinkedTo().Num() == 0);

			if (SubPin.Direction == EPinDirection::Input)
			{
				SubPin.CopyInputPinTo(*NewPin);
			}
			else
			{
				check(SubPin.Direction == EPinDirection::Output);
				SubPin.CopyOutputPinTo(*NewPin);
			}

			PinsToRemove.Add(&SubPin);
		}

		for (FPin* Pin : PinsToRemove)
		{
			Node.RemovePin(*Pin);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::FixPositionPins()
{
	VOXEL_FUNCTION_COUNTER();

	for (FNode& Node : GetNodesCopy())
	{
		for (FPin& Pin : Node.GetPins())
		{
			if (Pin.GetLinkedTo().Num() > 0)
			{
				continue;
			}

			const TSharedPtr<const FVoxelPin> VoxelPin = Node.GetVoxelNode().FindPin(Pin.Name);
			if (!ensure(VoxelPin) ||
				!VoxelPin->Metadata.bPositionPin)
			{
				continue;
			}

			UFunction* Function;
			bool bIs3D;
			if (Pin.Type.Is<FVoxelVector2DBuffer>())
			{
				Function = FindUFunctionChecked(UVoxelPositionFunctionLibrary, GetPosition2D);
				bIs3D = false;
			}
			else if (Pin.Type.Is<FVoxelVectorBuffer>())
			{
				Function = FindUFunctionChecked(UVoxelPositionFunctionLibrary, GetPosition3D);
				bIs3D = true;
			}
			else if (Pin.Type.Is<FVoxelDoubleVector2DBuffer>())
			{
				Function = FindUFunctionChecked(UVoxelPositionFunctionLibrary, GetPosition2D_Double);
				bIs3D = false;
			}
			else if (Pin.Type.Is<FVoxelDoubleVectorBuffer>())
			{
				Function = FindUFunctionChecked(UVoxelPositionFunctionLibrary, GetPosition3D_Double);
				bIs3D = true;
			}
			else
			{
				ensure(false);
				continue;
			}

			const TSharedRef<FVoxelNode_UFunction> FunctionNode = FVoxelNode_UFunction::Make(Function);

			FNode& PositionNode = NewGeneratedNode(Node.NodeRef.WithSuffix("GetPosition"));
			PositionNode.SetVoxelNode(FunctionNode);

			PositionNode.NewInputPin(
				FunctionNode->FindPinChecked("Space").Name,
				FVoxelPinType::Make<EVoxelPositionSpace>(),
				FVoxelPinValue::Make(EVoxelPositionSpace::LocalSpace));

			if (bIs3D)
			{
				// Don't raise errors on position pins
				PositionNode.NewInputPin(
					FunctionNode->FindPinChecked("bFallbackTo2D").Name,
					FVoxelPinType::Make<bool>(),
					FVoxelPinValue::Make(true));
			}

			PositionNode.NewOutputPin(
				FunctionNode->GetUniqueOutputPin().Name,
				FunctionNode->GetUniqueOutputPin().GetType()).MakeLinkTo(Pin);
		}
	}
}

void FVoxelGraphCompiler::FixSplineKeyPins()
{
	VOXEL_FUNCTION_COUNTER();

	const UClass* Class = LoadClass<UObject>(nullptr, TEXT("/Script/Voxel.VoxelSplineStampFunctionLibrary"));
	check(Class);

	UFunction* Function = Class->FindFunctionByName("GetClosestSplineKeyGeneric");
	check(Function);

	for (FNode& Node : GetNodesCopy())
	{
		for (FPin& Pin : Node.GetPins())
		{
			if (Pin.GetLinkedTo().Num() > 0)
			{
				continue;
			}

			const TSharedPtr<const FVoxelPin> VoxelPin = Node.GetVoxelNode().FindPin(Pin.Name);
			if (!ensure(VoxelPin) ||
				!VoxelPin->Metadata.bSplineKeyPin ||
				!ensure(Pin.Type.Is<FVoxelFloatBuffer>()))
			{
				continue;
			}

			FNode& SplineKeyNode = NewGeneratedNode(Node.NodeRef.WithSuffix("GetClosestSplineKeyGeneric"));
			{
				SplineKeyNode.SetVoxelNode(FVoxelNode_UFunction::Make(Function));

				SplineKeyNode.NewInputPin(
					SplineKeyNode.GetVoxelNode().GetUniqueInputPin().Name,
					SplineKeyNode.GetVoxelNode().GetUniqueInputPin().GetType());

				SplineKeyNode.NewOutputPin(
					SplineKeyNode.GetVoxelNode().GetUniqueOutputPin().Name,
					SplineKeyNode.GetVoxelNode().GetUniqueOutputPin().GetType()).MakeLinkTo(Pin);
			}

			FNode& PositionNode = NewGeneratedNode(Node.NodeRef.WithSuffix("GetPosition"));
			{
				PositionNode.SetVoxelNode(FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelPositionFunctionLibrary, GetPosition3D)));

				PositionNode.NewInputPin(
					PositionNode.GetVoxelNode().FindPinChecked("Space").Name,
					FVoxelPinType::Make<EVoxelPositionSpace>(),
					FVoxelPinValue::Make(EVoxelPositionSpace::LocalSpace));

				// Don't raise errors on position pins
				PositionNode.NewInputPin(
					PositionNode.GetVoxelNode().FindPinChecked("bFallbackTo2D").Name,
					FVoxelPinType::Make<bool>(),
					FVoxelPinValue::Make(true));

				PositionNode.NewOutputPin(
					PositionNode.GetVoxelNode().GetUniqueOutputPin().Name,
					PositionNode.GetVoxelNode().GetUniqueOutputPin().GetType());
			}

			PositionNode.GetOutputPin(0).MakeLinkTo(SplineKeyNode.GetInputPin(0));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::AddWildcardErrors()
{
	VOXEL_FUNCTION_COUNTER();

	for (FNode& Node : GetNodes())
	{
		for (const FPin& Pin : Node.GetPins())
		{
			if (Pin.Type.IsWildcard())
			{
				Node.AddError("Wildcard pin " + Pin.Name.ToString() + " needs to be converted. Please connect it to another pin or right click it -> Convert");
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::AddNoDefaultErrors()
{
	VOXEL_FUNCTION_COUNTER();

	for (FNode& Node : GetNodes())
	{
		for (const FPin& Pin : Node.GetPins())
		{
			if (Pin.GetLinkedTo().Num() > 0)
			{
				continue;
			}

			const TSharedPtr<const FVoxelPin> VoxelPin = Node.GetVoxelNode().FindPin(Pin.Name);
			if (!ensure(VoxelPin) ||
				!VoxelPin->Metadata.bNoDefault)
			{
				continue;
			}

			FString PinName = Pin.Name.ToString();
#if WITH_EDITOR
			if (!VoxelPin->Metadata.DisplayName.IsEmpty())
			{
				PinName = VoxelPin->Metadata.DisplayName;
			}
#endif
			Node.AddError("Pin " + PinName + " needs to be connected");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::CheckParameters() const
{
	VOXEL_FUNCTION_COUNTER();

	for (const FNode& Node : GetNodes())
	{
		const FVoxelNode_Parameter* ParameterNode = Node.GetVoxelNode().As<FVoxelNode_Parameter>();
		if (!ParameterNode)
		{
			continue;
		}

		if (!TerminalGraph.GetGraph().FindParameter(ParameterNode->ParameterGuid))
		{
			VOXEL_MESSAGE(Error, "{0}: No parameter named {1}", Node, ParameterNode->ParameterName);
		}
	}

	TVoxelMap<FGuid, TVoxelArray<const FNode*>> GuidToCustomizeNodes;
	for (const FNode& Node : GetNodes())
	{
		const FVoxelNode_CustomizeParameter* CustomizeParameterNode = Node.GetVoxelNode().As<FVoxelNode_CustomizeParameter>();
		if (!CustomizeParameterNode)
		{
			continue;
		}

		if (!TerminalGraph.GetGraph().FindParameter(CustomizeParameterNode->ParameterGuid))
		{
			VOXEL_MESSAGE(Error, "{0}: No parameter named {1}", Node, CustomizeParameterNode->ParameterName);
			continue;
		}

		GuidToCustomizeNodes.FindOrAdd(CustomizeParameterNode->ParameterGuid).Add(&Node);
	}

	for (const auto& It : GuidToCustomizeNodes)
	{
		if (It.Value.Num() > 1)
		{
			VOXEL_MESSAGE(Error, "Conflicting Customize nodes: {0}", It.Value);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::CheckFunctionInputs() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!TerminalGraph.IsFunction())
	{
		return;
	}

	TVoxelMap<FGuid, TVoxelArray<const FNode*>> InputGuidToDefaultNodes;
	TVoxelMap<FGuid, TVoxelArray<const FNode*>> InputGuidToPreviewNodes;
	for (const FNode& Node : GetNodes())
	{
		if (const FVoxelNode_FunctionInputDefault* InputNode = Node.GetVoxelNode().As<FVoxelNode_FunctionInputDefault>())
		{
			if (!TerminalGraph.FindInput(InputNode->Guid))
			{
				VOXEL_MESSAGE(Error, "{0}: No input with GUID {1}",
					Node,
					InputNode->Guid.ToString());

				continue;
			}

			InputGuidToDefaultNodes.FindOrAdd(InputNode->Guid).Add(&Node);
		}

		if (const FVoxelNode_FunctionInputPreview* InputNode = Node.GetVoxelNode().As<FVoxelNode_FunctionInputPreview>())
		{
			if (!TerminalGraph.FindInput(InputNode->Guid))
			{
				VOXEL_MESSAGE(Error, "{0}: No input with GUID {1}",
					Node,
					InputNode->Guid.ToString());

				continue;
			}

			InputGuidToPreviewNodes.FindOrAdd(InputNode->Guid).Add(&Node);
		}

		if (const FVoxelNode_FunctionInput* InputNode = Node.GetVoxelNode().As<FVoxelNode_FunctionInput>())
		{
			if (!TerminalGraph.FindInput(InputNode->Guid))
			{
				VOXEL_MESSAGE(Error, "{0}: No input with GUID {1}",
					Node,
					InputNode->Guid.ToString());
			}
		}
	}

	for (const auto& It : InputGuidToDefaultNodes)
	{
		if (It.Value.Num() > 1)
		{
			VOXEL_MESSAGE(Error, "Multiple default value input nodes for input {0}: {1}",
				TerminalGraph.FindInputChecked(It.Key).Name,
				It.Value);
		}
	}

	for (const auto& It : InputGuidToPreviewNodes)
	{
		if (It.Value.Num() > 1)
		{
			VOXEL_MESSAGE(Error, "Multiple preview input nodes for input {0}: {1}",
				TerminalGraph.FindInputChecked(It.Key).Name,
				It.Value);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::CheckFunctionOutputs() const
{
	VOXEL_FUNCTION_COUNTER();

	if (!TerminalGraph.IsFunction())
	{
		return;
	}

	TVoxelMap<FGuid, const FNode*> OutputGuidToNode;
	for (const FNode& Node : GetNodes())
	{
		const FVoxelNode_FunctionOutput* OutputNode = Node.GetVoxelNode().As<FVoxelNode_FunctionOutput>();
		if (!OutputNode)
		{
			continue;
		}

		if (!TerminalGraph.FindOutput(OutputNode->Guid))
		{
			VOXEL_MESSAGE(Error, "{0}: No output named {1}", Node, OutputNode->Guid.ToString());
			continue;
		}

		const FNode*& RootNode = OutputGuidToNode.FindOrAdd(OutputNode->Guid);
		if (RootNode)
		{
			VOXEL_MESSAGE(Error, "Duplicated output nodes: {0}, {1}", RootNode, Node);
			continue;
		}

		RootNode = &Node;
	}

	for (const FGuid& Guid : TerminalGraph.GetFunctionOutputs())
	{
		if (OutputGuidToNode.Contains(Guid))
		{
			continue;
		}

		VOXEL_MESSAGE(Error, "{0}: Missing function output node for {1}",
			TerminalGraph,
			TerminalGraph.FindOutputChecked(Guid).Name);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::AddToBuffer()
{
	VOXEL_FUNCTION_COUNTER();

	for (FNode& Node : GetNodesCopy())
	{
		for (FPin& ToPin : Node.GetInputPins())
		{
			if (!ToPin.Type.IsBuffer())
			{
				continue;
			}
			FNode* ToBufferNode = nullptr;

			for (FPin& FromPin : ToPin.GetLinkedTo())
			{
				if (FromPin.Type.IsBuffer())
				{
					continue;
				}

				if (!ToBufferNode)
				{
					ToBufferNode = &NewGeneratedNode(Node.NodeRef.WithSuffix(ToPin.Name.ToString() + "_ToBuffer"));

					const TSharedRef<FVoxelNode_UFunction> FunctionNode = FVoxelNode_UFunction::Make(FindUFunctionChecked(UVoxelBasicFunctionLibrary, ToBuffer));
					FunctionNode->PromotePin_Runtime(FunctionNode->GetUniqueInputPin(), FromPin.Type);
					FunctionNode->PromotePin_Runtime(FunctionNode->GetUniqueOutputPin(), ToPin.Type);
					ToBufferNode->SetVoxelNode(FunctionNode);

					ToBufferNode->NewInputPin(FunctionNode->GetUniqueInputPin().Name, FromPin.Type);
					ToBufferNode->NewOutputPin(FunctionNode->GetUniqueOutputPin().Name, ToPin.Type);
				}

				FromPin.MakeLinkTo(ToBufferNode->GetInputPin(0));
				ToPin.MakeLinkTo(ToBufferNode->GetOutputPin(0));
				ToPin.BreakLinkTo(FromPin);
			}
		}
	}

	for (const FNode& Node : GetNodes())
	{
		for (const FPin& ToPin : Node.GetInputPins())
		{
			for (const FPin& FromPin : ToPin.GetLinkedTo())
			{
				ensure(FromPin.Type.CanBeCastedTo(ToPin.Type));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::RemoveLocalVariables()
{
	VOXEL_FUNCTION_COUNTER();

	TMap<FGuid, FNode*> GuidToDeclaration;
	for (FNode& Node : GetNodes())
	{
		const FVoxelNode_LocalVariableDeclaration* Declaration = Node.GetVoxelNode().As<FVoxelNode_LocalVariableDeclaration>();
		if (!Declaration)
		{
			continue;
		}

		if (!TerminalGraph.FindLocalVariable(Declaration->Guid))
		{
			VOXEL_MESSAGE(
				Error,
				"Invalid local variable node: {0}",
				Declaration);
			return;
		}

		if (const FNode* ExistingNode = GuidToDeclaration.FindRef(Declaration->Guid))
		{
			VOXEL_MESSAGE(
				Error,
				"Multiple local variable declarations not supported: {0}, {1}",
				ExistingNode,
				Node);
			return;
		}

		GuidToDeclaration.Add(Declaration->Guid, &Node);
	}

	TMap<FNode*, TArray<FNode*>> DeclarationToUsage;
	for (FNode& Node : GetNodes())
	{
		const FVoxelNode_LocalVariableUsage* Usage = Node.GetVoxelNode().As<FVoxelNode_LocalVariableUsage>();
		if (!Usage)
		{
			continue;
		}

		FNode* Declaration = GuidToDeclaration.FindRef(Usage->Guid);
		if (!Declaration)
		{
			VOXEL_MESSAGE(
				Error,
				"Missing local variable declaration: {0}",
				Node);
			return;
		}

		DeclarationToUsage.FindOrAdd(Declaration).Add(&Node);
	}

	// Remove usages
	for (auto& It : DeclarationToUsage)
	{
		FNode& Declaration = *It.Key;

		const FPin& InputPin = Declaration.GetInputPin(0);
		for (FNode* Usage : It.Value)
		{
			for (FPin& LinkedTo : Usage->GetOutputPin(0).GetLinkedTo())
			{
				InputPin.CopyInputPinTo(LinkedTo);
			}

			RemoveNode(*Usage);
		}
	}

	// Remove declarations
	for (const auto& It : GuidToDeclaration)
	{
		FNode& Declaration = *It.Value;

		const FPin& InputPin = Declaration.GetInputPin(0);
		for (FPin& LinkedTo : Declaration.GetOutputPin(0).GetLinkedTo())
		{
			InputPin.CopyInputPinTo(LinkedTo);
		}
		RemoveNode(Declaration);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::CollapseInputs()
{
	VOXEL_FUNCTION_COUNTER();

	struct FInputData
	{
		TArray<FNode*> InputNodes;
		FNode* DefaultNode = nullptr;
		FNode* PreviewNode = nullptr;
	};
	TVoxelMap<FGuid, FInputData> InputGuidToNodes;

	for (FNode& Node : GetNodes())
	{
		const FVoxelNode_FunctionInputBase* InputBaseNode = Node.GetVoxelNode().As<FVoxelNode_FunctionInputBase>();
		if (!InputBaseNode)
		{
			continue;
		}

		FInputData& InputData = InputGuidToNodes.FindOrAdd(InputBaseNode->Guid);

		if (InputBaseNode->IsA<FVoxelNode_FunctionInputDefault>())
		{
			InputData.DefaultNode = &Node;
		}
		else if (InputBaseNode->IsA<FVoxelNode_FunctionInputPreview>())
		{
			InputData.PreviewNode = &Node;
		}
		else if (InputBaseNode->IsA<FVoxelNode_FunctionInput>())
		{
			InputData.InputNodes.Add(&Node);
		}
		else
		{
			ensure(false);
		}
	}

	for (auto& It : InputGuidToNodes)
	{
		const FInputData& InputData = It.Value;

		ON_SCOPE_EXIT
		{
			if (InputData.DefaultNode)
			{
				RemoveNode(*InputData.DefaultNode);
			}
			if (InputData.PreviewNode)
			{
				RemoveNode(*InputData.PreviewNode);
			}
		};

		if (InputData.InputNodes.Num() == 0)
		{
			continue;
		}

		const FVoxelGraphFunctionInput* Input = TerminalGraph.FindInput(It.Key);
		if (!ensure(Input))
		{
			VOXEL_MESSAGE(Error, "Invalid input");
			continue;
		}

		FNode& Node = NewGeneratedNode(InputData.InputNodes[0]->NodeRef);

		const TSharedRef<FVoxelNode_FunctionInput_WithDefaults> VoxelNode = MakeShared<FVoxelNode_FunctionInput_WithDefaults>();
		VoxelNode->Guid = It.Key;
		VoxelNode->PromotePin_Runtime(VoxelNode->FindPinChecked(VoxelNode->DefaultPin.GetName()), Input->Type);
		VoxelNode->PromotePin_Runtime(VoxelNode->FindPinChecked(VoxelNode->PreviewPin.GetName()), Input->Type);
		VoxelNode->PromotePin_Runtime(VoxelNode->FindPinChecked(VoxelNode->ValuePin.GetName()), Input->Type);

		Node.SetVoxelNode(VoxelNode);

		FVoxelPinValue DefaultValue;
		if (Input->Type.HasPinDefaultValue())
		{
			DefaultValue = FVoxelPinValue(Input->Type.GetPinDefaultValueType());
		}

		FPin& DefaultPin = Node.NewInputPin(VOXEL_PIN_NAME(FVoxelNode_FunctionInput_WithDefaults, DefaultPin), Input->Type, DefaultValue);
		if (InputData.DefaultNode)
		{
			VoxelNode->bHasDefaultNode = true;

			const FPin& DefaultValuePin = InputData.DefaultNode->GetInputPin(0);
			DefaultValuePin.CopyInputPinTo(DefaultPin);
		}

		FPin& PreviewPin = Node.NewInputPin(VOXEL_PIN_NAME(FVoxelNode_FunctionInput_WithDefaults, PreviewPin), Input->Type,DefaultValue);
		if (InputData.PreviewNode)
		{
			VoxelNode->bHasPreviewNode = true;

			const FPin& PreviewValuePin = InputData.PreviewNode->GetInputPin(0);
			PreviewValuePin.CopyInputPinTo(PreviewPin);
		}

		if (!InputData.DefaultNode &&
			!Input->bNoDefault)
		{
			VoxelNode->DefaultValue = Input->DefaultPinValue;
		}

		FPin& Pin = Node.NewOutputPin(VOXEL_PIN_NAME(FVoxelNode_FunctionInput_WithDefaults, ValuePin), Input->Type);
		for (FNode* InputNode : InputData.InputNodes)
		{
			InputNode->GetOutputPin(0).CopyOutputPinTo(Pin);
			RemoveNode(*InputNode);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::ReplaceTemplates()
{
	VOXEL_FUNCTION_COUNTER();

	while (true)
	{
		if (!ReplaceTemplatesImpl())
		{
			// No nodes removed, exit
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::RemovePassthroughs()
{
	VOXEL_FUNCTION_COUNTER();

	RemoveNodes([&](FNode& Node)
	{
		if (Node.Type != ENodeType::Passthrough)
		{
			return false;
		}

		const int32 Num = Node.GetInputPins().Num();
		check(Num == Node.GetOutputPins().Num());

		for (int32 Index = 0; Index < Num; Index++)
		{
			const FPin& InputPin = Node.GetInputPin(Index);
			const FPin& OutputPin = Node.GetOutputPin(Index);

			for (FPin& LinkedTo : OutputPin.GetLinkedTo())
			{
				InputPin.CopyInputPinTo(LinkedTo);
			}
		}

		return true;
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::RemoveNodesNotLinkedToQueryableNodes()
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<const FNode*> NodesToKeep;
	for (const FNode& Node : GetNodes())
	{
		if (Node.Type != ENodeType::Struct)
		{
			continue;
		}

		if (Node.GetVoxelNode().CanBeQueried())
		{
			NodesToKeep.Add(&Node);
		}
	}
	RemoveNodesImpl(NodesToKeep);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::MergeSameSubTrees()
{
	if (TerminalGraph.bDisableNodeMerging)
	{
		return;
	}

	TVoxelArray<TVoxelArray<FNode*>> LeafNodeGroups;

	// Find all nodes, which do not have input pins connected
	{
		TVoxelArray<FNode*> LeafNodes;
		for (FNode& Node : GetNodes())
		{
			bool bHasAnyInputLinks = false;
			for (const FPin& InputPin : Node.GetInputPins())
			{
				if (InputPin.GetLinkedTo().Num() == 0)
				{
					continue;
				}

				bHasAnyInputLinks = true;
				break;
			}

			if (bHasAnyInputLinks)
			{
				continue;
			}

			LeafNodes.Add(&Node);
		}

		if (LeafNodes.Num() > 1)
		{
			LeafNodeGroups.Add(MoveTemp(LeafNodes));
		}
	}

	const auto ArePinsSame = [](const FPin& A, const FPin& B, const auto& Lambda)
	{
		if (A.Type != B.Type)
		{
			return false;
		}

		if (A.Direction != B.Direction)
		{
			return false;
		}

		// Output pins don't have default value
		// and if are connected to other nodes, does not change the evaluation flow
		if (A.Direction == EPinDirection::Output)
		{
			return true;
		}

		if (A.GetRawLinkedTo().Num() != B.GetRawLinkedTo().Num())
		{
			return false;
		}

		// If no connection, check if default value is same
		if (A.GetRawLinkedTo().Num() == 0)
		{
			return A.GetDefaultValue() == B.GetDefaultValue();
		}

		for (int32 Index = 0; Index < A.GetRawLinkedTo().Num(); Index++)
		{
			const FPin* LinkedToPin = A.GetRawLinkedTo()[Index];
			const FPin* OtherLinkedToPin = B.GetRawLinkedTo()[Index];
			if ((LinkedToPin != nullptr) != (OtherLinkedToPin != nullptr))
			{
				return false;
			}

			if (!LinkedToPin)
			{
				continue;
			}

			if (LinkedToPin->Name != OtherLinkedToPin->Name ||
				LinkedToPin->Type != OtherLinkedToPin->Type)
			{
				return false;
			}

			// We check if linked to node is same
			if (!Lambda(LinkedToPin->Node, OtherLinkedToPin->Node, Lambda))
			{
				return false;
			}
		}

		return true;
	};
	const auto CanMergeNodes = [ArePinsSame](const FNode& A, const FNode& B, const auto& Lambda)
	{
		if (!A.GetVoxelNode().IsNodeIdentical(B.GetVoxelNode()))
		{
			return false;
		}

		if (A.GetRawPins().Num() != B.GetRawPins().Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < A.GetRawPins().Num(); Index++)
		{
			const FPin* PinA = A.GetRawPins()[Index];
			const FPin* PinB = B.GetRawPins()[Index];
			if ((PinA != nullptr) != (PinB != nullptr))
			{
				return false;
			}

			if (!PinA)
			{
				continue;
			}

			if (!ArePinsSame(*PinA, *PinB, Lambda))
			{
				return false;
			}
		}

		return true;
	};

	while (LeafNodeGroups.Num() > 0)
	{
		TVoxelArray<FNode*> LeafNodes = TVoxelSet<FNode*>(LeafNodeGroups.Pop()).Array();

		TVoxelMap<FNode*, TVoxelSet<const FNode*>> NodeToCopies;

		// From the current group, find all similar nodes and pair them
		// First node to all identical nodes
		for (int32 Index = 0; Index < LeafNodes.Num() - 1; Index++)
		{
			Voxel::Graph::FNode* Node = LeafNodes[Index];
			int32 InnerIndex = Index + 1;
			while (InnerIndex < LeafNodes.Num())
			{
				const Voxel::Graph::FNode* OtherNode = LeafNodes[InnerIndex];
				if (!CanMergeNodes(*Node, *OtherNode, CanMergeNodes))
				{
					InnerIndex++;
					continue;
				}

				NodeToCopies.FindOrAdd(Node).Add(OtherNode);
				LeafNodes.RemoveAtSwap(InnerIndex);
			}
		}

		// If no pairs found, stop
		if (NodeToCopies.Num() == 0)
		{
			break;
		}

		for (const auto& It : NodeToCopies)
		{
			// Copy output pins to the first node from identical nodes
			for (const FNode* OtherNode : It.Value)
			{
				It.Key->AddMergedNodeRef(*OtherNode);

				for (int32 Index = 0; Index < OtherNode->GetOutputPins().Num(); Index++)
				{
					const FPin& PinToCopyFrom = OtherNode->GetOutputPins()[Index];
					FPin& PinToCopyTo = It.Key->GetOutputPins()[Index];
					PinToCopyFrom.CopyOutputPinTo(PinToCopyTo);
				}
			}

			// Remove all, except the first, identical nodes
			RemoveNodes([&](const FNode& Node)
			{
				return It.Value.Contains(&Node);
			});

			// Take the first node from the pair and put all nodes, connected to output pins, as new leaf nodes 
			for (const FPin& Pin : It.Key->GetOutputPins())
			{
				if (Pin.GetLinkedTo().Num() < 2)
				{
					continue;
				}

				TVoxelArray<FNode*> NewLeafNodes;
				NewLeafNodes.Reserve(Pin.GetLinkedTo().Num());
				for (const FPin& LinkedPin : Pin.GetLinkedTo())
				{
					NewLeafNodes.Add(&LinkedPin.Node);
				}

				LeafNodeGroups.Add(NewLeafNodes);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphCompiler::CheckForLoops()
{
	VOXEL_FUNCTION_COUNTER();

	TSet<FNode*> VisitedNodes;
	TVoxelArray<FNode*> NodesToSort = GetNodesArray();

	while (NodesToSort.Num() > 0)
	{
		bool bHasRemovedNode = false;
		for (int32 Index = 0; Index < NodesToSort.Num(); Index++)
		{
			FNode* Node = NodesToSort[Index];

			const bool bHasInputPin = INLINE_LAMBDA
			{
				for (const FPin& Pin : Node->GetInputPins())
				{
					for (const FPin& LinkedTo : Pin.GetLinkedTo())
					{
						if (!VisitedNodes.Contains(&LinkedTo.Node))
						{
							return true;
						}
					}
				}
				return false;
			};

			if (bHasInputPin)
			{
				continue;
			}

			VisitedNodes.Add(Node);

			NodesToSort.RemoveAtSwap(Index);
			Index--;
			bHasRemovedNode = true;
		}

		if (!bHasRemovedNode &&
			NodesToSort.Num() > 0)
		{
			VOXEL_MESSAGE(Error, "Loop detected: {0}", NodesToSort);
			return;
		}
	}
}

void FVoxelGraphCompiler::CheckNodeGuids()
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelMap<FGuid, const FNode*> GuidToNode;

	for (const FNode& Node : GetNodes())
	{
		if (!Node.GetVoxelNode().NodeGuid.IsValid())
		{
			// Created during compilation
			continue;
		}

		if (const Voxel::Graph::FNode** OtherNode = GuidToNode.Find(Node.GetVoxelNode().NodeGuid))
		{
			VOXEL_MESSAGE(Error, "Duplicated node GUID: {0} and {1} have the same GUID", Node, OtherNode);
			continue;
		}

		GuidToNode.Add_EnsureNew(Node.GetVoxelNode().NodeGuid, &Node);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelGraphCompiler::ReplaceTemplatesImpl()
{
	VOXEL_FUNCTION_COUNTER();

	for (FNode& Node : GetNodes())
	{
		if (Node.Type != ENodeType::Struct ||
			!Node.GetVoxelNode().IsA<FVoxelTemplateNode>())
		{
			continue;
		}

		bool bHasWildcardPin = false;
		for (const FPin& Pin : Node.GetPins())
		{
			if (Pin.Type.IsWildcard())
			{
				bHasWildcardPin = true;
			}
		}

		if (bHasWildcardPin)
		{
			// Can't replace template with a wildcard
			// If this node is unused, it'll be removed by RemoveUnusedNodes down the pipeline
			// If it is used, AddWildcardErrors will catch it
			continue;
		}

		FVoxelTemplateNodeContext Context(Node.NodeRef);

		ensure(!GVoxelTemplateNodeContext);
		GVoxelTemplateNodeContext = &Context;

		ensure(!OnNewNode);
		OnNewNode = [&](FNode& NewNode)
		{
			Node.ApplyErrorsTo(NewNode);
		};

		ON_SCOPE_EXIT
		{
			ensure(GVoxelTemplateNodeContext == &Context);
			GVoxelTemplateNodeContext = nullptr;

			ensure(OnNewNode);
			OnNewNode = {};
		};

		InitializeTemplatesPassthroughNodes(Node);

		Node.GetVoxelNode<FVoxelTemplateNode>().ExpandNode(*this, Node);
		RemoveNode(Node);

		return true;
	}

	return false;
}

void FVoxelGraphCompiler::InitializeTemplatesPassthroughNodes(FNode& Node)
{
	VOXEL_FUNCTION_COUNTER();

	for (FPin& InputPin : Node.GetInputPins())
	{
		FNode& Passthrough = NewGeneratedNode(ENodeType::Passthrough, FVoxelTemplateNodeUtilities::GetNodeRef());
		FPin& PassthroughInputPin = Passthrough.NewInputPin("Input" + InputPin.Name, InputPin.Type);
		FPin& PassthroughOutputPin = Passthrough.NewOutputPin(InputPin.Name, InputPin.Type);

		InputPin.CopyInputPinTo(PassthroughInputPin);

		InputPin.BreakAllLinks();
		InputPin.MakeLinkTo(PassthroughOutputPin);
	}

	for (FPin& OutputPin : Node.GetOutputPins())
	{
		FNode& Passthrough = NewGeneratedNode(ENodeType::Passthrough, FVoxelTemplateNodeUtilities::GetNodeRef());
		FPin& PassthroughInputPin = Passthrough.NewInputPin(OutputPin.Name, OutputPin.Type);
		if (OutputPin.Type.HasPinDefaultValue())
		{
			PassthroughInputPin.SetDefaultValue(FVoxelPinValue(OutputPin.Type.GetPinDefaultValueType()));
		}
		FPin& PassthroughOutputPin = Passthrough.NewOutputPin("Output" + OutputPin.Name, OutputPin.Type);

		OutputPin.CopyOutputPinTo(PassthroughOutputPin);

		OutputPin.BreakAllLinks();
		OutputPin.MakeLinkTo(PassthroughInputPin);
	}
}

void FVoxelGraphCompiler::RemoveNodesImpl(const TVoxelArray<const FNode*>& RootNodes)
{
	VOXEL_FUNCTION_COUNTER();

	TSet<const FNode*> ValidNodes;

	TVoxelArray<const FNode*> NodesToVisit = RootNodes;
	while (NodesToVisit.Num() > 0)
	{
		const FNode* Node = NodesToVisit.Pop();
		if (ValidNodes.Contains(Node))
		{
			continue;
		}
		ValidNodes.Add(Node);

		for (const FPin& Pin : Node->GetInputPins())
		{
			for (const FPin& LinkedTo : Pin.GetLinkedTo())
			{
				NodesToVisit.Add(&LinkedTo.Node);
			}
		}
	}

	RemoveNodes([&](const FNode& Node)
	{
		return !ValidNodes.Contains(&Node);
	});
}