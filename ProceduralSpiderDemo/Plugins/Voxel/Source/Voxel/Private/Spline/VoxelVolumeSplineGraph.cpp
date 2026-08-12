// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Spline/VoxelVolumeSplineGraph.h"
#include "Spline/VoxelOutputNode_OutputVolumeSpline.h"

#if WITH_EDITOR
UVoxelGraph::FFactoryInfo UVoxelVolumeSplineGraph::GetFactoryInfo()
{
	FFactoryInfo Result;

	Result.Category = "Volume";
	Result.Description = "Used by Stamp actors to create a shape following a spline";

	Result.Template = LoadObject<UVoxelVolumeSplineGraph>(nullptr, TEXT("/Voxel/VolumeSplineGraphTemplate.VolumeSplineGraphTemplate"));
	return Result;
}
#endif

UScriptStruct* UVoxelVolumeSplineGraph::GetOutputNodeStruct() const
{
	return FVoxelOutputNode_OutputVolumeSpline::StaticStruct();
}