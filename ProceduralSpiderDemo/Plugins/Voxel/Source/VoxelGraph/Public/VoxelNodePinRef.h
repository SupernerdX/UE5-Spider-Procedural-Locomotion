// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"
#include "VoxelNodeValue.h"

struct FVoxelWildcard;
struct FVoxelWildcardBuffer;

struct FVoxelNode::FPinRef
{
public:
#if VOXEL_DEBUG
	uint64 Magic = 0xDEADBEEF;
#endif

	FPinRef() = default;
	explicit FPinRef(const FName Name)
		: Name(Name)
	{
	}

	FORCEINLINE FName GetName() const
	{
		return Name;
	}
	FORCEINLINE const FVoxelPinType& GetType_RuntimeOnly() const
	{
		checkVoxelSlow(bInitialized);
		return Type;
	}
	FORCEINLINE FVoxelPinType GetInnerType_RuntimeOnly() const
	{
		checkVoxelSlow(bInitialized);
		return Type.GetInnerType();
	}

protected:
	FName Name;
	bool bInitialized = false;
	FVoxelPinType Type;
	const FVoxelNode* OuterNode = nullptr;
	FVoxelPinRuntimeMetadata Metadata;

	friend FVoxelNode;
	friend class UVoxelTerminalGraphRuntime;
};

FORCEINLINE bool operator==(
	const FVoxelPin& Pin,
	const FVoxelNode::FPinRef& PinRef)
{
	return Pin.Name == PinRef.GetName();
}

struct VOXELGRAPH_API FVoxelNode::FPinRef_Input : FPinRef
{
public:
	explicit FPinRef_Input(const FName Name)
		: FPinRef(Name)
	{
	}

	FORCEINLINE bool IsDefaultValue() const
	{
		checkVoxelSlow(bInitialized);
		return LinkedPinIndex == -1;
	}
	FORCEINLINE const FVoxelRuntimePinValue& GetDefaultValue() const
	{
		checkVoxelSlow(IsDefaultValue());
		return DefaultValue;
	}

	FORCEINLINE FValue Get(FVoxelGraphQuery Query) const
	{
		checkVoxelSlow(bInitialized);

		if (IsDefaultValue())
		{
			return FValue(DefaultValue);
		}

		if (LinkedPinMetadata.bNoCache ||
			!Query->IsNodeComputed(LinkedNodeIndex))
		{
			Query.AddTask([this, Query]
			{
				ComputeLinkedNode(Query);
			});
		}

		return FValue(Query->GetPinValue(LinkedPinIndex));
	}

	FVoxelRuntimePinValue GetSynchronous(const FVoxelGraphQueryImpl& Query) const;

public:
	template<typename Type>
	FORCEINLINE TValue<Type> Get(const FVoxelGraphQuery Query) const
	{
		return TValue<Type>(Get(Query));
	}

	template<typename T>
	requires FValue::PassByValue<T>
	FORCEINLINE T GetSynchronous(const FVoxelGraphQueryImpl& Query) const
	{
		return GetSynchronous(Query).Get<T>();
	}
	template<typename T>
	requires (!FValue::PassByValue<T>)
	FORCEINLINE TSharedRef<const T> GetSynchronous(const FVoxelGraphQueryImpl& Query) const
	{
		return GetSynchronous(Query).GetSharedStruct<T>();
	}

private:
	FVoxelRuntimePinValue DefaultValue;
	const FVoxelNode* LinkedNode = nullptr;
	int32 LinkedNodeIndex = -1;
	int32 LinkedPinIndex = -1;
	FVoxelPinRuntimeMetadata LinkedPinMetadata;

	void ComputeLinkedNode(FVoxelGraphQuery Query) const;

	friend class UVoxelTerminalGraphRuntime;
};

struct FVoxelNode::FPinRef_Output : FPinRef
{
public:
	explicit FPinRef_Output(const FName Name)
		: FPinRef(Name)
	{
	}

	FORCEINLINE int32 GetPinIndex() const
	{
		return PinIndex;
	}
	FORCEINLINE bool ShouldCompute() const
	{
		return PinIndex != -1;
	}
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		FVoxelRuntimePinValue Value) const
	{
		checkVoxelSlow(bInitialized);

#if WITH_EDITOR
		ensure(Value.CanBeCastedTo(Type));
		ensure(Value.IsValidValue_Slow());
#endif

		if (PinIndex == -1)
		{
			return;
		}

		FVoxelRuntimePinValue& PinValue = Query->GetPinValue(PinIndex);
		checkVoxelSlow(!PinValue.IsValid() || Metadata.bNoCache);

		PinValue = MoveTemp(Value);
		PinValue.SetType(Type);
	}

