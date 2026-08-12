// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampRef.h"
#include "VoxelFloatMetadata.h"
#include "VoxelLinearColorMetadata.h"
#include "VoxelStampBlueprintFunctionLibrary.h"
#include "VoxelStamp_K2.generated.h"

UCLASS()
class VOXEL_API UVoxelStamp_K2 : public UVoxelStampBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp")
	static bool IsValid(const FVoxelStampRef Stamp)
	{
		return Stamp.IsValid();
	}

	// Make a copy of this stamp
	// You can then call Set XXXX on the copy without having the original stamp be modified
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void MakeCopy(const FVoxelStampRef Stamp, FVoxelStampRef& Copy)
	{
		Copy = Stamp.MakeCopy();
	}

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	static void Make(FVoxelStampRef& Stamp)
	{
		Stamp = {};
	}

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	static void Break(const FVoxelStampRef Stamp)
	{
	}

public:
	// Get Transform
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp")
	static void GetTransform(const FVoxelStampRef Stamp, FTransform& Transform)
	{
		Transform = {};

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Transform = Stamp->Transform;
	}

	// Set Transform
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void SetTransform(const FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, const FTransform Transform)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->Transform = Transform;
		Stamp.Update();
	}

public:
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp")
	static void GetBehavior(FVoxelStampRef Stamp, UPARAM(meta=(Bitmask, BitmaskEnum="/Script/Voxel.EVoxelStampBehavior")) uint8& Behavior)
	{
		Behavior = {};

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Behavior = uint8(Stamp->Behavior);
	}

	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp")
	static bool HasBehavior(FVoxelStampRef Stamp, const EVoxelStampBehavior Behavior)
	{
		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return false;
		}

		return EnumHasAllFlags(Stamp->Behavior, Behavior);
	}

	// Set exact Behavior
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void SetBehavior(FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, const EVoxelStampBehavior Behavior)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->Behavior = Behavior;
		Stamp.Update();
	}

	// Adds Behavior
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void AddBehavior(FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, const EVoxelStampBehavior Behavior)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		EnumAddFlags(Stamp->Behavior, Behavior);
		Stamp.Update();
	}

	// Removes Behavior
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void RemoveBehavior(FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, const EVoxelStampBehavior Behavior)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		EnumRemoveFlags(Stamp->Behavior, Behavior);
		Stamp.Update();
	}

public:
	// Get Priority
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp")
	static void GetPriority(const FVoxelStampRef Stamp, int32& Priority)
	{
		Priority = {};

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Priority = Stamp->Priority;
	}

	// Set Priority
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void SetPriority(FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, const int32 Priority)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->Priority = Priority;
		Stamp.Update();
	}

public:
	// Get Smoothness
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp")
	static void GetSmoothness(const FVoxelStampRef Stamp, float& Smoothness)
	{
		Smoothness = {};

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Smoothness = Stamp->Smoothness;
	}

	// Set Smoothness
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void SetSmoothness(const FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, const float Smoothness)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->Smoothness = Smoothness;
		Stamp.Update();
	}

public:
	// Get Metadata Overrides
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp")
	static void GetMetadataOverrides(const FVoxelStampRef Stamp, FVoxelMetadataOverrides& MetadataOverrides)
	{
		MetadataOverrides = {};

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		MetadataOverrides = Stamp->MetadataOverrides;
	}

	// Set Metadata Overrides
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void SetMetadataOverrides(const FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, const FVoxelMetadataOverrides MetadataOverrides)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->MetadataOverrides = MetadataOverrides;
		Stamp.Update();
	}

	// Add a float metadata override
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void AddFloatMetadataOverride(const FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, UVoxelFloatMetadata* Metadata, const float Value)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		if (!Metadata)
		{
			VOXEL_MESSAGE(Error, "Metadata is null");
			return;
		}

		Stamp->MetadataOverrides.Overrides.Add(FVoxelMetadataOverride
		{
			Metadata,
			FVoxelPinValue::Make(Value)
		});
		Stamp.Update();
	}

	// Add a color metadata override
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void AddColorMetadataOverride(const FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, UVoxelLinearColorMetadata* Metadata, const FLinearColor Value)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		if (!Metadata)
		{
			VOXEL_MESSAGE(Error, "Metadata is null");
			return;
		}

		Stamp->MetadataOverrides.Overrides.Add(FVoxelMetadataOverride
		{
			Metadata,
			FVoxelPinValue::Make(Value)
		});
		Stamp.Update();
	}

public:
	// Get LOD Range
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp", DisplayName = "Get LOD Range")
	static void GetLODRange(const FVoxelStampRef Stamp, FInt32Interval& Range)
	{
		Range = {};

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Range = Stamp->LODRange;
	}

	// Set LOD Range
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp", DisplayName = "Set LOD Range")
	static void SetLODRange(const FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, const FInt32Interval Range)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->LODRange = Range;
		Stamp.Update();
	}

public:
	// Get Stamp Seed
	UFUNCTION(BlueprintPure, Category = "Voxel|Stamp")
	static void GetStampSeed(const FVoxelStampRef Stamp, FVoxelExposedSeed& Seed)
	{
		Seed = {};

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Seed = Stamp->StampSeed;
	}

	// Set Stamp Seed
	// This will automatically refresh the stamp
	UFUNCTION(BlueprintCallable, Category = "Voxel|Stamp")
	static void SetStampSeed(const FVoxelStampRef Stamp, UPARAM(meta = (DisplayName = "Stamp")) FVoxelStampRef& OutStamp, const FVoxelExposedSeed Seed)
	{
		OutStamp = Stamp;

		if (!Stamp.IsValid())
		{
			VOXEL_MESSAGE(Error, "Stamp is invalid");
			return;
		}

		Stamp->StampSeed = Seed;
		Stamp.Update();
	}
};