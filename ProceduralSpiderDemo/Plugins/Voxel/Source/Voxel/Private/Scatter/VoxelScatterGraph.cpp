// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterGraph.h"

#if WITH_EDITOR
UVoxelGraph::FFactoryInfo UVoxelScatterGraph::GetFactoryInfo()
{
	FFactoryInfo Result;
	Result.Category = "Misc";
	Result.Description = "";
	Result.Template = LoadObject<UVoxelScatterGraph>(nullptr, TEXT("/Voxel/ScatterGraphTemplate.ScatterGraphTemplate"));
	return Result;
}
#endif