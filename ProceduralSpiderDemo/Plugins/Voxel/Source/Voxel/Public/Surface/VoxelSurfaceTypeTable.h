// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSurfaceType.h"

class FVoxelSmartSurfaceProxy;

class VOXEL_API FVoxelSurfaceTypeTable
{
public:
	const TVoxelArray<FVoxelSurfaceType> InvisibleSurfaceTypes;
	const TSharedRef<FVoxelDependency> InvisibleSurfaceTypesDependency;

	// Cubic surface types that don't occlude same-type neighbors
	const TVoxelSet<FVoxelSurfaceType> CubicNonSelfOccluding;
	// Cubic surface types that don't occlude different-type neighbors
	const TVoxelSet<FVoxelSurfaceType> CubicNonOtherOccluding;
	// Cubic surface types with masked blend mode (need masked material)
	const TVoxelSet<FVoxelSurfaceType> CubicMaskedTypes;
	// Cubic surface types with translucent blend mode (need separate mesh)
	const TVoxelSet<FVoxelSurfaceType> CubicTranslucentTypes;
	const TSharedRef<FVoxelDependency> CubicOcclusionDependency;

	const TVoxelMap<FVoxelSurfaceType, TSharedPtr<FVoxelSmartSurfaceProxy>> SurfaceTypeToSmartSurfaceProxy;

public:
	static TSharedRef<FVoxelSurfaceTypeTable> Get();
	static void Refresh();

private:
	FVoxelSurfaceTypeTable(
		TVoxelArray<FVoxelSurfaceType>&& InvisibleSurfaces,
		const TSharedRef<FVoxelDependency>& InvisibleSurfacesDependency,
		TVoxelSet<FVoxelSurfaceType>&& CubicNonSelfOccluding,
		TVoxelSet<FVoxelSurfaceType>&& CubicNonOtherOccluding,
		TVoxelSet<FVoxelSurfaceType>&& CubicMaskedTypes,
		TVoxelSet<FVoxelSurfaceType>&& CubicTranslucentTypes,
		const TSharedRef<FVoxelDependency>& CubicOcclusionDependency,
		TVoxelMap<FVoxelSurfaceType, TSharedPtr<FVoxelSmartSurfaceProxy>>&& SurfaceTypeToSmartSurfaceProxy)
		: InvisibleSurfaceTypes(MoveTemp(InvisibleSurfaces))
		, InvisibleSurfaceTypesDependency(InvisibleSurfacesDependency)
		, CubicNonSelfOccluding(MoveTemp(CubicNonSelfOccluding))
		, CubicNonOtherOccluding(MoveTemp(CubicNonOtherOccluding))
		, CubicMaskedTypes(MoveTemp(CubicMaskedTypes))
		, CubicTranslucentTypes(MoveTemp(CubicTranslucentTypes))
		, CubicOcclusionDependency(CubicOcclusionDependency)
		, SurfaceTypeToSmartSurfaceProxy(MoveTemp(SurfaceTypeToSmartSurfaceProxy))
	{
	}

	friend class FVoxelSurfaceTypeTableManager;
};