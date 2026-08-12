// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelRuntimePinValue.h"
#include "VoxelRuntimeStruct.generated.h"

class UUserDefinedStruct;

USTRUCT()
struct VOXELGRAPH_API FVoxelRuntimeStruct
{
	GENERATED_BODY()

	UUserDefinedStruct* UserStruct = nullptr;
	TVoxelMap<FName, FVoxelRuntimePinValue> PropertyNameToValue;

	FVoxelRuntimeStruct() = default;

	explicit FVoxelRuntimeStruct(
		FConstVoxelStructView Struct,
		const FVoxelPinType::FRuntimeValueContext& Context);

	static FVoxelPinType GetRuntimeType(const FVoxelPinType& Type);
};