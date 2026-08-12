// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelFunctionLibrary.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "VoxelShapeFunctionLibrary.generated.h"

UCLASS()
class VOXEL_API UVoxelShapeFunctionLibrary : public UVoxelFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Shape")
	UPARAM(DisplayName = "Distance") FVoxelFloatBuffer MakeSphere(
		UPARAM(meta = (PositionPin)) const FVoxelDoubleVectorBuffer& Position,
		const FVoxelDoubleBuffer& Radius = 1000.f) const;

	UFUNCTION(Category = "Shape")
	FVoxelBox MakeSphereBounds(double Radius = 1000.f) const;

public:
	UFUNCTION(Category = "Shape")
	UPARAM(DisplayName = "Distance") FVoxelFloatBuffer MakeCube(
		UPARAM(meta = (PositionPin)) const FVoxelDoubleVectorBuffer& Position,
		const FVoxelDoubleVectorBuffer& Size = 1000.f,
		const FVoxelDoubleBuffer& Roundness = 0.f) const;

	UFUNCTION(Category = "Shape")
	FVoxelBox MakeCubeBounds(const FVoxelDoubleVector& Size = 1000.f) const;
};