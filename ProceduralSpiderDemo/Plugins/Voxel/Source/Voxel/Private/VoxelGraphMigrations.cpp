// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.h"
#include "VoxelGraphMigration.h"
#include "Graphs/VoxelOutputNode_OutputHeight.h"
#include "Graphs/VoxelOutputNode_OutputVolume.h"
#include "Spline/VoxelOutputNode_OutputHeightSpline.h"
#include "Spline/VoxelOutputNode_OutputVolumeSpline.h"
#include "Surface/VoxelOutputNode_OutputSurface.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputHeight, MaterialPin, SurfaceTypePin);
	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputVolume, MaterialPin, SurfaceTypePin);
	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputHeightSpline, MaterialPin, SurfaceTypePin);
	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputVolumeSpline, MaterialPin, SurfaceTypePin);
	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputSurface, MaterialPin, SurfaceTypePin);

	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputHeight, SurfacePin, SurfaceTypePin);
	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputVolume, SurfacePin, SurfaceTypePin);
	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputHeightSpline, SurfacePin, SurfaceTypePin);
	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputVolumeSpline, SurfacePin, SurfaceTypePin);
	REGISTER_VOXEL_NODE_PIN_MIGRATION(FVoxelOutputNode_OutputSurface, SurfacePin, SurfaceTypePin);
}