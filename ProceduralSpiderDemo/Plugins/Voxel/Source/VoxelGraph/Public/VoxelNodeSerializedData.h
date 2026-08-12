// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelPinValue.h"
#include "VoxelNodeSerializedData.generated.h"

USTRUCT()
struct VOXELGRAPH_API FVoxelNodeExposedPinValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FName Name;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelPinValue Value;

	bool operator==(const FName OtherName) const
	{
		return Name == OtherName;
	}

	// Required to compare nodes
	friend uint32 GetTypeHash(const FVoxelNodeExposedPinValue& InValue)
	{
		return FVoxelUtilities::MurmurHash(InValue.Name);
	}
};

USTRUCT()
struct FVoxelNodeVariadicPinSerializedData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FName> PinNames;
};

USTRUCT()
struct FVoxelNodeSerializedData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsValid = false;

	UPROPERTY()
	TMap<FName, FVoxelPinType> NameToPinType;

	UPROPERTY()
	TMap<FName, FVoxelNodeVariadicPinSerializedData> VariadicPinNameToSerializedData;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TSet<FName> ExposedPins;

	UPROPERTY()
	TArray<FVoxelNodeExposedPinValue> ExposedPinsValues;
#endif

	// Required to compare nodes
	friend uint32 GetTypeHash(const FVoxelNodeSerializedData& Data)
	{
		return FVoxelUtilities::MurmurHashMulti(
			Data.NameToPinType.Num(),
			Data.VariadicPinNameToSerializedData.Num()
#if WITH_EDITOR
			, Data.ExposedPins.Num()
			, Data.ExposedPinsValues.Num()
#endif
		);
	}
};