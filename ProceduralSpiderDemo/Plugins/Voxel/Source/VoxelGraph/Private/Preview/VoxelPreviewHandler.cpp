// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Preview/VoxelPreviewHandler.h"
#include "VoxelGraph.h"
#include "VoxelNodeEvaluator.h"
#include "VoxelTerminalGraph.h"
#include "VoxelCompiledGraph.h"
#include "VoxelCompiledTerminalGraph.h"
#include "Nodes/VoxelPreviewDataNode.h"
#include "Nodes/VoxelNode_ValueDebug.h"
#include "VoxelGraphPositionParameter.h"
#include "Preview/VoxelNode_Preview.h"

TVoxelArray<TSharedRef<const FVoxelPreviewHandler>> GVoxelPreviewHandlers;

VOXEL_RUN_ON_STARTUP_GAME()
{
	GOnVoxelModuleUnloaded_DoCleanup.AddLambda([]
	{
		GVoxelPreviewHandlers.Empty();
	});
}

TConstVoxelArrayView<TSharedRef<const FVoxelPreviewHandler>> FVoxelPreviewHandler::GetHandlers()
{
	if (GVoxelPreviewHandlers.Num() == 0)
	{
		for (const UScriptStruct* Struct : GetDerivedStructs<FVoxelPreviewHandler>())
		{
			GVoxelPreviewHandlers.Add(MakeSharedStruct<FVoxelPreviewHandler>(Struct));
		}
	}

	return GVoxelPreviewHandlers;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelFuture<TVoxelArray<uint8>> FVoxelPreviewHandler::Compute(UVoxelTerminalGraph& TerminalGraph)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelDependencyCollector> DependencyCollector = MakeShared<FVoxelDependencyCollector>(STATIC_FNAME("FVoxelPreviewHandler::Compute"));

	const TSharedPtr<FVoxelGraphEnvironment> Environment = FVoxelGraphEnvironment::CreatePreview(
		TerminalGraph.GetGraph(),
		TerminalGraph.GetGraph(),
		FTransform::Identity,
		*DependencyCollector);

	if (!Environment)
	{
		return {};
	}

	const FVoxelCompiledTerminalGraph* CompiledTerminalGraph = Environment->RootCompiledGraph->FindTerminalGraph(TerminalGraph.GetGuid());
	if (!CompiledTerminalGraph)
	{
		return {};
	}

	const TConstVoxelArrayView<const FVoxelNode_Preview*> Nodes = CompiledTerminalGraph->GetNodes<FVoxelNode_Preview>();
	if (Nodes.Num() == 0)
	{
		return {};
	}

	if (Nodes.Num() > 1)
	{
		VOXEL_MESSAGE(Error, "More than one preview node: {0}", Nodes);
		return {};
	}

	const TVoxelNodeEvaluator<FVoxelNode_Preview> Evaluator = FVoxelNodeEvaluator::Create<FVoxelNode_Preview>(
		Environment.ToSharedRef(),
		TerminalGraph,
		*Nodes[0]);

	if (!ensureVoxelSlow(Evaluator))
	{
		return {};
	}

	const FVoxelCompiledTerminalGraph* CompiledEditorTerminalGraph = Environment->RootCompiledGraph->FindTerminalGraph(GVoxelEditorTerminalGraphGuid);
	TVoxelArray<const FVoxelPreviewDataNode*> QueryParameterNodes;
	if (CompiledEditorTerminalGraph)
	{
		for (const FVoxelPreviewDataNode* Node : CompiledEditorTerminalGraph->GetNodes<FVoxelPreviewDataNode>())
		{
			QueryParameterNodes.Add(Node);
		}
	}

	return Voxel::AsyncTask(MakeStrongPtrLambda(this, [=, this]() -> TVoxelFuture<TVoxelArray<uint8>>
	{
		FVoxelVectorBuffer Positions;
		Positions.Allocate(PreviewSize * PreviewSize);

		Voxel::ParallelFor(PreviewSize, [&](const int32 Y)
		{
			for (int32 X = 0; X < PreviewSize; X++)
			{
				const int32 Index = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, X, Y);
				const FVector Position = LocalToWorld.TransformPosition(FVector(X, Y, 0));

				Positions.Set(Index, FVector3f(Position));
			}
		});

		FVoxelGraphContext Context = Evaluator.MakeContext(*DependencyCollector);
		FVoxelGraphQueryImpl& Query = Context.MakeQuery();

		Query.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(Positions);

		for (const FVoxelPreviewDataNode* Node : QueryParameterNodes)
		{
			const FVoxelGraphQueryImpl::FFunctionCallData FunctionCallData
			{
				*Node,
				Query
			};

			const FVoxelGraphQueryImpl& NewQuery = Query.GetChild(
				*Environment->RootCompiledGraph,
				*CompiledEditorTerminalGraph,
				&FunctionCallData);
			Node->Query_WithPosition(NewQuery, Query);
		}

		const FVoxelRuntimePinValue Value = Evaluator->ValuePin.GetSynchronous(Query);
		if (!ensureVoxelSlow(SupportsType(Value.GetType())))
		{
			return {};
		}

		{
			VOXEL_SCOPE_LOCK(DependencyTrackers_CriticalSection);

			DependencyTrackers_RequiresLock.Add(DependencyCollector->Finalize(nullptr, [OnInvalidated = OnInvalidated](const FVoxelInvalidationCallstack&)
			{
				OnInvalidated();
			}));
		}

		Initialize(Value);

		TVoxelArray<FLinearColor> Colors;
		FVoxelUtilities::SetNum(Colors, PreviewSize * PreviewSize);
		GetColors(Colors);

		const TSharedRef<TVoxelArray<uint8>> Bytes = MakeShared<TVoxelArray<uint8>>();
		FVoxelUtilities::SetNumFast(*Bytes, PreviewSize * PreviewSize * sizeof(FColor));
		{
			VOXEL_SCOPE_COUNTER("Write colors");

			const TVoxelArrayView<FColor> FinalColors = MakeVoxelArrayView(*Bytes).ReinterpretAs<FColor>();

			Voxel::ParallelFor(PreviewSize, [&](const int32 Y)
			{
				const int32 ReadIndex = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, 0, Y);
				const int32 WriteIndex = FVoxelUtilities::Get2DIndex<int32>(PreviewSize, 0, PreviewSize - 1 - Y);
				for (int32 X = 0; X < PreviewSize; X++)
				{
					checkVoxelSlow(ReadIndex + X == FVoxelUtilities::Get2DIndex<int32>(PreviewSize, X, Y));
					checkVoxelSlow(WriteIndex + X == FVoxelUtilities::Get2DIndex<int32>(PreviewSize, X, PreviewSize - 1 - Y));

					FLinearColor Color = Colors[ReadIndex + X];
					Color.A = 1.f;
					FinalColors[WriteIndex + X] = Color.ToFColor(false);
				}
			});
		}

		return Bytes;
	}));
}

