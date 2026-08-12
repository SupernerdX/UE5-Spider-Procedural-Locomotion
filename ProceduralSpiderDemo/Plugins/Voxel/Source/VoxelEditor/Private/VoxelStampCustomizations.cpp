// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelEditorMinimal.h"
#include "VoxelGraphStampCustomization.h"
#include "Graphs/VoxelHeightGraphStamp.h"
#include "Graphs/VoxelVolumeGraphStamp.h"
#include "Spline/VoxelHeightSplineStamp.h"
#include "Spline/VoxelVolumeSplineStamp.h"

DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelHeightGraphStamp, TVoxelGraphStampCustomization<FVoxelHeightGraphStamp>);
DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelVolumeGraphStamp, TVoxelGraphStampCustomization<FVoxelVolumeGraphStamp>);
DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelHeightSplineStamp, TVoxelGraphStampCustomization<FVoxelHeightSplineStamp>);
DEFINE_VOXEL_STRUCT_LAYOUT(FVoxelVolumeSplineStamp, TVoxelGraphStampCustomization<FVoxelVolumeSplineStamp>);