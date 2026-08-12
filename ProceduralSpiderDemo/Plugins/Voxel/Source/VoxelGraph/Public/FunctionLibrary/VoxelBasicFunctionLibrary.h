// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFunctionLibrary.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "VoxelBasicFunctionLibrary.generated.h"

UCLASS()
class VOXELGRAPH_API UVoxelBasicFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	// Create unique seeds from float positions
	UFUNCTION(Category = "Random")
	FVoxelSeedBuffer HashPosition(
		const FVoxelVectorBuffer& Position,
		const FVoxelSeed& Seed,
		int32 RoundingDecimals = 3) const;

	UFUNCTION(Category = "Random")
	FVoxelVectorBuffer RandUnitVector(const FVoxelSeedBuffer& Seed) const;

	UFUNCTION(Category = "Misc", DisplayName = "Get LOD")
	int32 GetLOD() const;

	// Distance between two voxels at this LOD
	// = 1 << LOD
	UFUNCTION(Category = "Misc", DisplayName = "Get LOD Step")
	int32 GetLODStep() const;

	UFUNCTION(Category = "Misc")
	bool IsPreviewScene() const;

	UFUNCTION(Category = "Misc")
	ECollisionEnabled::Type GetCollisionEnabled(const FBodyInstance& BodyInstance) const;

public:
	UFUNCTION(Category = "Actor", meta = (Keywords = "matrix"))
	FTransform GetLocalToWorldTransform() const;

public:
	UFUNCTION(meta = (Internal))
	FVoxelRuntimePinValue ToBuffer(const FVoxelRuntimePinValue& Value) const;
};