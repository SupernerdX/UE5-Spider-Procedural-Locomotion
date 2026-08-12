// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelTestManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTestLibrary.generated.h"

USTRUCT(BlueprintType)
struct FVoxelTestHandle
{
	GENERATED_BODY()

	FVoxelTestId Id;
};

UCLASS()
class VOXELTESTS_API UVoxelTestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tests")
	static FVoxelTestHandle StartTest(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tests")
	static void PassTest(const FVoxelTestHandle& Handle);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tests")
	static void FailTest(
		const FVoxelTestHandle& Handle,
		const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tests")
	static void TakeScreenshot(const FGuid& Guid);

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tests", meta = (WorldContext = "WorldContextObject"))
	static void SetNavMeshGeneration(UObject* WorldContextObject, bool bRuntime);
};