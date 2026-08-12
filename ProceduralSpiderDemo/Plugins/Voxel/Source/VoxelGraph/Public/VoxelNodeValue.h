// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelNode.h"

struct FVoxelNode::FValue
{
public:
	template<typename T>
	static constexpr bool IsTyped =
		!std::is_void_v<T> &&
		!std::is_same_v<T, FVoxelWildcard> &&
		!std::is_same_v<T, FVoxelWildcardBuffer>;

	template<typename T>
	static constexpr bool PassByValue =
		std::is_trivially_destructible_v<T> ||
		std::is_same_v<T, FVoxelRuntimePinValue>;

public:
	FORCEINLINE const FVoxelRuntimePinValue& GetValue() const
	{
		checkVoxelSlow(Value);
		checkVoxelSlow(Value->IsValid());
		checkVoxelSlow(Value->IsValidValue_Slow());
		return *Value;
	}

	template<typename T>
	FORCEINLINE auto Get() const -> decltype(auto)
	{
		return GetValue().Get<T>();
	}
	template<typename T>
	FORCEINLINE const TSharedRef<const T>& GetShared() const
	{
		return GetValue().GetSharedStruct<T>();
	}

private:
	const FVoxelRuntimePinValue* Value = nullptr;

	FORCEINLINE explicit FValue(const FVoxelRuntimePinValue& Value)
		: Value(&Value)
	{
	}

	friend struct FVoxelNode;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
requires
(
	IsSafeVoxelPinValue<T> &&
	FVoxelNode::FValue::PassByValue<T>
)
struct FVoxelNode::TValue<T> : private FValue
{
public:
	FORCEINLINE auto Get() const -> decltype(auto)
	{
		return FValue::Get<T>();
	}
	FORCEINLINE const T* operator->() const
	{
		return &Get();
	}
	FORCEINLINE const T& operator*() const
	{
		return Get();
	}

private:
	TValue(const FValue& Future)
		: FValue(Future)
	{
	}

	friend struct FVoxelNode;
};

template<typename T>
requires
(
	IsSafeVoxelPinValue<T> &&
	!FVoxelNode::FValue::PassByValue<T>
)
struct FVoxelNode::TValue<T> : private FValue
{
public:
	FORCEINLINE const TSharedRef<const T>& Get() const
	{
		return FValue::GetShared<T>();
	}
	FORCEINLINE const T* operator->() const
	{
		return &Get().Get();
	}
	FORCEINLINE const T& operator*() const
	{
		return Get().Get();
	}

private:
	TValue(const FValue& Future)
		: FValue(Future)
	{
	}

	friend struct FVoxelNode;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename>
struct FVoxelNode::TIsValue
{
	static constexpr bool Value = false;
};

template<>
struct FVoxelNode::TIsValue<FVoxelNode::FValue>
{
	static constexpr bool Value = true;
};

template<typename T>
struct FVoxelNode::TIsValue<FVoxelNode::TValue<T>>
{
	static constexpr bool Value = true;
};