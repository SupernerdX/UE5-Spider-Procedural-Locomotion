// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"

struct FVoxelStampRuntime;

struct FVoxelStampDelta
{
	FVoxelWeakStackLayer Layer;
	TSharedPtr<const FVoxelStampRuntime> Stamp;

	float DistanceBefore = 0.f;
	float DistanceAfter = 0.f;

	bool HasDifferentSign() const
	{
		if (FVoxelUtilities::IsNaN(DistanceBefore))
		{
			// If we are positive we are still "empty"
			return DistanceAfter < 0;
		}

		return (DistanceBefore >= 0) != (DistanceAfter >= 0);
	}
	float GetDistanceDeltaAbs() const
	{
		if (FVoxelUtilities::IsNaN(DistanceBefore))
		{
			return MAX_flt;
		}

		return FMath::Abs(DistanceAfter - DistanceBefore);
	}
};