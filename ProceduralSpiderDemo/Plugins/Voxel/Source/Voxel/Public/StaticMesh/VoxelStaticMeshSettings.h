// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStaticMeshSettings.generated.h"

USTRUCT()
struct VOXEL_API FVoxelStaticMeshSettings
{
	GENERATED_BODY()

public:
	// Sweep direction to determine the voxel signs. If you have a plane, use Z
	UPROPERTY(Category = "Advanced", EditAnywhere)
	EVoxelAxis SweepDirection = EVoxelAxis::X;

	// Will do the sweep the other way around: eg, if SweepDirection = Z, the sweep will be done top to bottom if true
	UPROPERTY(Category = "Advanced", EditAnywhere)
	bool bReverseSweep = true;

	// If true, will assume every line of voxels starts outside the mesh, then goes inside, then goes outside it
	// Set to false if you have a shell and not a true volume
	// For example:
	// - sphere: set to true
	// - half sphere with no bottom geometry: set to false
	UPROPERTY(Category = "Config", EditAnywhere)
	bool bWatertight = true;

	// If true, will hide leaks by having holes instead
	// If false, leaks will be long tubes going through the entire asset
	UPROPERTY(Category = "Advanced", EditAnywhere)
	bool bHideLeaks = true;

public:
	bool operator==(const FVoxelStaticMeshSettings& Other) const
	{
		return
			SweepDirection == Other.SweepDirection &&
			bReverseSweep == Other.bReverseSweep &&
			bWatertight == Other.bWatertight &&
			bHideLeaks == Other.bHideLeaks;
	}

public:
	friend FArchive& operator<<(FArchive& Ar, FVoxelStaticMeshSettings& Settings)
	{
		Ar << Settings.SweepDirection;
		Ar << Settings.bReverseSweep;
		Ar << Settings.bWatertight;
		Ar << Settings.bHideLeaks;

		return Ar;
	}
};