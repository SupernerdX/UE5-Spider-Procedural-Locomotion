// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"

template<typename>
struct TVoxelGraphWaitLambdaArg;

template<typename T>
struct TVoxelGraphWaitLambdaArg<TSharedRef<T>>
{
	FORCEINLINE static const TSharedRef<T>& Get(const TSharedRef<T>& Arg)
	{
		return Arg;
	}
};

template<typename T>
struct TVoxelGraphWaitLambdaArg<TSharedPtr<T>>
{
	FORCEINLINE static const TSharedPtr<T>& Get(const TSharedPtr<T>& Arg)
	{
		return Arg;
	}
};

template<typename T>
struct TVoxelGraphWaitLambdaArg<TVoxelArray<TSharedRef<T>>>
{
	FORCEINLINE static const TVoxelArray<TSharedRef<T>>& Get(const TVoxelArray<TSharedRef<T>>& Arg)
	{
		return Arg;
	}
};

template<typename T>
struct TVoxelGraphWaitLambdaArg<TVoxelArray<TSharedPtr<T>>>
{
	FORCEINLINE static const TVoxelArray<TSharedPtr<T>>& Get(const TVoxelArray<TSharedPtr<T>>& Arg)
	{
		return Arg;
	}
};

template<typename Key, typename Value>
struct TVoxelGraphWaitLambdaArg<TVoxelMap<Key, TSharedRef<Value>>>
{
	FORCEINLINE static const TVoxelMap<Key, TSharedRef<Value>>& Get(const TVoxelMap<Key, TSharedRef<Value>>& Arg)
	{
		return Arg;
	}
};

template<typename Key, typename Value>
struct TVoxelGraphWaitLambdaArg<TVoxelMap<Key, TSharedPtr<Value>>>
{
	FORCEINLINE static const TVoxelMap<Key, TSharedPtr<Value>>& Get(const TVoxelMap<Key, TSharedPtr<Value>>& Arg)
	{
		return Arg;
	}
};

template<typename T>
requires
(
	std::is_trivially_destructible_v<T> &&
	!FVoxelNode::TIsValue<T>::Value
)
struct TVoxelGraphWaitLambdaArg<T>
{
	FORCEINLINE static const T& Get(const T& Arg)
	{
		return Arg;
	}
};

template<typename T>
requires
(
	std::is_trivially_destructible_v<T> &&
	!FVoxelNode::TIsValue<T>::Value
)
struct TVoxelGraphWaitLambdaArg<TVoxelArray<T>>
{
	FORCEINLINE static const TVoxelArray<T>& Get(const TVoxelArray<T>& Arg)
	{
		return Arg;
	}
};

template<>
struct TVoxelGraphWaitLambdaArg<FVoxelNode::FValue>
{
	FORCEINLINE static const FVoxelRuntimePinValue& Get(const FVoxelNode::FValue& Arg)
	{
		return Arg.GetValue();
	}
};

template<typename T>
struct TVoxelGraphWaitLambdaArg<FVoxelNode::TValue<T>>
{
	FORCEINLINE static auto Get(const FVoxelNode::TValue<T>& Arg) -> decltype(auto)
	{
		return Arg.Get();
	}
};

template<>
struct TVoxelGraphWaitLambdaArg<TOptional<FVoxelNode::FValue>>
{
	FORCEINLINE static TOptional<FVoxelRuntimePinValue> Get(const TOptional<FVoxelNode::FValue>& Arg)
	{
		if (Arg.IsSet())
		{
			return Arg->GetValue();
		}

		return {};
	}
};

template<typename T>
requires FVoxelNode::FValue::PassByValue<T>
struct TVoxelGraphWaitLambdaArg<TOptional<FVoxelNode::TValue<T>>>
{
	FORCEINLINE static TOptional<T> Get(const TOptional<FVoxelNode::TValue<T>>& Arg)
	{
		if (Arg.IsSet())
		{
			return TOptional<T>(Arg->Get());
		}

		return TOptional<T>();
	}
};

template<typename T>
requires (!FVoxelNode::FValue::PassByValue<T>)
struct TVoxelGraphWaitLambdaArg<TOptional<FVoxelNode::TValue<T>>>
{
	FORCEINLINE static TOptional<TSharedRef<const T>> Get(const TOptional<FVoxelNode::TValue<T>>& Arg)
	{
		if (Arg.IsSet())
		{
			return TOptional<TSharedRef<const T>>(Arg->Get());
		}

		return TOptional<TSharedRef<const T>>();
	}
};

