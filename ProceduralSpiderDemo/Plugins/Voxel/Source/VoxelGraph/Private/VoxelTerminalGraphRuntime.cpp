// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelTerminalGraphRuntime.h"
#include "VoxelGraph.h"
#include "VoxelMessage.h"
#include "VoxelGraphTracker.h"
#include "VoxelGraphCompiler.h"
#include "VoxelGraphCompileScope.h"
#include "VoxelTerminalGraph.h"
#include "VoxelCompilationGraph.h"
#include "VoxelCompiledTerminalGraph.h"
#include "Nodes/VoxelCallFunctionNodes.h"
#include "Nodes/VoxelNode_FunctionOutput.h"

VOXEL_CONSOLE_VARIABLE(
	VOXELGRAPH_API, bool, GVoxelDisableReconstructOnError, false,
	"voxel.graphs.DisableReconstructOnError",
	"Disable reconstructing voxel graph nodes when graph fails to compile");

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_GAME()
{
	GVoxelMessageManager->OnMessageLogged.AddLambda([](const TSharedRef<FVoxelMessage>& Message)
	{
		check(IsInGameThread());

		for (const UVoxelTerminalGraph* TerminalGraph : Message->GetTypedOuters<UVoxelTerminalGraph>())
		{
			TerminalGraph->GetRuntime().AddMessage(Message);
		}
	});
}
#endif

#if WITH_EDITOR
IVoxelGraphEditorInterface* GVoxelGraphEditorInterface = nullptr;
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelCompiledTerminalGraphRef::AddStructReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_FUNCTION_COUNTER();
	using namespace Voxel::Graph;

	if (!IsSet())
	{
		return;
	}

