// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelVolumeGraph.h"
#include "Graphs/VoxelOutputNode_OutputVolume.h"

#if WITH_EDITOR
UVoxelGraph::FFactoryInfo UVoxelVolumeGraph::GetFactoryInfo()
{
	FFactoryInfo Result;

	Result.Category = "Volume";
	Result.Description =
		"Used by Stamp actors to create shapes using XYZ positions and outputting distance from surface (negative meaning inside of shape, positive - outside). "
		"This allows to produce volumetric shapes, like caves or planets.";

	Result.Template = LoadObject<UVoxelVolumeGraph>(nullptr, TEXT("/Voxel/VolumeGraphTemplate.VolumeGraphTemplate"));
	return Result;
}
#endif

UScriptStruct* UVoxelVolumeGraph::GetOutputNodeStruct() const
{
	return FVoxelOutputNode_OutputVolume::StaticStruct();
}