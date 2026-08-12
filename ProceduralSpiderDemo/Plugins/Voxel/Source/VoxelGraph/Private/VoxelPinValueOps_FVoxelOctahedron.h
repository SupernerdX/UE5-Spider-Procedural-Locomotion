// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValueOps.h"
#include "VoxelPinValueOps_FVoxelOctahedron.generated.h"

USTRUCT()
struct FVoxelPinValueOps_FVoxelOctahedron : public FVoxelPinValueOps
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
		return FVoxelPinType::Make<FVector>();
	}

	virtual FVoxelRuntimePinValue MakeRuntimeValue(
		const FVoxelPinValue& Value,
		const FVoxelPinType::FRuntimeValueContext& Context) const override
	{
		if (!ensure(Value.Is<FVector>()))
		{
			return {};
		}

		const FVector Normal = Value.Get<FVector>().GetSafeNormal(SMALL_NUMBER, FVector::UpVector);

		return FVoxelRuntimePinValue::Make(FVoxelOctahedron(FVector3f(Normal)));
	}
	//~ End FVoxelPinValueOps Interface
};