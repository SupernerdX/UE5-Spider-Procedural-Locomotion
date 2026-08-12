// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"

#define GENERATED_VOXEL_DIFF_ENTRY_BODY(StructName) \
	static FName GetStaticType() \
	{ \
		return # StructName; \
	} \
	virtual FName GetType() const override \
	{ \
		return GetStaticType(); \
	}

struct FVoxelDiffMode;

struct VOXELGRAPHEDITOR_API FVoxelDiffEntry : public TSharedFromThis<FVoxelDiffEntry>
{
public:
	FVoxelDiffEntry(const TWeakPtr<FVoxelDiffMode>& Mode)
		: WeakMode(Mode)
	{}

	virtual FName GetType() const = 0;
	virtual TSharedRef<SWidget> GenerateWidget() const { return SNullWidget::NullWidget; }
	virtual void OnEntrySelected() {}
	TArray<TSharedPtr<FVoxelDiffEntry>> Children;
	TWeakPtr<FVoxelDiffMode> WeakMode;

public:
	template<typename T>
	FORCEINLINE bool IsA() const
	{
		return GetType() == T::GetStaticType();
	}
	template<typename T>
	FORCEINLINE T& Get()
	{
		checkVoxelSlow(IsA<T>());
		return *static_cast<T*>(this);
	}
	template<typename T>
	FORCEINLINE const T& Get() const
	{
		return ConstCast(this)->Get<T>();
	}
	template<typename T>
	TSharedPtr<T> GetShared() const
	{
		return IsA<T>() ? StaticCastSharedPtr<TSharedPtr<T>>(AsShared()) : nullptr;
	}

public:
	virtual ~FVoxelDiffEntry() = default;
};
