// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelQuery.h"

extern VOXEL_API bool GVoxelShowAllQueries;

class VOXEL_API FVoxelQueryDebugDrawer
{
public:
	static void OnHeightLayerQuery(const FVoxelHeightBulkQuery& Query, double Time);
	static void OnHeightLayerQuery(const FVoxelHeightSparseQuery& Query, double Time);
	static void OnVolumeLayerQuery(const FVoxelVolumeBulkQuery& Query, double Time);
	static void OnVolumeLayerQuery(const FVoxelVolumeSparseQuery& Query, double Time);

private:
	static FLinearColor GetColor(
		double Time,
		int32 NumVoxels);
};