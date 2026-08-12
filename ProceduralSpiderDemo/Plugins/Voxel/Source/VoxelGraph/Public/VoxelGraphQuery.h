// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelGraphContext.h"
#include "VoxelRuntimePinValue.h"

struct FVoxelNode;
struct FVoxelNode_CallFunction;

namespace FVoxelGraphParameters
{
	struct FLOD : FUniformParameter
	{
		int32 Value = 0;
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct VOXELGRAPH_API FVoxelGraphQueryImpl
{
public:
	FVoxelGraphContext& Context;

	// Used to resolve function calls
	const FVoxelCompiledGraph& CompiledGraph;

	// Used to preallocate computed nodes/pins
	const FVoxelCompiledTerminalGraph& CompiledTerminalGraph;

	struct FFunctionCallData
	{
		const FVoxelNode& Node;
		const FVoxelGraphQueryImpl& Query;
	};
	TOptional<FFunctionCallData> FunctionCallData;

	UE_NONCOPYABLE(FVoxelGraphQueryImpl);

	VOXEL_COUNT_INSTANCES();

public:
	const FVoxelGraphQueryImpl& GetChild(
		const FVoxelCompiledGraph& NewCompiledGraph,
		const FVoxelCompiledTerminalGraph& NewCompiledTerminalGraph,
		const FFunctionCallData* NewFunctionCallData) const;

	FVoxelGraphQueryImpl& CloneParameters() const;
	TVoxelArray<FVoxelGraphQueryImpl*> Split(const FVoxelBufferSplitter& Splitter) const;

public:
	FORCEINLINE bool CanEditParameters() const
	{
		return
			NameIndexToUniformParameter_Ptr == &NameIndexToUniformParameter_Storage &&
			NameIndexToBufferParameter_Ptr == &NameIndexToBufferParameter_Storage;
	}
	FORCEINLINE bool HasSameParameters(const FVoxelGraphQueryImpl& Other) const
	{
		return
			NameIndexToUniformParameter_Ptr == Other.NameIndexToUniformParameter_Ptr &&
			NameIndexToBufferParameter_Ptr == Other.NameIndexToBufferParameter_Ptr;
	}

public:
	template<typename T, typename... ArgTypes>
	requires
	(
		FVoxelGraphParameters::IsUniform<T> &&
		std::is_constructible_v<T, ArgTypes&&...>
	)
	T& AddParameter(ArgTypes&&... Args)
	{
		checkVoxelSlow(CanEditParameters());

		T* Value = Context.Allocate<T>(Forward<ArgTypes>(Args)...);

		const int32 ParameterIndex = FVoxelGraphParameterManager::GetUniformIndex<T>();

		if (NameIndexToUniformParameter_Storage.Num() <= ParameterIndex)
		{
			NameIndexToUniformParameter_Storage.SetNumZeroed(FVoxelGraphParameterManager::NumUniform());
		}
		NameIndexToUniformParameter_Storage[ParameterIndex] = Value;

		return *Value;
	}
	template<typename T>
	requires FVoxelGraphParameters::IsBuffer<T>
	T& AddParameter()
	{
		checkVoxelSlow(CanEditParameters());

		T* Value = Context.Allocate<T>();

		const int32 ParameterIndex = FVoxelGraphParameterManager::GetBufferIndex<T>();

		if (NameIndexToBufferParameter_Storage.Num() <= ParameterIndex)
		{
			NameIndexToBufferParameter_Storage.SetNumZeroed(FVoxelGraphParameterManager::NumBuffer());
		}
		NameIndexToBufferParameter_Storage[ParameterIndex] = Value;

		return *Value;
	}

public:
	template<typename T>
	requires FVoxelGraphParameters::IsUniform<T>
	void RemoveParameter()
	{
		checkVoxelSlow(CanEditParameters());

		const int32 ParameterIndex = FVoxelGraphParameterManager::GetUniformIndex<T>();

		if (NameIndexToUniformParameter_Storage.IsValidIndex(ParameterIndex))
		{
			NameIndexToUniformParameter_Storage[ParameterIndex] = nullptr;
		}
	}
	template<typename T>
	requires FVoxelGraphParameters::IsBuffer<T>
	void RemoveParameter()
	{
		checkVoxelSlow(CanEditParameters());

		const int32 ParameterIndex = FVoxelGraphParameterManager::GetBufferIndex<T>();

		if (NameIndexToBufferParameter_Storage.IsValidIndex(ParameterIndex))
		{
			NameIndexToBufferParameter_Storage[ParameterIndex] = nullptr;
		}
	}

public:
	template<typename T>
	requires FVoxelGraphParameters::IsUniform<T>
	FORCEINLINE const T* FindParameter() const
	{
		const int32 ParameterIndex = FVoxelGraphParameterManager::GetUniformIndex<T>();

		if (NameIndexToUniformParameter_Ptr->Num() <= ParameterIndex)
		{
			return nullptr;
		}

		return static_cast<const T*>((*NameIndexToUniformParameter_Ptr)[ParameterIndex]);
	}
	template<typename T>
	requires FVoxelGraphParameters::IsBuffer<T>
	FORCEINLINE const T* FindParameter() const
	{
		const int32 ParameterIndex = FVoxelGraphParameterManager::GetBufferIndex<T>();

		if (NameIndexToBufferParameter_Ptr->Num() <= ParameterIndex)
		{
			return nullptr;
		}

		return static_cast<const T*>((*NameIndexToBufferParameter_Ptr)[ParameterIndex]);
	}

public:
	FORCEINLINE bool IsNodeComputed(const int32 NodeIndex) const
	{
		return ComputedNodes[NodeIndex];
	}
	FORCEINLINE void SetNodeComputed(const int32 NodeIndex) const
	{
		checkVoxelSlow(!ComputedNodes[NodeIndex]);
		ComputedNodes[NodeIndex] = true;
	}
	FORCEINLINE FVoxelRuntimePinValue& GetPinValue(const int32 PinIndex) const
	{
		return PinValues[PinIndex];
	}

private:
	TVoxelArray<const FVoxelGraphParameters::FUniformParameter*> NameIndexToUniformParameter_Storage;
	TVoxelArray<const FVoxelGraphParameters::FBufferParameter*> NameIndexToBufferParameter_Storage;

	const TVoxelArray<const FVoxelGraphParameters::FUniformParameter*>* NameIndexToUniformParameter_Ptr = &NameIndexToUniformParameter_Storage;
	const TVoxelArray<const FVoxelGraphParameters::FBufferParameter*>* NameIndexToBufferParameter_Ptr = &NameIndexToBufferParameter_Storage;

	mutable FVoxelBitArray ComputedNodes;
	mutable TVoxelArray<FVoxelRuntimePinValue> PinValues;

	struct FChildKey
	{
		const FVoxelCompiledGraph* CompiledGraph = nullptr;
		const FVoxelNode* CallNode = nullptr;
		const FVoxelGraphQueryImpl* FunctionCallQuery = nullptr;

		FORCEINLINE bool operator==(const FChildKey& Other) const
		{
			return
				CompiledGraph == Other.CompiledGraph &&
				CallNode == Other.CallNode &&
				FunctionCallQuery == Other.FunctionCallQuery;
		}
		FORCEINLINE friend uint32 GetTypeHash(const FChildKey& Key)
		{
			// FunctionCallQuery is as unique as FunctionCallNode
			return HashCombineFast(
				GetTypeHash(Key.CompiledGraph),
				GetTypeHash(Key.FunctionCallQuery));
		}
	};
	mutable TVoxelMap<FChildKey, const FVoxelGraphQueryImpl*> ChildKeyToChild;

	FVoxelGraphQueryImpl(
		FVoxelGraphContext& Context,
		const FVoxelCompiledGraph& CompiledGraph,
		const FVoxelCompiledTerminalGraph& CompiledTerminalGraph);

	friend FVoxelGraphContext;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXELGRAPH_API FVoxelGraphQuery
{
public:
	FORCEINLINE explicit FVoxelGraphQuery(
		const FVoxelGraphQueryImpl& Impl,
		const FVoxelGraphCallstack* Callstack)
		: Impl(&Impl)
#if WITH_EDITOR
		, Callstack_EditorOnly(Callstack)
#endif
	{
#if WITH_EDITOR
		checkVoxelSlow(Callstack_EditorOnly);
#endif
	}

public:
	FORCEINLINE const FVoxelGraphQueryImpl& GetImpl() const
	{
		checkVoxelSlow(Impl);
		return *Impl;
	}
	FORCEINLINE const FVoxelGraphQueryImpl* operator->() const
	{
		checkVoxelSlow(Impl);
		return Impl;
	}
	FORCEINLINE const FVoxelGraphCallstack* GetCallstack() const
	{
#if WITH_EDITOR
		checkVoxelSlow(Callstack_EditorOnly);
		return Callstack_EditorOnly;
#else
		return nullptr;
#endif
	}

	FORCEINLINE bool IsPreview() const
	{
		return Impl->Context.Environment.bIsPreviewScene;
	}

	template<typename LambdaType>
	FORCEINLINE void AddTask(LambdaType&& Lambda) const
	{
#if WITH_EDITOR
		Impl->Context.AddTask([&Context = Impl->Context, Callstack = Callstack_EditorOnly]
		{
			Context.CurrentCallstack_EditorOnly = Callstack;
		});
#endif

		Impl->Context.AddTask(MoveTemp(Lambda));
	}

private:
	const FVoxelGraphQueryImpl* Impl = nullptr;
#if WITH_EDITOR
	const FVoxelGraphCallstack* Callstack_EditorOnly = nullptr;
#endif

	FVoxelGraphQuery() = default;

	friend class UVoxelFunctionLibrary;
};