private:
	int32 PinIndex = -1;

	friend FVoxelNode;
	friend FVoxelNode_CallFunction;
	friend class UVoxelTerminalGraphRuntime;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
requires
(
	FVoxelNode::FValue::IsTyped<T> &&
	HasNativeVoxelBuffer<T>
)
struct FVoxelNode::TTemplatePinRef_Input<T> : FPinRef_Input
{
public:
	explicit TTemplatePinRef_Input(const FPinRef_Input& Pin)
		: FPinRef_Input(Pin)
	{
	}

	FORCEINLINE TValue<T> GetUniform(const FVoxelGraphQuery Query) const
	{
		return Get<T>(Query);
	}
	FORCEINLINE TValue<TVoxelBufferType<T>> GetBuffer(const FVoxelGraphQuery Query) const
	{
		return Get<TVoxelBufferType<T>>(Query);
	}

private:
	using FPinRef_Input::Get;
};

template<typename T>
requires
(
	FVoxelNode::FValue::IsTyped<T> &&
	!HasNativeVoxelBuffer<T>
)
struct FVoxelNode::TTemplatePinRef_Input<T> : FPinRef_Input
{
public:
	explicit TTemplatePinRef_Input(const FPinRef_Input& Pin)
		: FPinRef_Input(Pin)
	{
	}

	FORCEINLINE TValue<T> Get(const FVoxelGraphQuery Query) const
	{
		return FPinRef_Input::Get<T>(Query);
	}
};

template<typename T>
requires (!FVoxelNode::FValue::IsTyped<T>)
struct FVoxelNode::TTemplatePinRef_Input<T> : FPinRef_Input
{
	explicit TTemplatePinRef_Input(const FPinRef_Input& Pin)
		: FPinRef_Input(Pin)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
requires
(
	FVoxelNode::FValue::IsTyped<T> &&
	!HasNativeVoxelBuffer<T>
)
struct FVoxelNode::TTemplatePinRef_Output<T> : FPinRef_Output
{
	explicit TTemplatePinRef_Output(const FPinRef_Output& Pin)
		: FPinRef_Output(Pin)
	{
	}

	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		const T& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(Value));
	}
};

template<typename T>
requires
(
	FVoxelNode::FValue::IsTyped<T> &&
	HasNativeVoxelBuffer<T>
)
struct FVoxelNode::TTemplatePinRef_Output<T> : FPinRef_Output
{
	explicit TTemplatePinRef_Output(const FPinRef_Output& Pin)
		: FPinRef_Output(Pin)
	{
	}

	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		const T& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(Value));
	}
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		TVoxelBufferType<T> Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(MakeSharedCopy(MoveTemp(Value))));
	}
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		const TSharedRef<TVoxelBufferType<T>>& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(Value));
	}
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		const TSharedRef<const TVoxelBufferType<T>>& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(Value));
	}
};

template<typename T>
requires (!FVoxelNode::FValue::IsTyped<T>)
struct FVoxelNode::TTemplatePinRef_Output<T> : FPinRef_Output
{
	explicit TTemplatePinRef_Output(const FPinRef_Output& Pin)
		: FPinRef_Output(Pin)
	{
	}

	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		const TSharedRef<FVoxelBuffer>& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(Value));
	}
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		const TSharedRef<const FVoxelBuffer>& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(Value));
	}
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		FVoxelRuntimePinValue Value) const
	{
		FPinRef_Output::Set(Query, MoveTemp(Value));
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
requires FVoxelNode::FValue::IsTyped<T>
struct FVoxelNode::TPinRef_Input<T> : FPinRef_Input
{
	explicit TPinRef_Input(const FPinRef_Input& Pin)
		: FPinRef_Input(Pin)
	{
	}

	FORCEINLINE TValue<T> Get(const FVoxelGraphQuery Query) const
	{
		return FPinRef_Input::Get<T>(Query);
	}
	FORCEINLINE auto GetSynchronous(const FVoxelGraphQueryImpl& Query) const -> decltype(auto)
	{
		return FPinRef_Input::GetSynchronous<T>(Query);
	}
};

template<typename T>
requires (!FVoxelNode::FValue::IsTyped<T>)
struct FVoxelNode::TPinRef_Input<T> : FPinRef_Input
{
	explicit TPinRef_Input(const FPinRef_Input& Pin)
		: FPinRef_Input(Pin)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
requires FVoxelNode::FValue::IsTyped<T>
struct FVoxelNode::TPinRef_Output<T> : FPinRef_Output
{
	explicit TPinRef_Output(const FPinRef_Output& Pin)
		: FPinRef_Output(Pin)
	{
	}

	template<typename OtherType>
	requires std::is_constructible_v<T, const OtherType&>
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		const OtherType& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(T(Value)));
	}
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		T&& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(MoveTemp(Value)));
	}
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		const TSharedRef<T>& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(Value));
	}
	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		const TSharedRef<const T>& Value) const
	{
		FPinRef_Output::Set(Query, FVoxelRuntimePinValue::Make(Value));
	}
};

