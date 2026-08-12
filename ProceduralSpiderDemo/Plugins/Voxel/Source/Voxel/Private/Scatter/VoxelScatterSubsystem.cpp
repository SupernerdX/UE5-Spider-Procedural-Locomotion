// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterSubsystem.h"
#include "VoxelScatterInvokersManager.h"
#include "Scatter/VoxelScatterManager.h"
#include "Scatter/VoxelNode_ScatterBase.h"
#include "Scatter/VoxelScatterNodeRuntime.h"
#include "Scatter/VoxelScatterCacheManager.h"

void FVoxelScatterSubsystem::LoadFromPrevious(FVoxelSubsystem& InPreviousSubsystem)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelScatterSubsystem& PreviousSubsystem = CastStructChecked<FVoxelScatterSubsystem>(InPreviousSubsystem);

	PreviousNodeRefToRuntime = MoveTemp(PreviousSubsystem.NodeRefToRuntime);
	ensure(PreviousSubsystem.PreviousNodeRefToRuntime.Num() == 0);

	CacheManager = MoveTemp(PreviousSubsystem.CacheManager);

	if (InvokerView &&
		!InvokerView->Equal(*this, PreviousSubsystem))
	{
		InvokerView = nullptr;
	}
}

void FVoxelScatterSubsystem::Initialize()
{
	VOXEL_FUNCTION_COUNTER();

	if (!GetConfig().bRenderScatterActors)
	{
		return;
	}

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelScatterSubsystem"));

	ON_SCOPE_EXIT
	{
		DependencyTracker = Finalize(DependencyCollector);
	};

	const TSharedRef<FVoxelScatterManager> ScatterManager = FVoxelScatterManager::Get(GetConfig().World);
	const TVoxelMap<FVoxelScatterNodeWeakRef, TVoxelNodeEvaluator<FVoxelNode_ScatterBase>>& NodeRefToEvaluator = ScatterManager->GetNodeRefToEvaluator(DependencyCollector);

	NodeRefToRuntime.Reserve(NodeRefToEvaluator.Num());

	if (!CacheManager)
	{
		CacheManager = MakeShared<FVoxelScatterCacheManager>();
	}

	for (const auto& It : NodeRefToEvaluator)
	{
		if (const TSharedPtr<FVoxelScatterNodeRuntime> Runtime = PreviousNodeRefToRuntime.FindRef(It.Key))
		{
			if (!Runtime->IsInvalidated() &&
				Runtime->GetEvaluator().Equals_EnvironmentPtr(It.Value))
			{
				NodeRefToRuntime.Add_EnsureNew(It.Key, Runtime);
				ensure(PreviousNodeRefToRuntime.Remove(It.Key));
				continue;
			}
		}

		const TSharedRef<FVoxelScatterNodeRuntime> Runtime = It.Value->MakeRuntime();
		Runtime->Initialize(*this, It.Key, It.Value);
		NodeRefToRuntime.Add_EnsureNew(It.Key, Runtime);
		CacheManager->ResetCache(It.Key.Actor);
	}

	if (!InvokerView)
	{
		InvokerView = FVoxelScatterInvokersManager::Get(GetConfig().World)->MakeView(*this);
	}
}

void FVoxelScatterSubsystem::Compute()
{
	VOXEL_FUNCTION_COUNTER();

	if (!InvokerView)
	{
		return;
	}

	const TSharedRef<const TVoxelArray<FVector>> Invokers = INLINE_LAMBDA
	{
		FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelScatterSubsystem Invokers"));

		const TSharedRef<const TVoxelArray<FVector>> Result = InvokerView->GetInvokers(DependencyCollector);
		InvokersDependencyTracker = Finalize(DependencyCollector);
		return Result;
	};

	for (const auto& It : NodeRefToRuntime)
	{
		Voxel::AsyncTask([this, Runtime = It.Value, Invokers]
		{
			Runtime->Compute(
				*this,
				*Invokers);
		});
	}
}

void FVoxelScatterSubsystem::Render(FVoxelRuntime& Runtime)
{
	VOXEL_FUNCTION_COUNTER();

	for (const auto& It : NodeRefToRuntime)
	{
		It.Value->Render(Runtime);
	}

	for (const auto& It : PreviousNodeRefToRuntime)
	{
		It.Value->Destroy(Runtime);
	}
	PreviousNodeRefToRuntime.Empty();
}