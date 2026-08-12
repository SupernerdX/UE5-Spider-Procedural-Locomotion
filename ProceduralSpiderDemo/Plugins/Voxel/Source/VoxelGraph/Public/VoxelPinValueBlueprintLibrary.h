// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValue.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelPinValueBlueprintLibrary.generated.h"

UCLASS()
class VOXELGRAPH_API UVoxelPinValueBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, DisplayName = "Make Voxel Pin Value", CustomThunk, Category = "Voxel", meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true", NativeMakeFunc))
	static FVoxelPinValue K2_MakeVoxelPinValue(int32 Value)
	{
		unimplemented();
		return {};
	}
	DECLARE_FUNCTION(execK2_MakeVoxelPinValue);

	UFUNCTION(BlueprintPure, DisplayName = "Break Voxel Pin Value", CustomThunk, Category = "Voxel", meta = (CustomStructureParam = "OutValue", BlueprintInternalUseOnly = "true", NativeMakeFunc))
	static void K2_BreakVoxelPinValue(
		FVoxelPinValue Value,
		bool& bIsValid,
		UPARAM(DisplayName = "Value") int32& OutValue)
	{
		unimplemented();
	}
	DECLARE_FUNCTION(execK2_BreakVoxelPinValue);
};