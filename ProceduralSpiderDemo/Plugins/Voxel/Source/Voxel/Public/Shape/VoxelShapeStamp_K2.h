// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelShapeStampRef.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#include "VoxelShapeStamp_K2.generated.h"

////////////////////////////////////////////////////
///////// The code below is auto-generated /////////
////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelShapeStamp_K2 : public UVoxelStampBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Casting", meta = (ExpandEnumAsExecs = "Result"))
	static FVoxelShapeStampRef CastToShapeStamp(
		const FVoxelStampRef Stamp,
		EVoxelStampCastResult& Result)
	{
		return CastToStampImpl<FVoxelShapeStamp>(Stamp, Result);
	}

	// Make a copy of this stamp
	// You can then call Set XXXX on the copy without having the original stamp be modified
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Shape", DisplayName = "Get Shape Stamp")
	static void MakeCopy(
		const FVoxelShapeStampRef Stamp,
		FVoxelShapeStampRef& Copy)
	{
		Copy = FVoxelShapeStampRef(Stamp.MakeCopy());
	}

	// Get Surface Type
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp|Shape", DisplayName = "Get Surface Type")
	static void GetSurfaceType(
		UPARAM(Required) FVoxelShapeStampRef Stamp,
		UVoxelSurfaceTypeInterface*& SurfaceType)
	{
		SurfaceType = FVoxelUtilities::MakeSafe<UVoxelSurfaceTypeInterface*>();

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		SurfaceType = Stamp->SurfaceType;
	}

	// Set Surface Type
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp|Shape", DisplayName = "Set Surface Type")
	static void SetSurfaceType(
		UPARAM(Required) FVoxelShapeStampRef Stamp,
		UPARAM(DisplayName = "Stamp") FVoxelShapeStampRef& OutStamp,
		UVoxelSurfaceTypeInterface* SurfaceType)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->SurfaceType = SurfaceType;
		Stamp.Update();
	}
};