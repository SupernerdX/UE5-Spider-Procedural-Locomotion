// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelRuntimePinValue.h"
#include "VoxelPinValueOps.generated.h"

enum class EVoxelPinValueOpsUsage : uint32
{
	ToDebugString       = 1 << 0,
	GetExposedType      = 1 << 1,
	MakeRuntimeValue    = 1 << 2,
	IsNumericType       = 1 << 3,
	HasPinDefaultValue  = 1 << 4,
	Fixup               = 1 << 5,
	CopyFrom            = 1 << 6,
	ImportFromUnrelated = 1 << 7,
	MigrateParameter    = 1 << 8,
	CustomMetaData      = 1 << 9
};
ENUM_CLASS_FLAGS(EVoxelPinValueOpsUsage);

USTRUCT()
struct VOXELGRAPH_API FVoxelPinValueOps : public FVoxelVirtualStruct
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	static TSharedPtr<FVoxelPinValueOps> Find(
		const FVoxelPinType& RuntimeType,
		EVoxelPinValueOpsUsage Usage);

public:
	virtual EVoxelPinValueOpsUsage GetUsage() const VOXEL_PURE_VIRTUAL({});

	virtual FString ToDebugString(const FVoxelRuntimePinValue& Value) const  VOXEL_PURE_VIRTUAL({});
	virtual FVoxelPinType GetExposedType() const VOXEL_PURE_VIRTUAL({});

	virtual FVoxelRuntimePinValue MakeRuntimeValue(
		const FVoxelPinValue& Value,
		const FVoxelPinType::FRuntimeValueContext& Context) const VOXEL_PURE_VIRTUAL({});

	virtual bool IsNumericType() const VOXEL_PURE_VIRTUAL({});
	virtual bool HasPinDefaultValue() const VOXEL_PURE_VIRTUAL({});
	virtual void Fixup(FVoxelPinValueBase& Value) const VOXEL_PURE_VIRTUAL();

	virtual void CopyFrom(
		FVoxelPinValueBase& This,
		const FVoxelPinValueBase& Other) const VOXEL_PURE_VIRTUAL();

	virtual bool ImportFromUnrelated(
		FVoxelPinValueBase& This,
		const FVoxelPinValueBase& Other) const VOXEL_PURE_VIRTUAL({});

	virtual void MigrateParameter(FVoxelPinValue& Value) const VOXEL_PURE_VIRTUAL();

#if WITH_EDITOR
	virtual TMap<FName, FString> GetMetaData() const VOXEL_PURE_VIRTUAL({});
#endif
};