template<typename AllocatorType>
struct TVoxelGraphWaitLambdaArg<TVoxelArray<FVoxelNode::FValue, AllocatorType>>
{
	FORCEINLINE static TVoxelArray<FVoxelRuntimePinValue, AllocatorType> Get(const TVoxelArray<FVoxelNode::FValue, AllocatorType>& Arg)
	{
		TVoxelArray<FVoxelRuntimePinValue, AllocatorType> Result;
		Result.Reserve(Arg.Num());
		for (const FVoxelNode::FValue& Element : Arg)
		{
			Result.Add_EnsureNoGrow(Element.GetValue());
		}
		return Result;
	}
};
template<typename T>
requires FVoxelNode::FValue::PassByValue<T>
struct TVoxelGraphWaitLambdaArg<TVoxelArray<FVoxelNode::TValue<T>>>
{
	FORCEINLINE static TVoxelArray<T> Get(const TVoxelArray<FVoxelNode::TValue<T>>& Arg)
	{
		TVoxelArray<T> Result;
		Result.Reserve(Arg.Num());
		for (const FVoxelNode::TValue<T>& Element : Arg)
		{
			Result.Add_EnsureNoGrow(Element.Get());
		}
		return Result;
	}
};
template<typename T>
requires (!FVoxelNode::FValue::PassByValue<T>)
struct TVoxelGraphWaitLambdaArg<TVoxelArray<FVoxelNode::TValue<T>>>
{
	FORCEINLINE static TVoxelArray<TSharedRef<const T>> Get(const TVoxelArray<FVoxelNode::TValue<T>>& Arg)
	{
		TVoxelArray<TSharedRef<const T>> Result;
		Result.Reserve(Arg.Num());
		for (const FVoxelNode::TValue<T>& Element : Arg)
		{
			Result.Add_EnsureNoGrow(Element.Get());
		}
		return Result;
	}
};

template<typename T>
struct TVoxelGraphWaitLambdaArgType
{
	using Type = VOXEL_GET_TYPE(TVoxelGraphWaitLambdaArg<T>::Get(std::declval<T>()));
};

template<typename, typename... ArgTypes>
class TVoxelGraphWait
{
public:
	const FVoxelGraphQuery Query;
	const FMinimalName Name;
	TTuple<ArgTypes...> Args;

	FORCEINLINE explicit TVoxelGraphWait(
		const FVoxelGraphQuery Query,
		const FName Name,
		const ArgTypes&... Args)
		: Query(Query)
		, Name(Name)
		, Args(Args...)
	{
	}

public:
	template<typename LambdaType>
	FORCEINLINE void operator+(LambdaType&& Lambda)
	{
		return this->Execute(MoveTemp(Lambda), TMakeIntegerSequence<uint32, sizeof...(ArgTypes)>());
	}

private:
	template<typename LambdaType, uint32... ArgIndices>
	FORCEINLINE void Execute(LambdaType&& Lambda, TIntegerSequence<uint32, ArgIndices...>)
	{
		Query.AddTask([
#if WITH_EDITOR
			Query = Query,
#endif
			Name = Name,
			Args = MoveTemp(Args),
			Lambda = MoveTemp(Lambda)]
		{
			VOXEL_SCOPE_COUNTER_FNAME(FName(Name));
#if WITH_EDITOR
			Query->Context.CurrentCallstack_EditorOnly = Query.GetCallstack();
#endif
			return Lambda(0, TVoxelGraphWaitLambdaArg<ArgTypes>::Get(Args.template Get<ArgIndices>())...);
		});
	}
};

#define IMPL_VOXEL_GRAPH_WAIT_DECLTYPE(Data) , std::remove_const_t<VOXEL_GET_TYPE(Data)>
#define IMPL_VOXEL_GRAPH_WAIT_LAMBDA_ARGS(Data) , const TVoxelGraphWaitLambdaArgType<std::remove_const_t<VOXEL_GET_TYPE(Data)>>::Type& Data

#define VOXEL_GRAPH_WAIT(...) \
	TVoxelGraphWait<void VOXEL_FOREACH(IMPL_VOXEL_GRAPH_WAIT_DECLTYPE, ##__VA_ARGS__)>(Query, STATIC_FNAME(VOXEL_STATS_CLEAN_FUNCTION_NAME), ##__VA_ARGS__) + \
	[this, Query = Query](int32 VOXEL_FOREACH(IMPL_VOXEL_GRAPH_WAIT_LAMBDA_ARGS, ##__VA_ARGS__)) -> void
