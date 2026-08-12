// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelHeightGraph.h"
#include "Graphs/VoxelOutputNode_OutputHeight.h"

#if WITH_EDITOR
UVoxelGraph::FFactoryInfo UVoxelHeightGraph::GetFactoryInfo()
{
	FFactoryInfo Result;

	Result.Category = "Height";
	Result.Description =
		"Used by Stamp actors to create shapes using XY positions and outputting height. "
		"Because of that this type cannot produce overhangs, caves or planets. "
		"They are usually used to create mountains or define the base of your terrain.";

	Result.Template = LoadObject<UVoxelHeightGraph>(nullptr, TEXT("/Voxel/HeightGraphTemplate.HeightGraphTemplate"));
	return Result;
}
#endif

UScriptStruct* UVoxelHeightGraph::GetOutputNodeStruct() const
{
	return FVoxelOutputNode_OutputHeight::StaticStruct();
}