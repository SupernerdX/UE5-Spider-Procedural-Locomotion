// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampRef.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelStampBlueprintFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EVoxelStampCastResult : uint8
{
	CastSucceeded,
	CastFailed
};

UCLASS()
class VOXEL_API UVoxelStampBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	template<typename Type>
	static TVoxelStampRef<Type> CastToStampImpl(
		const FVoxelStampRef Stamp,
		EVoxelStampCastResult& Result)
	{
		const TVoxelStampRef<Type> OutStamp = Stamp.CastTo<Type>();

		Result = OutStamp.IsValid()
			? EVoxelStampCastResult::CastSucceeded
			: EVoxelStampCastResult::CastFailed;

		return OutStamp;
	}
};