FVoxelFuture FVoxelPreviewHandler::ComputeAtMousePosition(UVoxelTerminalGraph& TerminalGraph)
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelDependencyCollector> DependencyCollector = MakeShared<FVoxelDependencyCollector>(STATIC_FNAME("FVoxelPreviewHandler::ComputeAtMousePosition"));

	const TSharedPtr<FVoxelGraphEnvironment> Environment = FVoxelGraphEnvironment::CreatePreview(
		TerminalGraph.GetGraph(),
		TerminalGraph.GetGraph(),
		FTransform::Identity,
		*DependencyCollector);

	if (!Environment)
	{
		return {};
	}

	const FVoxelCompiledTerminalGraph* CompiledTerminalGraph = Environment->RootCompiledGraph->FindTerminalGraph(TerminalGraph.GetGuid());
	if (!CompiledTerminalGraph)
	{
		return {};
	}

	const TConstVoxelArrayView<const FVoxelNode_Preview*> Nodes = CompiledTerminalGraph->GetNodes<FVoxelNode_Preview>();
	if (Nodes.Num() == 0)
	{
		return {};
	}

	if (Nodes.Num() > 1)
	{
		VOXEL_MESSAGE(Error, "More than one preview node: {0}", Nodes);
		return {};
	}

	const TVoxelNodeEvaluator<FVoxelNode_Preview> Evaluator = FVoxelNodeEvaluator::Create<FVoxelNode_Preview>(
		Environment.ToSharedRef(),
		TerminalGraph,
		*Nodes[0]);

	if (!ensureVoxelSlow(Evaluator))
	{
		return {};
	}

	const FVoxelCompiledTerminalGraph* CompiledEditorTerminalGraph = Environment->RootCompiledGraph->FindTerminalGraph(GVoxelEditorTerminalGraphGuid);
	TVoxelArray<const FVoxelPreviewDataNode*> QueryParameterNodes;
	if (CompiledEditorTerminalGraph)
	{
		for (const FVoxelPreviewDataNode* Node : CompiledEditorTerminalGraph->GetNodes<FVoxelPreviewDataNode>())
		{
			QueryParameterNodes.Add(Node);
		}
	}

	return Voxel::AsyncTask(MakeStrongPtrLambda(this, [=, this]
	{
		const FVector Position = LocalToWorld.TransformPosition(FVector(MousePosition.X, MousePosition.Y, 0.f));

		FVoxelGraphContext Context = Evaluator.MakeContext(*DependencyCollector);
		FVoxelGraphQueryImpl& Query = Context.MakeQuery();

		Query.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(Position);
		Query.AddParameter<FVoxelGraphParameters::FDebugValuePosition>().bFullValue = bShowFullValue;

		for (const FVoxelPreviewDataNode* Node : QueryParameterNodes)
		{
			const FVoxelGraphQueryImpl::FFunctionCallData FunctionCallData
			{
				*Node,
				Query
			};

			const FVoxelGraphQueryImpl& NewQuery = Query.GetChild(
				*Environment->RootCompiledGraph,
				*CompiledEditorTerminalGraph,
				&FunctionCallData);
			Node->Query_WithPosition(NewQuery, Query);
		}

		(void)Evaluator->ValuePin.GetSynchronous(Query);

		VOXEL_SCOPE_LOCK(DependencyTrackers_CriticalSection);

		DependencyTrackers_RequiresLock.Add(DependencyCollector->Finalize(nullptr, [OnInvalidated = OnInvalidated](const FVoxelInvalidationCallstack&)
		{
			OnInvalidated();
		}));
	}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelPreviewHandler::SetMousePosition(const FVector2D& NewMousePosition)
{
	MousePosition = NewMousePosition.RoundToVector();
}

void FVoxelPreviewHandler::BuildStats(const FAddStat& AddStat)
{
	const auto ValueToString = [this](const FString& Prefix, const float Value)
	{
		FString Result;
		if (bShowFullDistance)
		{
			Result = LexToSanitizedString(Value);
		}
		else
		{
			Result = FVoxelUtilities::DistanceToString(Value);
		}

		if (Value >= 0.f)
		{
			Result = " " + Result;
		}

		return Prefix + Result;
	};

	AddStat(
		"Position",
		false,
		MakeWeakPtrLambda(this, [this]
		{
			const FVector Position = LocalToWorld.TransformPosition(FVector(MousePosition.X, MousePosition.Y, 0.f));
			return Position.ToString();
		}),
		MakeWeakPtrLambda(this, [this, ValueToString]() -> TArray<FString>
		{
			const FVector Position = LocalToWorld.TransformPosition(FVector(MousePosition.X, MousePosition.Y, 0.f));
			return {
				ValueToString("X=", Position.X),
				ValueToString("Y=", Position.Y),
				ValueToString("Z=", Position.Z)
			};
		}));
}