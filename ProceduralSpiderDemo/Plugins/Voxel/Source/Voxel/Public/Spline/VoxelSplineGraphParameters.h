// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSplineStamp.h"
#include "VoxelGraphParameters.h"

struct FSplineCurves;
struct FVoxelSplineMetadataRuntime;

namespace FVoxelGraphParameters
{
	struct FSplineStamp : FUniformParameter
	{
		bool bIs2D = false;
		bool bClosedLoop = false;
		int32 ReparamStepsPerSegment = 0;
		float SplineLength = 0.f;
		const FSplineCurves* SplineCurves = nullptr;
		const FVoxelSplineMetadataRuntime* MetadataRuntime = nullptr;
		TConstVoxelArrayView<FVoxelSplineSegment> Segments;
	};
}