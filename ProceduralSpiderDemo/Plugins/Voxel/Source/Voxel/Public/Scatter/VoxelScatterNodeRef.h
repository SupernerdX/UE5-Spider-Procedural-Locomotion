// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelScatterNodeRef.generated.h"

class AVoxelScatterActor;

USTRUCT()
struct VOXEL_API FVoxelScatterNodeRef
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AVoxelScatterActor> Actor;

	UPROPERTY()
	FGuid NodeGuid;
};

struct VOXEL_API FVoxelScatterNodeWeakRef
{
	TVoxelObjectPtr<AVoxelScatterActor> Actor;
	FGuid NodeGuid;

	FORCEINLINE bool operator==(const FVoxelScatterNodeWeakRef& Other) const
	{
		return
			Actor == Other.Actor &&
			NodeGuid == Other.NodeGuid;
	}
	FORCEINLINE friend uint32 GetTypeHash(const FVoxelScatterNodeWeakRef& WeakRef)
	{
		return HashCombine(
			GetTypeHash(WeakRef.Actor),
			GetTypeHash(WeakRef.NodeGuid));
	}
};