#if WITH_EDITOR
	const TSharedPtr<FVoxelCompiledTerminalGraph> Graph = GetValue();
	if (!Graph ||
		!ensure(Graph->Graph_DiffingOnly))
	{
		return;
	}

	for (const FNode& Node : Graph->Graph_DiffingOnly->GetNodes())
	{
		ConstCast(Node.GetVoxelNode()).AddStructReferencedObjects(Collector);

		for (const FPin& Pin : Node.GetInputPins())
		{
			FVoxelUtilities::AddStructReferencedObjects(Collector, ConstCast(Pin.GetDefaultValue()));
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const UVoxelGraph& UVoxelTerminalGraphRuntime::GetGraph() const
{
	return GetTerminalGraph().GetGraph();
}

UVoxelTerminalGraph& UVoxelTerminalGraphRuntime::GetTerminalGraph()
{
	return *GetOuterUVoxelTerminalGraph();
}

const UVoxelTerminalGraph& UVoxelTerminalGraphRuntime::GetTerminalGraph() const
{
	return *GetOuterUVoxelTerminalGraph();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const FVoxelSerializedGraph& UVoxelTerminalGraphRuntime::GetSerializedGraph() const
{
#if WITH_EDITOR
	if (!PrivateSerializedGraph.bIsValid ||
		PrivateSerializedGraph.Version != FVoxelSerializedGraph::FVersion::LatestVersion)
	{
		ConstCast(this)->Translate();
	}
#endif

	return PrivateSerializedGraph;
}

void UVoxelTerminalGraphRuntime::EnsureIsCompiled(
	const bool bForce,
	const TFunction<void(FName, FVoxelGraphCompiler&)>& OnGetGraphAtPass)
{
	VOXEL_FUNCTION_COUNTER();

	if (CompiledGraph.IsSet() &&
		!bForce)
	{
		return;
	}

	const TSharedPtr<FVoxelCompiledTerminalGraph> PreviousCompiledGraph = CompiledGraph.IsSet() ? CompiledGraph.GetValue() : nullptr;
	const TSharedPtr<FVoxelCompiledTerminalGraph> NewCompiledGraph = Compile(OnGetGraphAtPass);

#if WITH_EDITOR
	ON_SCOPE_EXIT
	{
		if (PreviousCompiledGraph)
		{
			// Make sure to clear it as AddReferencedObjects won't track this one anymore
			ensure(PreviousCompiledGraph->Graph_DiffingOnly.IsUnique());
			PreviousCompiledGraph->Graph_DiffingOnly.Reset();
		}
	};
#endif

	CompiledGraph.Emplace(NewCompiledGraph);

	// Link functions, but only once CompiledGraph is set to handle recursion
	if (NewCompiledGraph)
	{
		FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("UVoxelTerminalGraphRuntime"));

		for (const TSharedPtr<FVoxelNode>& Node : NewCompiledGraph->NodeRefs)
		{
			if (const TSharedPtr<FVoxelNode_CallFunction> CallFunction = CastStruct<FVoxelNode_CallFunction>(Node))
			{
				CallFunction->Link(DependencyCollector);
			}
		}

		NewCompiledGraph->DependencyTracker = DependencyCollector.Finalize(
			nullptr,
			MakeWeakObjectPtrLambda(this, [this](const FVoxelInvalidationCallstack& Callstack)
			{
				// Recompile if any compilation dependency changed (eg, function calls)
				ensure(WITH_EDITOR);
				EnsureIsCompiled(true);
			}));
	}

#if WITH_EDITOR
	if (IsRunningCookCommandlet())
	{
		return;
	}

	if (!PreviousCompiledGraph.IsValid() &&
		!NewCompiledGraph.IsValid())
	{
		return;
	}

	if (!PreviousCompiledGraph.IsValid() ||
		!NewCompiledGraph.IsValid())
	{
		GVoxelGraphTracker->NotifyTerminalGraphCompiled(GetTerminalGraph());
		return;
	}

	if (!ensure(PreviousCompiledGraph->Graph_DiffingOnly) ||
		!ensure(NewCompiledGraph->Graph_DiffingOnly))
	{
		GVoxelGraphTracker->NotifyTerminalGraphCompiled(GetTerminalGraph());
		return;
	}

	FString Diff;
	if (PreviousCompiledGraph->Graph_DiffingOnly->Identical(*NewCompiledGraph->Graph_DiffingOnly, &Diff))
	{
		return;
	}

	LOG_VOXEL(Verbose, "Updating %s %s: %s",
		*GetGraph().GetPathName(),
		*GetTerminalGraph().GetDisplayName(),
		*Diff);

	GVoxelGraphTracker->NotifyTerminalGraphCompiled(GetTerminalGraph());
#endif
}

#if WITH_EDITOR
void UVoxelTerminalGraphRuntime::BindOnEdGraphChanged()
{
	if (OnEdGraphChangedPtr)
	{
		// Already bound
		return;
	}

	OnEdGraphChangedPtr = MakeSharedVoid();

	GVoxelGraphTracker->OnEdGraphChanged(GetTerminalGraph().GetEdGraph()).Add(FOnVoxelGraphChanged::Make(OnEdGraphChangedPtr, this, [this]
	{
		Translate();
	}));
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelTerminalGraphRuntime::HasNode(const UScriptStruct* Struct) const
{
	VOXEL_FUNCTION_COUNTER();

#if WITH_EDITOR
	// Bypass in editor to avoid recursively migrating
	check(GVoxelGraphEditorInterface);
	return GVoxelGraphEditorInterface->HasNode(GetTerminalGraph(), Struct);
#endif

	for (const auto& It : GetSerializedGraph().NodeNameToNode)
	{
		if (It.Value.VoxelNode &&
			It.Value.VoxelNode.GetScriptStruct()->IsChildOf(Struct))
		{
			return true;
		}
	}

	return false;
}

#if WITH_EDITOR
bool UVoxelTerminalGraphRuntime::HasFunctionInputDefault_EditorOnly(const FGuid& Guid) const
{
	VOXEL_FUNCTION_COUNTER();

	// Bypass in editor to avoid recursively migrating
	check(GVoxelGraphEditorInterface);
	return GVoxelGraphEditorInterface->HasFunctionInputDefault(GetTerminalGraph(), Guid);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelTerminalGraphRuntime::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void UVoxelTerminalGraphRuntime::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	Super::BeginCacheForCookedPlatformData(TargetPlatform);

	if (IsTemplate())
	{
		// UDerivedDataCacheCommandlet calls BeginCacheForCookedPlatformData on templates too
		return;
	}

	if (GetTerminalGraph().PrivateGuid == GVoxelMainTerminalGraphGuid &&
		!GetGraph().HasMainTerminalGraph())
	{
		return;
	}

	// Make sure serialized graph is up-to-date & raise compile errors
	EnsureIsCompiled(true);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelTerminalGraphRuntime::AddMessage(const TSharedRef<FVoxelMessage>& Message)
{
	check(IsInGameThread());

	if (GVoxelGraphCompileScope)
	{
		// Will be logged as compile error
		return;
	}

	RuntimeMessages.Add(Message);
	OnMessagesChanged.Broadcast();
}

bool UVoxelTerminalGraphRuntime::HasCompileMessages() const
{
	// Meaningless otherwise
	ensure(CompiledGraph.IsSet());

	return CompileMessages.Num() > 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelTerminalGraphRuntime::Translate()
{
	VOXEL_FUNCTION_COUNTER();
	check(GVoxelGraphEditorInterface);

	const FVoxelSerializedGraph OldSerializedGraph = MoveTemp(PrivateSerializedGraph);

	PrivateSerializedGraph = GVoxelGraphEditorInterface->Translate(GetTerminalGraph());

	if (OldSerializedGraph.bIsValid ||
		PrivateSerializedGraph.bIsValid)
	{
		// Avoid infinite notify loop, never notify if we were invalid & still are
		GVoxelGraphTracker->NotifyTerminalGraphTranslated(GetTerminalGraph());
	}

	// Clear runtime messages after translating as they're likely out of date
	RuntimeMessages.Reset();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelCompiledTerminalGraph> UVoxelTerminalGraphRuntime::Compile(const TFunction<void(FName, FVoxelGraphCompiler&)>& OnGetGraphAtPass)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	using namespace Voxel::Graph;

#if WITH_EDITOR
	OnTranslatedPtr = MakeSharedVoid();

	const FOnVoxelGraphChanged OnTranslated = FOnVoxelGraphChanged::Make(OnTranslatedPtr, this, [this]
	{
		// Compile again to raise errors
		EnsureIsCompiled(true);
	});

	const FOnVoxelGraphChanged OnForceRecompile = FOnVoxelGraphChanged::Make(OnTranslatedPtr, this, [this]
	{
		// Notify that graph has recompiled
		GVoxelGraphTracker->NotifyTerminalGraphCompiled(GetTerminalGraph());
	});

	GVoxelGraphTracker->OnTerminalGraphTranslated(GetTerminalGraph()).Add(OnTranslated);
#else
	const FOnVoxelGraphChanged OnTranslated = FOnVoxelGraphChanged::Null();
	const FOnVoxelGraphChanged OnForceRecompile = FOnVoxelGraphChanged::Null();
#endif

	TSharedPtr<FVoxelCompiledTerminalGraph> NewCompiledGraph = Compile(
		OnTranslated,
		OnForceRecompile,
		GetTerminalGraph(),
		// In the editor the second Compile will handle error logging
		!WITH_EDITOR,
		CompileMessages,
		OnGetGraphAtPass);

#if WITH_EDITOR
	if (!NewCompiledGraph &&
		!GVoxelDisableReconstructOnError)
	{
		// Mute all notifications to avoid an infinite compile loop
		GVoxelGraphTracker->Mute();

		// Try reconstructing if we errored out
		check(GVoxelGraphEditorInterface);
		GVoxelGraphEditorInterface->ReconstructAllNodes(GetTerminalGraph());

		// Force translate now
		Translate();

		NewCompiledGraph = Compile(
			OnTranslated,
			OnForceRecompile,
			GetTerminalGraph(),
			// Always log errors
			true,
			CompileMessages);

		GVoxelGraphTracker->Unmute();
	}
#endif

	// We updated the compile messages
	OnMessagesChanged.Broadcast();

	return NewCompiledGraph;
}

TSharedPtr<FVoxelCompiledTerminalGraph> UVoxelTerminalGraphRuntime::Compile(
	const FOnVoxelGraphChanged& OnTranslated,
	const FOnVoxelGraphChanged& OnForceRecompile,
	const UVoxelTerminalGraph& TerminalGraph,
	const bool bEnableLogging,
	TVoxelArray<TSharedRef<FVoxelMessage>>& OutCompileMessages,
	const TFunction<void(FName, FVoxelGraphCompiler&)>& OnGetGraphAtPass)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	using namespace Voxel::Graph;

	const FVoxelGraphCompileScope Scope(TerminalGraph, bEnableLogging);
	ON_SCOPE_EXIT
	{
		OutCompileMessages = Scope.GetMessages();
	};

	const TSharedRef<FVoxelGraphCompiler> GraphCompiler = MakeShared<FVoxelGraphCompiler>(TerminalGraph);
	if (!GraphCompiler->LoadSerializedGraph(OnTranslated, OnForceRecompile))
	{
		return nullptr;
	}
	ensure(!Scope.HasError());

	for (const FVoxelGraphCompilerPass& Pass : GVoxelGraphCompilerPasses)
	{
		ensure(!Scope.HasError());
		Pass.Function.Call(*GraphCompiler);

		if (OnGetGraphAtPass)
		{
			OnGetGraphAtPass(Pass.Name, *GraphCompiler);
		}

		if (Scope.HasError())
		{
			return nullptr;
		}

		GraphCompiler->Check();
		if (Scope.HasError())
		{
			return nullptr;
		}
	}

	// Do this at the end, will assert before CheckOutputs if we have multiple outputs
	{
		TVoxelSet<FVoxelGraphNodeRef> VisitedRefs;
		VisitedRefs.Reserve(GraphCompiler->GetNodes().Num());

		for (const FNode& Node : GraphCompiler->GetNodes())
		{
			if (VisitedRefs.Contains(Node.NodeRef))
			{
				ensure(false);
				VOXEL_MESSAGE(Error, "Duplicated node ref {0}", Node.NodeRef);
				continue;
			}

			VisitedRefs.Add(Node.NodeRef);
		}
	}

	if (Scope.HasError())
	{
		return nullptr;
	}

	for (const FNode& Node : GraphCompiler->GetNodes())
	{
		Node.ApplyErrors();
	}

	GraphCompiler->Check();

	if (Scope.HasError())
	{
		return nullptr;
	}

	TVoxelMap<const FNode*, TSharedPtr<FVoxelNode>> NodeToVoxelNode;
	{
		VOXEL_SCOPE_COUNTER("Copy nodes");

		for (FNode& Node : GraphCompiler->GetNodes())
		{
			NodeToVoxelNode.Add_EnsureNew(&Node, Node.GetVoxelNode().MakeSharedCopy());
		}
	}

	struct FNodeInfo
	{
		int32 NodeIndex = -1;
		TVoxelMap<FName, FVoxelNode::FPinRef_Input*> NameToInputPinRefs;
		TVoxelMap<FName, FVoxelNode::FPinRef_Output*> NameToOutputPinRefs;
	};
	TVoxelMap<const FNode*, FNodeInfo> NodeToInfo;

	int32 NumNodes = 0;
	for (const auto& It : NodeToVoxelNode)
	{
		FNodeInfo& Pins = NodeToInfo.Add_CheckNew(It.Key);
		Pins.NodeIndex = NumNodes++;

		It.Value->InitializeNodeRuntime(
			Pins.NodeIndex,
			FVoxelGraphMergedNodeRef(It.Key->NodeRef, It.Key->GetMergedNodeRefs()),
			Pins.NameToInputPinRefs,
			Pins.NameToOutputPinRefs);

		if (Scope.HasError())
		{
			return nullptr;
		}
	}

	// Assign output pins
	int32 NumPins = 0;
	for (const FNode& Node : GraphCompiler->GetNodes())
	{
		const FNodeInfo& NodeInfo = NodeToInfo[&Node];

		for (const FPin& Pin : Node.GetOutputPins())
		{
			FVoxelNode::FPinRef_Output* PinRef = NodeInfo.NameToOutputPinRefs.FindRef(Pin.Name);
			if (!ensure(PinRef))
			{
				VOXEL_MESSAGE(Error, "{0}: Missing pin ref for {1}", Node, Pin.Name);
				return nullptr;
			}

			check(!PinRef->bInitialized);
			PinRef->bInitialized = true;
			PinRef->Type = Pin.Type;
			PinRef->OuterNode = NodeToVoxelNode[&Node].Get();

			if (Pin.GetLinkedTo().Num() == 0)
			{
				// No need to compute
				continue;
			}

			PinRef->PinIndex = NumPins++;
		}
	}

	// Assign input pins
	for (const FNode& Node : GraphCompiler->GetNodes())
	{
		const FNodeInfo& NodeInfo = NodeToInfo[&Node];

		for (const FPin& Pin : Node.GetInputPins())
		{
			FVoxelNode::FPinRef_Input* PinRef = NodeInfo.NameToInputPinRefs.FindRef(Pin.Name);
			if (!ensure(PinRef))
			{
				VOXEL_MESSAGE(Error, "{0}: Missing pin ref for {1}", Node, Pin.Name);
				return nullptr;
			}

			check(!PinRef->bInitialized);
			PinRef->bInitialized = true;
			PinRef->Type = Pin.Type;
			PinRef->OuterNode = NodeToVoxelNode[&Node].Get();

			if (Pin.GetLinkedTo().Num() == 0)
			{
				const FVoxelRuntimePinValue DefaultValue = INLINE_LAMBDA
				{
					if (Pin.Type.HasPinDefaultValue())
					{
						return FVoxelPinType::MakeRuntimeValueFromInnerValue(
							Pin.Type,
							Pin.GetDefaultValue(),
							{});
					}
					else
					{
						return FVoxelRuntimePinValue(Pin.Type);
					}
				};

				if (!ensureVoxelSlow(DefaultValue.IsValid()) ||
					!ensure(DefaultValue.GetType().CanBeCastedTo(Pin.Type)))
				{
					VOXEL_MESSAGE(Error, "{0}: Invalid default value", Pin);
					return nullptr;
				}

				PinRef->DefaultValue = DefaultValue;
				continue;
			}
			check(Pin.GetLinkedTo().Num() == 1);

			const FPin& OutputPin = Pin.GetLinkedTo()[0];
			check(OutputPin.Direction == EPinDirection::Output);

			const FNodeInfo& OtherNodeInfo = NodeToInfo[&OutputPin.Node];

			const FVoxelNode::FPinRef_Output* OtherPinRef = OtherNodeInfo.NameToOutputPinRefs.FindRef(OutputPin.Name);
			if (!ensure(OtherPinRef))
			{
				return nullptr;
			}
			check(OtherPinRef->PinIndex != -1);

			PinRef->LinkedNode = NodeToVoxelNode[&OutputPin.Node].Get();
			PinRef->LinkedNodeIndex = OtherNodeInfo.NodeIndex;
			PinRef->LinkedPinIndex = OtherPinRef->PinIndex;
			PinRef->LinkedPinMetadata = OtherPinRef->Metadata;
		}
	}

	TVoxelMap<UScriptStruct*, TVoxelArray<const FVoxelNode*>> StructToNodes;
	for (const auto& It : NodeToVoxelNode)
	{
		const FVoxelNode& Node = *It.Value;
		if (!Node.CanBeQueried())
		{
			continue;
		}

		for (UScriptStruct* Struct = Node.GetStruct(); Struct != StaticStructFast<FVoxelNode>(); Struct = CastChecked<UScriptStruct>(Struct->GetSuperStruct()))
		{
			StructToNodes.FindOrAdd(Struct).Add(&Node);
		}
	}

	if (TVoxelArray<const FVoxelNode*>* Nodes = StructToNodes.Find(StaticStructFast<FVoxelNode_FunctionOutput>()))
	{
		Nodes->Sort([](const FVoxelNode& A, const FVoxelNode& B)
		{
			return CastStructChecked<FVoxelNode_FunctionOutput>(A).Guid < CastStructChecked<FVoxelNode_FunctionOutput>(B).Guid;
		});
	}

	TVoxelSet<const FVoxelNode*> OwnedNodes;
	{
		OwnedNodes.Reserve(NodeToVoxelNode.Num());

		for (const auto& It : NodeToVoxelNode)
		{
			OwnedNodes.Add(It.Value.Get());
		}
	}

	return MakeShareable(new FVoxelCompiledTerminalGraph(
		TerminalGraph.GetGuid(),
		NumNodes,
		NumPins,
#if WITH_EDITOR
		GraphCompiler->Clone(),
#endif
		NodeToVoxelNode.ValueArray(),
		MoveTemp(OwnedNodes),
		MoveTemp(StructToNodes)));
}