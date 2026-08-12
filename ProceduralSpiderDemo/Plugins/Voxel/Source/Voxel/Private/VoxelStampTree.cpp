// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampTree.h"
#include "VoxelTaskContext.h"

TUniquePtr<FVoxelStampTree::FIterator> FVoxelStampTree::CreateIterator(
	const FVoxelQuery& Query,
	const FVoxelBox& Bounds) const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelInlineArray<int32, 128> Indices;

	ForeachElementIndex_Unsorted(Query.DependencyCollector, Bounds, [&](const int32 Index)
	{
		checkVoxelSlow(!Indices.Contains(Index));
		Indices.Add(Index);
		return EVoxelIterate::Continue;
	});

	Indices.Sort();

	TUniquePtr<FIterator> Iterator = MakeUnique<FIterator>();
	Iterator->Stamps.Reserve(Indices.Num());
	Iterator->ElementStorage.Reserve(Indices.Num());

	int32 Index = 0;
	while (Index < Indices.Num())
	{
		const FVoxelStampTreeElement& FirstElement = Elements[Indices[Index++]];
		const FVoxelStampRuntime& Stamp = *FirstElement.Stamp;

		if (Query.ShouldStopTraversal_BeforeStamp &&
			(*Query.ShouldStopTraversal_BeforeStamp)(Stamp))
		{
			break;
		}

		const int32 StartIndex = Iterator->ElementStorage.Add_EnsureNoGrow(&FirstElement);

		while (Index < Indices.Num())
		{
			const FVoxelStampTreeElement& NextElement = Elements[Indices[Index]];
			if (NextElement.Stamp != &Stamp)
			{
				break;
			}

			Iterator->ElementStorage.Add_EnsureNoGrow(&NextElement);
			Index++;
		}

		const int32 EndIndex = Iterator->ElementStorage.Num();

		Iterator->Stamps.Add_EnsureNoGrow(FStamp
		{
			&Stamp,
			Iterator->ElementStorage.View().Slice(StartIndex, EndIndex - StartIndex)
		});

		if (Query.ShouldStopTraversal_AfterStamp &&
			(*Query.ShouldStopTraversal_AfterStamp)(Stamp))
		{
			break;
		}
	}

	return Iterator;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStampTreeManager* GVoxelStampTreeManager = new FVoxelStampTreeManager();

void FVoxelStampTreeManager::AddTask(TVoxelUniqueFunction<void()> Lambda)
{
	VOXEL_FUNCTION_COUNTER();

	{
		VOXEL_SCOPE_LOCK(CriticalSection);
		Tasks_RequiresLock.Enqueue(MoveTemp(Lambda));
	}

	FVoxelTaskScope Scope(*GVoxelGlobalTaskContext);

	Voxel::AsyncTask([]
	{
		GVoxelStampTreeManager->ProcessTask();
	});
}

void FVoxelStampTreeManager::ProcessTask()
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelUniqueFunction<void()> Task;
	{
		VOXEL_SCOPE_LOCK(CriticalSection);

		if (!Tasks_RequiresLock.Dequeue(Task))
		{
			return;
		}
	}

	Task();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelFutureStampTree::Set(const TSharedRef<FVoxelStampTree>& NewTree)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);

	check(!Tree);
	check(!PrivateIsComplete.Get());

	Tree = NewTree;
	PrivateIsComplete.Set(true);

	for (TVoxelUniqueFunction<void(const TSharedRef<FVoxelStampTree>&)>& Continuation : Continuations_RequiresLock)
	{
		GVoxelStampTreeManager->AddTask([Continuation = MoveTemp(Continuation), NewTree]
		{
			Continuation(NewTree);
		});
	}
	Continuations_RequiresLock.Empty();
}

void FVoxelFutureStampTree::Then(TVoxelUniqueFunction<void(const TSharedRef<FVoxelStampTree>&)> Lambda)
{
	VOXEL_SCOPE_LOCK(CriticalSection);

	if (IsComplete())
	{
		GVoxelStampTreeManager->AddTask([Lambda = MoveTemp(Lambda), Tree = Tree.ToSharedRef()]
		{
			Lambda(Tree);
		});
	}
	else
	{
		Continuations_RequiresLock.Add(MoveTemp(Lambda));
	}
}

FVoxelStampTree& FVoxelFutureStampTree::GetTree_Impl() const
{
	VOXEL_FUNCTION_COUNTER();

	while (!IsComplete())
	{
		GVoxelStampTreeManager->ProcessTask();
	}

	return *Tree;
}