// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelCallFunctionNodes.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "VoxelCompiledGraph.h"
#include "VoxelCompiledTerminalGraph.h"
#include "VoxelFunctionLibraryAsset.h"
#include "VoxelTerminalGraphRuntime.h"
#include "Nodes/VoxelOutputNode.h"
#include "Nodes/VoxelFunctionInputNodes.h"
#include "Nodes/VoxelNode_FunctionOutput.h"

VOXEL_CONSOLE_VARIABLE(
	VOXELGRAPH_API, int32, GVoxelMaxRecursionDepth, 16,
	"voxel.graphs.MaxRecursionDepth",
	"Max function recursion depth in voxel graph");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_CallFunction::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(GuidToInputPinRef.Num() == 0);
	GuidToInputPinRef.Reserve(GetPins().Num());

	ensure(OutputPins.Num() == 0);
	OutputPins.Reserve(GetPins().Num());

	for (const FVoxelPin& Pin : GetPins())
	{
		FGuid Guid;
		if (!ensure(FGuid::ParseExact(Pin.Name.ToString(), EGuidFormats::Digits, Guid)))
		{
			continue;
		}

		if (Pin.bIsInput)
		{
			const TSharedRef<FPinRef_Input> PinRef = MakeShared<FPinRef_Input>(Pin.Name);
			Initializer.InitializePinRef(*PinRef);
			GuidToInputPinRef.Add_EnsureNew(Guid, PinRef);
		}
		else
		{
			OutputPins.Add(FOutputPin
			{
				Guid,
				FPinRef_Output(Pin.Name)
			});
		}
	}

	OutputPins.Sort([](const FOutputPin& A, const FOutputPin& B)
	{
		return A.Guid < B.Guid;
	});

	for (FOutputPin& OutputPin : OutputPins)
	{
		Initializer.InitializePinRef(OutputPin.PinRef);
	}
}

