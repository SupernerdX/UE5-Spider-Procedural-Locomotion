// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelParameterBlueprintLibrary.generated.h"

struct FVoxelHeightGraphStampRef;
struct FVoxelVolumeGraphStampRef;
struct FVoxelHeightSculptGraphWrapper;
struct FVoxelVolumeSculptGraphWrapper;

UCLASS()
class VOXEL_API UVoxelParameterBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, DisplayName = "Get Height Graph Parameter", CustomThunk, Category = "Voxel|Parameters", meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
	static void K2_GetVoxelHeightGraphParameter(
		const FVoxelHeightGraphStampRef& Stamp,
		FName Name,
		int32& Value)
	{
		unimplemented();
	}
	DECLARE_FUNCTION(execK2_GetVoxelHeightGraphParameter);

	UFUNCTION(BlueprintPure, DisplayName = "Get Volume Graph Parameter", CustomThunk, Category = "Voxel|Parameters", meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
	static void K2_GetVoxelVolumeGraphParameter(
		const FVoxelVolumeGraphStampRef& Stamp,
		FName Name,
		int32& Value)
	{
		unimplemented();
	}
	DECLARE_FUNCTION(execK2_GetVoxelVolumeGraphParameter);

	UFUNCTION(BlueprintPure, DisplayName = "Get Height Sculpt Graph Parameter", CustomThunk, Category = "Voxel|Parameters", meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
	static void K2_GetVoxelHeightSculptGraphParameter(
		const FVoxelHeightSculptGraphWrapper& SculptGraph,
		FName Name,
		int32& Value)
	{
		unimplemented();
	}
	DECLARE_FUNCTION(execK2_GetVoxelHeightSculptGraphParameter);

	UFUNCTION(BlueprintPure, DisplayName = "Get Volume Sculpt Graph Parameter", CustomThunk, Category = "Voxel|Parameters", meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
	static void K2_GetVoxelVolumeSculptGraphParameter(
		const FVoxelVolumeSculptGraphWrapper& SculptGraph,
		FName Name,
		int32& Value)
	{
		unimplemented();
	}
	DECLARE_FUNCTION(execK2_GetVoxelVolumeSculptGraphParameter);

public:
	UFUNCTION(BlueprintCallable, DisplayName = "Set Height Graph Parameter", CustomThunk, Category = "Voxel|Parameters", meta = (AutoCreateRefTerm = "Value", CustomStructureParam = "Value,OutValue", BlueprintInternalUseOnly = "true"))
	static void K2_SetVoxelHeightGraphParameter(
		const FVoxelHeightGraphStampRef& Stamp,
		FName Name,
		const int32& Value,
		int32& OutValue)
	{
		unimplemented();
	}
	DECLARE_FUNCTION(execK2_SetVoxelHeightGraphParameter);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Volume Graph Parameter", CustomThunk, Category = "Voxel|Parameters", meta = (AutoCreateRefTerm = "Value", CustomStructureParam = "Value,OutValue", BlueprintInternalUseOnly = "true"))
	static void K2_SetVoxelVolumeGraphParameter(
		const FVoxelVolumeGraphStampRef& Stamp,
		FName Name,
		const int32& Value,
		int32& OutValue)
	{
		unimplemented();
	}
	DECLARE_FUNCTION(execK2_SetVoxelVolumeGraphParameter);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Height Sculpt Graph Parameter", CustomThunk, Category = "Voxel|Parameters", meta = (AutoCreateRefTerm = "Value", CustomStructureParam = "Value,OutValue", BlueprintInternalUseOnly = "true"))
	static void K2_SetVoxelHeightSculptGraphParameter(
		UPARAM(ref) FVoxelHeightSculptGraphWrapper& SculptGraph,
		FName Name,
		const int32& Value,
		UPARAM(DisplayName = "Sculpt Graph") FVoxelHeightSculptGraphWrapper& OutSculptGraph,
		int32& OutValue)
	{
		unimplemented();
	}
	DECLARE_FUNCTION(execK2_SetVoxelHeightSculptGraphParameter);

	UFUNCTION(BlueprintCallable, DisplayName = "Set Volume Sculpt Graph Parameter", CustomThunk, Category = "Voxel|Parameters", meta = (AutoCreateRefTerm = "Value", CustomStructureParam = "Value,OutValue", BlueprintInternalUseOnly = "true"))
	static void K2_SetVoxelVolumeSculptGraphParameter(
		UPARAM(ref) FVoxelVolumeSculptGraphWrapper& SculptGraph,
		FName Name,
		const int32& Value,
		UPARAM(DisplayName = "Sculpt Graph") FVoxelVolumeSculptGraphWrapper& OutSculptGraph,
		int32& OutValue)
	{
		unimplemented();
	}
	DECLARE_FUNCTION(execK2_SetVoxelVolumeSculptGraphParameter);

public:
	UFUNCTION(BlueprintPure, DisplayName = "Has Height Graph Parameter", Category = "Voxel|Parameters")
	static bool HasVoxelHeightGraphParameter(
		const FVoxelHeightGraphStampRef& Stamp,
		FName Name);

	UFUNCTION(BlueprintPure, DisplayName = "Has Volume Graph Parameter", Category = "Voxel|Parameters")
	static bool HasVoxelVolumeGraphParameter(
		const FVoxelVolumeGraphStampRef& Stamp,
		FName Name);

	UFUNCTION(BlueprintPure, DisplayName = "Has Height Sculpt Graph Parameter", Category = "Voxel|Parameters")
	static bool HasVoxelHeightSculptGraphParameter(
		const FVoxelHeightSculptGraphWrapper& SculptGraph,
		FName Name);

	UFUNCTION(BlueprintPure, DisplayName = "Has Volume Sculpt Graph Parameter", Category = "Voxel|Parameters")
	static bool HasVoxelVolumeSculptGraphParameter(
		const FVoxelVolumeSculptGraphWrapper& SculptGraph,
		FName Name);
};