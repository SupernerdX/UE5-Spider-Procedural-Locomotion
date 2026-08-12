// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterActorRuntime.h"
#include "Scatter/VoxelScatterGraph.h"
#include "Scatter/VoxelScatterActor.h"
#include "Scatter/VoxelScatterManager.h"
#include "Scatter/VoxelScatterNodeRuntime.h"
#include "Scatter/VoxelNode_ScatterBase.h"
#include "VoxelNodeEvaluator.h"
#include "VoxelTerminalGraph.h"
#include "VoxelCompiledGraph.h"
#include "VoxelCompiledTerminalGraph.h"
#include "VoxelGraphTracker.h"
#include "VoxelGraphEnvironment.h"
#include "VoxelInvalidationCallstack.h"

TSharedPtr<FVoxelScatterActorRuntime> FVoxelScatterActorRuntime::Create(AVoxelScatterActor& Actor)
{
	VOXEL_FUNCTION_COUNTER();

	UVoxelScatterGraph* Graph = Actor.Graph;
	if (!Graph)
	{
		return {};
	}

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelScatterActorRuntime"));

	const TSharedPtr<FVoxelGraphEnvironment> Environment = FVoxelGraphEnvironment::Create(
		Actor,
		Actor,
		Actor.ActorToWorld(),
		DependencyCollector);

	if (!Environment)
	{
		return {};
	}

	const UVoxelTerminalGraph* TerminalGraph = Graph->GetMainTerminalGraph_CheckBaseGraphs();
	if (!TerminalGraph)
	{
		return {};
	}

	const FVoxelCompiledTerminalGraph* CompiledTerminalGraph = Environment->RootCompiledGraph->FindTerminalGraph(TerminalGraph->GetGuid());
	if (!CompiledTerminalGraph)
	{
		VOXEL_MESSAGE(Error, "Failed to compile {0}", TerminalGraph);
		return {};
	}

	const TConstVoxelArrayView<const FVoxelNode_ScatterBase*> Nodes = CompiledTerminalGraph->GetNodes<FVoxelNode_ScatterBase>();

	TVoxelArray<TVoxelNodeEvaluator<FVoxelNode_ScatterBase>> Evaluators;
	Evaluators.Reserve(Nodes.Num());

	for (const FVoxelNode_ScatterBase* Node : Nodes)
	{
		const TVoxelNodeEvaluator<FVoxelNode_ScatterBase> Evaluator = FVoxelNodeEvaluator::Create<FVoxelNode_ScatterBase>(
			Environment.ToSharedRef(),
			*TerminalGraph,
			*Node);

		if (!ensureVoxelSlow(Evaluator))
		{
			continue;
		}

		Evaluators.Add_EnsureNoGrow(Evaluator);
	}

	const TSharedRef<FVoxelScatterActorRuntime> Runtime = MakeShareable(new FVoxelScatterActorRuntime(Actor, MoveTemp(Evaluators)));
	Runtime->Initialize();

	Runtime->DependencyTracker = DependencyCollector.Finalize(
		nullptr,
		MakeWeakPtrLambda(Runtime, [&Runtime = *Runtime](const FVoxelInvalidationCallstack& Callstack)
		{
			Runtime.Update();
		}));

#if WITH_EDITOR
	GVoxelGraphTracker->OnParameterValueChanged(*Graph).Add(FOnVoxelGraphChanged::Make(Runtime, [&Runtime = *Runtime]
	{
		Runtime.Update();
	}));
#endif

	return Runtime;
}

void FVoxelScatterActorRuntime::Update() const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelInvalidationCallstack> Callstack = FVoxelInvalidationCallstack::Create("FVoxelScatterActorRuntime::Update");

	Voxel::GameTask([WeakActor = WeakActor, Callstack]
	{
		FVoxelInvalidationScope Scope(Callstack);

		if (AVoxelScatterActor* Actor = WeakActor.Resolve())
		{
			Actor->UpdateRuntime();
		}
	});
}

void FVoxelScatterActorRuntime::Destroy()
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());

	const TSharedRef<FVoxelScatterManager> ScatterManager = FVoxelScatterManager::Get(WeakWorld);

	for (const FVoxelScatterNodeWeakRef& NodeRef : NodeRefs)
	{
		ScatterManager->RemoveNode(NodeRef);
	}
	NodeRefs.Reset();
}

FVoxelScatterActorRuntime::~FVoxelScatterActorRuntime()
{
	if (NodeRefs.Num() > 0)
	{
		ensureVoxelSlow(false);
		Destroy();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelScatterActorRuntime::FVoxelScatterActorRuntime(
	AVoxelScatterActor& Actor,
	TVoxelArray<TVoxelNodeEvaluator<FVoxelNode_ScatterBase>>&& Evaluators)
	: WeakWorld(Actor.GetWorld())
	, WeakActor(Actor)
	, Evaluators(MoveTemp(Evaluators))
{
}

void FVoxelScatterActorRuntime::Initialize()
{
	VOXEL_FUNCTION_COUNTER();

	NodeRefs.Reserve(Evaluators.Num());

	const TSharedRef<FVoxelScatterManager> ScatterManager = FVoxelScatterManager::Get(WeakWorld);

	for (const TVoxelNodeEvaluator<FVoxelNode_ScatterBase>& Evaluator : Evaluators)
	{
		const FVoxelScatterNodeWeakRef NodeRef{ WeakActor, Evaluator->NodeGuid };
		NodeRefs.Add_EnsureNoGrow(NodeRef);

		ScatterManager->AddNode(NodeRef, Evaluator);
	}
}