void FVoxelNode_CallFunction::Link(FVoxelDependencyCollector& DependencyCollector)
{
	PinIndexToOutputPinIndex.Reserve(OutputPins.Num());

	for (int32 Index = 0; Index < OutputPins.Num(); Index++)
	{
		const FPinRef_Output& OutputPinRef = OutputPins[Index].PinRef;
		if (!OutputPinRef.ShouldCompute())
		{
			continue;
		}

		PinIndexToOutputPinIndex.Add_EnsureNew(OutputPinRef.PinIndex, Index);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_CallFunction::FixupPinsImpl(
	const UVoxelTerminalGraph* BaseTerminalGraph,
	const FOnVoxelGraphChanged& OnChanged)
{
	VOXEL_FUNCTION_COUNTER();

	for (const auto& It : MakeCopy(GetNameToPin()))
	{
		RemovePin(It.Key);
	}

	if (!BaseTerminalGraph)
	{
		return;
	}
	ensure(!BaseTerminalGraph->HasAnyFlags(RF_NeedLoad));

#if WITH_EDITOR
	GVoxelGraphTracker->OnInputChanged(*BaseTerminalGraph).Add(OnChanged);
	GVoxelGraphTracker->OnOutputChanged(*BaseTerminalGraph).Add(OnChanged);
#endif

	for (const FGuid& Guid : BaseTerminalGraph->GetFunctionInputs())
	{
		const FVoxelGraphFunctionInput& Input = BaseTerminalGraph->FindInputChecked(Guid);
		const FName PinName(Guid.ToString(EGuidFormats::Digits));

		FVoxelPinMetadata Metadata;
#if WITH_EDITOR
		if (!BaseTerminalGraph->GetRuntime().HasFunctionInputDefault_EditorOnly(Guid))
		{
			Metadata.bNoDefault = Input.bNoDefault;
		}
		Metadata.DisplayName = Input.Name.ToString();
#endif

		CreateInputPin(Input.Type, PinName, Metadata);
	}

	for (const FGuid& Guid : BaseTerminalGraph->GetFunctionOutputs())
	{
		const FVoxelGraphFunctionOutput& Output = BaseTerminalGraph->FindOutputChecked(Guid);
		const FName PinName(Guid.ToString(EGuidFormats::Digits));

		CreateOutputPin(Output.Type, PinName);
	}
}

bool FVoxelNode_CallFunction::ComputeImpl(
	const FVoxelGraphQuery Query,
	const int32 PinIndex,
	const FVoxelCompiledGraph& CompiledGraph,
	const FGuid& TerminalGraphGuid) const
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(Query->CompiledTerminalGraph.OwnsNode(this));

	// CallFunction nodes are never fully cached, we rely on the caching of inner nodes and cache per output pin
	checkVoxelSlow(!IsNodeComputed(Query));

	if (Query->GetPinValue(PinIndex).IsValid())
	{
		// That specific pin is already computed
		return true;
	}

	const FVoxelCompiledTerminalGraph* TerminalGraph = CompiledGraph.FindTerminalGraph(TerminalGraphGuid);
	if (!TerminalGraph)
	{
		VOXEL_MESSAGE(Error, "{0}: Failed to find function", this);
		return false;
	}

	const FVoxelGraphQueryImpl::FFunctionCallData FunctionCallData
	{
		*this,
		Query.GetImpl()
	};

#if WITH_EDITOR
	FVoxelGraphCallstack* Callstack = &Query->Context.Callstacks_EditorOnly.Emplace_GetRef();
	Callstack->Node = this;
	Callstack->Parent = Query.GetCallstack();
	Query->Context.CurrentCallstack_EditorOnly = Callstack;
#else
	FVoxelGraphCallstack* Callstack = nullptr;
#endif

	const FVoxelGraphQuery NewQuery(
		Query->GetChild(
			CompiledGraph,
			*TerminalGraph,
			&FunctionCallData),
		Callstack);

	const TConstVoxelArrayView<const FVoxelNode_FunctionOutput*> FunctionOutputs = TerminalGraph->GetNodes<FVoxelNode_FunctionOutput>();
	if (!ensureVoxelSlow(FunctionOutputs.Num() == OutputPins.Num()))
	{
		VOXEL_MESSAGE(Error, "{0}: Outdated function call: missing function output", this);
		return false;
	}

	checkVoxelSlow(Algo::IsSorted(FunctionOutputs, [](const FVoxelNode_FunctionOutput* A, const FVoxelNode_FunctionOutput* B)
	{
		return A->Guid < B->Guid;
	}));

	const int32* OutputPinIndexPtr = PinIndexToOutputPinIndex.Find(PinIndex);
	if (!ensureVoxelSlow(OutputPinIndexPtr))
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid output pin queried", this);
		return false;
	}
	const int32 OutputPinIndex = *OutputPinIndexPtr;

	const FVoxelNode_FunctionOutput& FunctionOutput = *FunctionOutputs[OutputPinIndex];
	const FOutputPin& OutputPin = OutputPins[OutputPinIndex];

	if (!ensureVoxelSlow(FunctionOutput.Guid == OutputPin.Guid))
	{
		VOXEL_MESSAGE(Error, "{0}: Outdated function call", this);
		return false;
	}

	checkVoxelSlow(OutputPin.PinRef.ShouldCompute());

	{
		int32& NumRecursiveCalls = Query->Context.CallFunctionToNumRecursiveCalls.FindOrAdd(this);

		if (NumRecursiveCalls >= GVoxelMaxRecursionDepth)
		{
			VOXEL_MESSAGE(Error, "{0}: Max recursion depth reached", this);
			return false;
		}

		NumRecursiveCalls++;
	}

	const FValue Value = FunctionOutput.ValuePin.Get(NewQuery);

	VOXEL_GRAPH_WAIT(OutputPinIndex, Value)
	{
		{
			int32& NumRecursiveCalls = Query->Context.CallFunctionToNumRecursiveCalls.FindChecked(this);
			NumRecursiveCalls--;
			checkVoxelSlow(NumRecursiveCalls >= 0);
		}

		OutputPins[OutputPinIndex].PinRef.Set(Query, Value);
	};

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_CallMemberFunction::Link(FVoxelDependencyCollector& DependencyCollector)
{
	Super::Link(DependencyCollector);

	if (!ContextOverride)
	{
		return;
	}

	ContextOverrideCompiledGraph = ContextOverride->GetCompiledGraph(DependencyCollector);
}

void FVoxelNode_CallMemberFunction::FixupPins(
	const UVoxelGraph& Context,
	const FOnVoxelGraphChanged& OnChanged,
	const FOnVoxelGraphChanged& OnForceRecompile)
{
	if (ContextOverride &&
		ContextOverride != &Context)
	{
		FixupPins(*ContextOverride, OnChanged, OnForceRecompile);
		return;
	}

#if WITH_EDITOR
	GVoxelGraphTracker->OnTerminalGraphChanged(Context).Add(OnChanged);
#endif

	const UVoxelTerminalGraph* TerminalGraph = Context.FindTerminalGraph(Guid);
	if (!TerminalGraph)
	{
		FixupPinsImpl(nullptr, OnChanged);
		return;
	}

	FixupPinsImpl(TerminalGraph, OnChanged);
}

void FVoxelNode_CallMemberFunction::ComputeIfNeeded(
	const FVoxelGraphQuery Query,
	const int32 PinIndex) const
{
	if (ContextOverride)
	{
		if (!ContextOverrideCompiledGraph)
		{
			VOXEL_MESSAGE(Error, "{0}: Failed to compile function", this);
			EnsureOutputValuesAreSet(Query);
			return;
		}

		if (ComputeImpl(Query, PinIndex, *ContextOverrideCompiledGraph, Guid))
		{
			return;
		}
	}
	else
	{
		if (ComputeImpl(Query, PinIndex, Query->CompiledGraph, Guid))
		{
			return;
		}
	}

	EnsureOutputValuesAreSet(Query);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_CallExternalFunction::Link(FVoxelDependencyCollector& DependencyCollector)
{
	Super::Link(DependencyCollector);

	if (!FunctionLibrary)
	{
		return;
	}

	CompiledGraph = FunctionLibrary->GetGraph().GetCompiledGraph(DependencyCollector);
}

void FVoxelNode_CallExternalFunction::FixupPins(
	const UVoxelGraph& Context,
	const FOnVoxelGraphChanged& OnChanged,
	const FOnVoxelGraphChanged& OnForceRecompile)
{
	if (!FunctionLibrary)
	{
		FixupPinsImpl(nullptr, OnChanged);
		return;
	}

	const UVoxelTerminalGraph* TerminalGraph = FunctionLibrary->GetGraph().FindTerminalGraph(Guid);
	if (!TerminalGraph)
	{
		FixupPinsImpl(nullptr, OnChanged);
		return;
	}

	FixupPinsImpl(TerminalGraph, OnChanged);

#if WITH_EDITOR
	GVoxelGraphTracker->OnTerminalGraphTranslated(*TerminalGraph).Add(OnForceRecompile);
#endif
}

void FVoxelNode_CallExternalFunction::ComputeIfNeeded(
	const FVoxelGraphQuery Query,
	const int32 PinIndex) const
{
	if (!FunctionLibrary)
	{
		VOXEL_MESSAGE(Error, "{0}: FunctionLibrary is null", this);
		EnsureOutputValuesAreSet(Query);
		return;
	}

	if (!CompiledGraph)
	{
		VOXEL_MESSAGE(Error, "{0}: Failed to compile function", this);
		EnsureOutputValuesAreSet(Query);
		return;
	}

	if (ComputeImpl(Query, PinIndex, *CompiledGraph, Guid))
	{
		return;
	}

	EnsureOutputValuesAreSet(Query);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_CallParentMainGraph::Initialize(FInitializer& Initializer)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(OutputPins.Num() == 0);
	OutputPins.Reserve(GetPins().Num());

	for (const FVoxelPin& Pin : GetPins())
	{
		if (ensure(!Pin.bIsInput))
		{
			OutputPins.Add(FOutputPin
			{
				FGuid(),
				FPinRef_Output(Pin.Name)
			});
		}
	}

	for (FOutputPin& OutputPin : OutputPins)
	{
		Initializer.InitializePinRef(OutputPin.PinRef);
	}
}

void FVoxelNode_CallParentMainGraph::ComputeIfNeeded(
	const FVoxelGraphQuery Query,
	const int32 PinIndex) const
{
	if (!ContextOverrideCompiledGraph)
	{
		VOXEL_MESSAGE(Error, "{0}: Failed to compile function", this);
		EnsureOutputValuesAreSet(Query);
		return;
	}

	if (ComputeImpl(Query, PinIndex, *ContextOverrideCompiledGraph))
	{
		return;
	}

	EnsureOutputValuesAreSet(Query);
}

void FVoxelNode_CallParentMainGraph::Link(FVoxelDependencyCollector& DependencyCollector)
{
	Super::Link(DependencyCollector);

	if (!ContextOverride)
	{
		return;
	}

	ContextOverrideCompiledGraph = ContextOverride->GetCompiledGraph(DependencyCollector);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_CallParentMainGraph::FixupPins(
	const UVoxelGraph& Context,
	const FOnVoxelGraphChanged& OnChanged,
	const FOnVoxelGraphChanged& OnForceRecompile)
{
	VOXEL_FUNCTION_COUNTER();

	for (const auto& It : MakeCopy(GetNameToPin()))
	{
		RemovePin(It.Key);
	}

	if (!ContextOverride)
	{
		return;
	}

#if WITH_EDITOR
	GVoxelGraphTracker->OnTerminalGraphChanged(*ContextOverride).Add(OnChanged);
#endif

	if (!ContextOverride->HasMainTerminalGraph())
	{
		return;
	}

	const UVoxelTerminalGraph& BaseTerminalGraph = ContextOverride->GetMainTerminalGraph();
	ensure(!BaseTerminalGraph.HasAnyFlags(RF_NeedLoad));

	BaseTerminalGraph.GetRuntime().EnsureIsCompiled(false);

	const FVoxelSerializedGraph& SerializedGraph = BaseTerminalGraph.GetRuntime().GetSerializedGraph();

	if (SerializedGraph.OutputNodeName.IsNone())
	{
		return;
	}

	const FVoxelSerializedNode* OutputNode = SerializedGraph.NodeNameToNode.Find(SerializedGraph.OutputNodeName);
	if (!OutputNode)
	{
		return;
	}

	for (const auto& It : OutputNode->InputPins)
	{
		CreateOutputPin(It.Value.Type, It.Key, {});
	}
}

bool FVoxelNode_CallParentMainGraph::ComputeImpl(
	const FVoxelGraphQuery Query,
	const int32 PinIndex,
	const FVoxelCompiledGraph& CompiledGraph) const
{
	VOXEL_FUNCTION_COUNTER();
	checkVoxelSlow(Query->CompiledTerminalGraph.OwnsNode(this));

	// CallFunction nodes are never fully cached, we rely on the caching of inner nodes and cache per output pin
	checkVoxelSlow(!IsNodeComputed(Query));

	if (Query->GetPinValue(PinIndex).IsValid())
	{
		// That specific pin is already computed
		return true;
	}

	const FVoxelCompiledTerminalGraph* TerminalGraph = CompiledGraph.FindTerminalGraph(GVoxelMainTerminalGraphGuid);
	if (!TerminalGraph)
	{
		VOXEL_MESSAGE(Error, "{0}: Failed to find function", this);
		return false;
	}

	const FVoxelGraphQueryImpl::FFunctionCallData FunctionCallData
	{
		*this,
		Query.GetImpl()
	};

#if WITH_EDITOR
	FVoxelGraphCallstack* Callstack = &Query->Context.Callstacks_EditorOnly.Emplace_GetRef();
	Callstack->Node = this;
	Callstack->Parent = Query.GetCallstack();
	Query->Context.CurrentCallstack_EditorOnly = Callstack;
#else
	FVoxelGraphCallstack* Callstack = nullptr;
#endif

	const FVoxelGraphQuery NewQuery(
		Query->GetChild(
			CompiledGraph,
			*TerminalGraph,
			&FunctionCallData),
		Callstack);

	const TConstVoxelArrayView<const FVoxelOutputNode*> OutputNodes = TerminalGraph->GetNodes<FVoxelOutputNode>();
	if (!ensureVoxelSlow(OutputNodes.Num() > 0))
	{
		VOXEL_MESSAGE(Error, "{0}: Outdated function call: missing parent output node", this);
		return false;
	}

	if (!ensureVoxelSlow(OutputNodes.Num() == 1))
	{
		VOXEL_MESSAGE(Error, "{0}: Outdated function call: multiple parent output nodes", this);
		return false;
	}

	const FVoxelOutputNode* OutputNode = OutputNodes[0];
	checkVoxelSlow(OutputNode);

	const int32* OutputPinIndexPtr = PinIndexToOutputPinIndex.Find(PinIndex);
	if (!ensureVoxelSlow(OutputPinIndexPtr))
	{
		VOXEL_MESSAGE(Error, "{0}: Invalid output pin queried", this);
		return false;
	}
	const int32 OutputPinIndex = *OutputPinIndexPtr;

	const FOutputPin& OutputPin = OutputPins[OutputPinIndex];

	const FPinRef_Input* InputPin = OutputNode->NameToInputPin.FindRef(OutputPin.PinRef.GetName());
	if (!ensure(InputPin))
	{
		VOXEL_MESSAGE(Error, "{0}: Outdated parent call", this);
		return false;
	}

	checkVoxelSlow(OutputPin.PinRef.ShouldCompute());

	{
		int32& NumRecursiveCalls = Query->Context.CallFunctionToNumRecursiveCalls.FindOrAdd(this);

		if (NumRecursiveCalls >= GVoxelMaxRecursionDepth)
		{
			VOXEL_MESSAGE(Error, "{0}: Max recursion depth reached", this);
			return false;
		}

		NumRecursiveCalls++;
	}

	const FValue Value = InputPin->Get(NewQuery);

	VOXEL_GRAPH_WAIT(OutputPinIndex, Value)
	{
		{
			int32& NumRecursiveCalls = Query->Context.CallFunctionToNumRecursiveCalls.FindChecked(this);
			NumRecursiveCalls--;
			checkVoxelSlow(NumRecursiveCalls >= 0);
		}

		OutputPins[OutputPinIndex].PinRef.Set(Query, Value);
	};

	return true;
}