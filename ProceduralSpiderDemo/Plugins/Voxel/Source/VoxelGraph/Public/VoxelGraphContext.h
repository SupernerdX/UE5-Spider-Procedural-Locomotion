// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelGraphEnvironment.h"

class FVoxelCompiledGraph;
class FVoxelCompiledTerminalGraph;
struct FVoxelNode;
struct FVoxelNode_CallFunction;
struct FVoxelGraphQueryImpl;

struct FVoxelGraphTask
{
	FVoxelGraphTask* NextTask;
	void (*Execute)(void*);
	uint8 LambdaData[240];
};
checkStatic(sizeof(FVoxelGraphTask) == 256);

struct FVoxelGraphCallstack
{
	const FVoxelNode* Node = nullptr;
	const FVoxelGraphCallstack* Parent = nullptr;
};

extern VOXELGRAPH_API const uint32 GVoxelGraphContextTLS;

class VOXELGRAPH_API FVoxelGraphContext
{
public:
	const FVoxelGraphEnvironment& Environment;
	const FVoxelCompiledTerminalGraph& TerminalGraph;
	FVoxelDependencyCollector& DependencyCollector;

	TVoxelMap<const FVoxelNode_CallFunction*, int32> CallFunctionToNumRecursiveCalls;

	FVoxelGraphContext(
		const FVoxelGraphEnvironment& Environment,
		const FVoxelCompiledTerminalGraph& TerminalGraph,
		FVoxelDependencyCollector& DependencyCollector);
	~FVoxelGraphContext();
	UE_NONCOPYABLE(FVoxelGraphContext);

	FORCEINLINE static FVoxelGraphContext* Get()
	{
		return static_cast<FVoxelGraphContext*>(FPlatformTLS::GetTlsValue(GVoxelGraphContextTLS));
	}

public:
#if WITH_EDITOR
	const FVoxelGraphCallstack* CurrentCallstack_EditorOnly = nullptr;
	TVoxelChunkedArray<FVoxelGraphCallstack> Callstacks_EditorOnly;
#endif

public:
	void Execute();

public:
	FVoxelGraphQueryImpl& MakeQuery();

	template<typename LambdaType>
	FORCEINLINE void AddTask(LambdaType&& Lambda)
	{
		checkStatic(alignof(LambdaType) <= 8);
		static_assert(sizeof(LambdaType) <= sizeof(FVoxelGraphTask::LambdaData), "Lambda is too big");

		FVoxelGraphTask& Task = INLINE_LAMBDA -> FVoxelGraphTask&
		{
			if (TaskPool.Num() > 0)
			{
				return *TaskPool.Pop();
			}

			return TaskStorage.Emplace_GetRef();
		};

		Task.Execute = [](void* LambdaData)
		{
			static_cast<LambdaType*>(LambdaData)->operator()();
			static_cast<LambdaType*>(LambdaData)->~LambdaType();
		};

		new(Task.LambdaData) LambdaType(MoveTemp(Lambda));

		// AddTask calls within a task should be added sequentially at the start of the linked list

		if (TaskToInsertAfter)
		{
			// Insert ourselves after the last task queued
			Task.NextTask = TaskToInsertAfter->NextTask;
			TaskToInsertAfter->NextTask = &Task;
		}
		else
		{
			// We're the first task queued, replace the first task with ourselves
			Task.NextTask = NextTask;
			NextTask = &Task;
		}

		TaskToInsertAfter = &Task;
	}

public:
	FORCEINLINE void* Allocate(int32 NumBytes)
	{
		NumBytes = Align(NumBytes, 16);

		if (Pages.Last().NumAllocated + NumBytes > FPage::Size)
		{
			Pages.Emplace();
		}
		FPage& Page = Pages.Last();

		void* Data = static_cast<uint8*>(Page.Allocation) + Page.NumAllocated;
		Page.NumAllocated += NumBytes;

		return Data;
	}
	FORCEINLINE void AddDeleter(void* Data, void (*Deleter)(void*))
	{
		Deleters.Add(FDeleter
		{
			Data,
			Deleter
		});
	}

public:
	template<typename T, typename... ArgTypes>
	FORCEINLINE T* Allocate(ArgTypes&&... Args)
	{
		checkStatic(alignof(T) <= 16);

		T* Value = new (Allocate(sizeof(T))) T(Forward<ArgTypes>(Args)...);

		if constexpr (!std::is_trivially_destructible_v<T>)
		{
			this->AddDeleter(
				Value,
				[](void* Data)
				{
					static_cast<T*>(Data)->~T();
				});
		}

		return Value;
	}

private:
	void* const PreviousTLS;

	FVoxelGraphTask* NextTask = nullptr;
	FVoxelGraphTask* TaskToInsertAfter = nullptr;

	TVoxelChunkedArray<FVoxelGraphTask> TaskStorage;
	TVoxelChunkedArray<FVoxelGraphTask*> TaskPool;

	struct FDeleter
	{
		void* Data = nullptr;
		void (*Deleter)(void*);
	};
	TVoxelChunkedArray<FDeleter> Deleters;

	struct FPage
	{
		static constexpr int32 Size = 1 << 14;

		void* const Allocation = FMemory::Malloc(Size);
		int32 NumAllocated = 0;

		FPage() = default;
		~FPage()
		{
			FMemory::Free(Allocation);
		}
		UE_NONCOPYABLE(FPage);
	};
	TVoxelArray<FPage> Pages;
};