template<typename T>
requires (!FVoxelNode::FValue::IsTyped<T>)
struct FVoxelNode::TPinRef_Output<T> : FPinRef_Output
{
	explicit TPinRef_Output(const FPinRef_Output& Pin)
		: FPinRef_Output(Pin)
	{
	}

	FORCEINLINE void Set(
		const FVoxelGraphQuery Query,
		FVoxelRuntimePinValue Value) const
	{
		FPinRef_Output::Set(Query, MoveTemp(Value));
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelNode::FVariadicPinRef_Input
{
public:
#if VOXEL_DEBUG
	uint64 Magic = 0xDEADBEEF;
#endif

	FVariadicPinRef_Input() = default;
	explicit FVariadicPinRef_Input(const FName Name)
		: Name(Name)
	{
	}

	FORCEINLINE FName GetName() const
	{
		return Name;
	}

	FORCEINLINE TVoxelArray<FValue> Get(const FVoxelGraphQuery Query) const
	{
		TVoxelArray<FValue> Result;
		Result.Reserve(PinRefs.Num());

		for (const FPinRef_Input& PinRef : PinRefs)
		{
			Result.Add(PinRef.Get(Query));
		}

		return Result;
	}

	template<typename Type>
	FORCEINLINE TVoxelArray<TValue<Type>> Get(const FVoxelGraphQuery Query) const
	{
		TVoxelArray<TValue<Type>> Result;
		Result.Reserve(PinRefs.Num());

		for (const FPinRef_Input& PinRef : PinRefs)
		{
			Result.Add(PinRef.Get<Type>(Query));
		}

		return Result;
	}

private:
	FName Name;
	TVoxelArray<FPinRef_Input> PinRefs;

	friend FVoxelNode;
};

template<typename T>
struct FVoxelNode::TVariadicTemplatePinRef_Input : FVariadicPinRef_Input
{
	explicit TVariadicTemplatePinRef_Input(const FVariadicPinRef_Input& Pin)
		: FVariadicPinRef_Input(Pin)
	{
	}
};

template<typename T>
requires FVoxelNode::FValue::IsTyped<T>
struct FVoxelNode::TVariadicPinRef_Input<T> : FVariadicPinRef_Input
{
	explicit TVariadicPinRef_Input(const FVariadicPinRef_Input& Pin)
		: FVariadicPinRef_Input(Pin)
	{
	}

	FORCEINLINE TVoxelArray<TValue<T>> Get(const FVoxelGraphQuery Query) const
	{
		return FVariadicPinRef_Input::Get<T>(Query);
	}
};

template<typename T>
requires (!FVoxelNode::FValue::IsTyped<T>)
struct FVoxelNode::TVariadicPinRef_Input<T> : FVariadicPinRef_Input
{
	explicit TVariadicPinRef_Input(const FVariadicPinRef_Input& Pin)
		: FVariadicPinRef_Input(Pin)
	{
	}

	FORCEINLINE TVoxelArray<FValue> Get(const FVoxelGraphQuery Query) const
	{
		return FVariadicPinRef_Input::Get(Query);
	}

	template<typename Type>
	FORCEINLINE TVoxelArray<TValue<Type>> Get(const FVoxelGraphQuery Query) const
	{
		return FVariadicPinRef_Input::Get<Type>(Query);
	}
};