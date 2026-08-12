// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelAABBTree.h"
#include "Render/VoxelLODVolumeComponent.h"

struct FVoxelLODVolume
{
	FVector Position = FVector::ZeroVector;

	// Sphere
	double Radius = 0.f;

	// Box
	FQuat InvRotation = FQuat::Identity;
	FVector Extents = FVector::ZeroVector;
	FMatrix AbsRotationMatrix = FMatrix::Identity;

	EVoxelLODInvokerType Type = EVoxelLODInvokerType::Sphere;
	int32 MinLOD = 0;

public:
	static FVoxelLODVolume MakeSphere(
		const FVector& Position,
		double Radius,
		int32 MinLOD);

	static FVoxelLODVolume MakeBox(
		const FVector& Position,
		const FQuat& Rotation,
		const FVector& Extents,
		int32 MinLOD);

public:
	FVoxelBox GetBounds() const;

	bool operator==(const FVoxelLODVolume& Other) const;
};

struct VOXEL_API FVoxelRenderInvokersContainer
{
	TVoxelArray<FVector> CameraInvokers;
	TVoxelArray<FVoxelLODVolume> LODVolumes;
	FVoxelAABBTree Tree;

	bool operator==(const FVoxelRenderInvokersContainer& Other) const;
};