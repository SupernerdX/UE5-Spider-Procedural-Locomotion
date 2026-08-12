// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinType.h"

enum class EVoxelPinFlags : uint32
{
	None         = 0,
	TemplatePin  = 1 << 0,
	VariadicPin  = 1 << 1,
};
ENUM_CLASS_FLAGS(EVoxelPinFlags);

struct FVoxelPinRuntimeMetadata
{
	uint32 bHidePin : 1;
	uint32 bArrayPin : 1;
	uint32 bPositionPin : 1;
	uint32 bSplineKeyPin : 1;
	uint32 bDisplayLast : 1;
	uint32 bNoCache : 1;
	uint32 bNoDefault : 1;
	uint32 bHideDefault : 1;
	uint32 bShowInDetail : 1;

	FVoxelPinRuntimeMetadata()
	{
		ReinterpretCastRef<uint32>(*this) = 0;
	}

	FORCEINLINE bool IsOptional() const
	{
		return
			bPositionPin ||
			bSplineKeyPin ||
			bHideDefault;
	}
};

struct FVoxelPinMetadata : FVoxelPinRuntimeMetadata
{
#if WITH_EDITOR
	FString DisplayName;
	FString Category;
	TAttribute<FString> Tooltip;
	FString DefaultValue;
	int32 Line = 0;
	UScriptStruct* Struct = nullptr;
	TVoxelMap<FName, FString> CustomMetadata;
#endif
};

struct VOXELGRAPH_API FVoxelPin
{
public:
	const FName Name;
	const bool bIsInput;
	const float SortOrder;
	const FName VariadicPinName;
	const EVoxelPinFlags Flags;
	const FVoxelPinType BaseType;
	const FVoxelPinMetadata Metadata;

	bool IsPromotable() const
	{
		return BaseType.IsWildcard();
	}

	void SetType(const FVoxelPinType& NewType)
	{
		ensure(IsPromotable());
		ChildType = NewType;
	}
	const FVoxelPinType& GetType() const
	{
		ensure(BaseType == ChildType || IsPromotable());
		return ChildType;
	}

private:
	FVoxelPinType ChildType;

	FVoxelPin(
		const FName Name,
		const bool bIsInput,
		const float SortOrder,
		const FName VariadicPinName,
		const EVoxelPinFlags Flags,
		const FVoxelPinType& BaseType,
		const FVoxelPinType& ChildType,
		const FVoxelPinMetadata& Metadata)
		: Name(Name)
		, bIsInput(bIsInput)
		, SortOrder(SortOrder)
		, VariadicPinName(VariadicPinName)
		, Flags(Flags)
		, BaseType(BaseType)
		, Metadata(Metadata)
		, ChildType(ChildType)
	{
		ensure(BaseType.IsValid());
		ensure(ChildType.IsValid());
	}

	friend struct FVoxelNode;
};