// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelPinValueOps.h"
#include "VoxelPinValueOps_FVoxelStackLayer.generated.h"

USTRUCT()
struct FVoxelPinValueOps_FVoxelWeakStackLayer : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override
	{
		return
			EVoxelPinValueOpsUsage::GetExposedType |
			EVoxelPinValueOpsUsage::MakeRuntimeValue;
	}
	virtual FVoxelPinType GetExposedType() const override
	{
		return FVoxelPinType::Make<FVoxelStackLayer>();
	}
	virtual FVoxelRuntimePinValue MakeRuntimeValue(
		const FVoxelPinValue& Value,
		const FVoxelPinType::FRuntimeValueContext& Context) const override
	{
		if (!ensure(Value.Is<FVoxelStackLayer>()))
		{
			return {};
		}

		return FVoxelRuntimePinValue::Make(FVoxelWeakStackLayer(Value.Get<FVoxelStackLayer>()));
	}
	//~ End FVoxelPinValueOps Interface
};

USTRUCT()
struct FVoxelPinValueOps_FVoxelWeakStackHeightLayer : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override
	{
		return
			EVoxelPinValueOpsUsage::GetExposedType |
			EVoxelPinValueOpsUsage::MakeRuntimeValue;
	}
	virtual FVoxelPinType GetExposedType() const override
	{
		return FVoxelPinType::Make<FVoxelStackHeightLayer>();
	}
	virtual FVoxelRuntimePinValue MakeRuntimeValue(
		const FVoxelPinValue& Value,
		const FVoxelPinType::FRuntimeValueContext& Context) const override
	{
		if (!ensure(Value.Is<FVoxelStackHeightLayer>()))
		{
			return {};
		}

		return FVoxelRuntimePinValue::Make(FVoxelWeakStackHeightLayer(Value.Get<FVoxelStackHeightLayer>()));
	}
	//~ End FVoxelPinValueOps Interface
};

USTRUCT()
struct FVoxelPinValueOps_FVoxelWeakStackVolumeLayer : public FVoxelPinValueOps
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	//~ Begin FVoxelPinValueOps Interface
	virtual EVoxelPinValueOpsUsage GetUsage() const override
	{
		return
			EVoxelPinValueOpsUsage::GetExposedType |
			EVoxelPinValueOpsUsage::MakeRuntimeValue;
	}
	virtual FVoxelPinType GetExposedType() const override
	{
		return FVoxelPinType::Make<FVoxelStackVolumeLayer>();
	}
	virtual FVoxelRuntimePinValue MakeRuntimeValue(
		const FVoxelPinValue& Value,
		const FVoxelPinType::FRuntimeValueContext& Context) const override
	{
		if (!ensure(Value.Is<FVoxelStackVolumeLayer>()))
		{
			return {};
		}

		return FVoxelRuntimePinValue::Make(FVoxelWeakStackVolumeLayer(Value.Get<FVoxelStackVolumeLayer>()));
	}
	//~ End FVoxelPinValueOps Interface
};