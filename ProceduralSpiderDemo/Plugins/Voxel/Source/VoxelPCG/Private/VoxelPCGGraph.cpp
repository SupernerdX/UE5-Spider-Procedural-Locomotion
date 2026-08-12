// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPCGGraph.h"
#include "VoxelOutputNode_OutputPoints.h"

#if WITH_EDITOR
UVoxelGraph::FFactoryInfo UVoxelPCGGraph::GetFactoryInfo()
{
	FFactoryInfo Result;
	Result.DisplayNameOverride = GetClass()->GetDisplayNameText().ToString();
	Result.Category = "Misc";
	Result.Description =
		"This is a graph used only in PCG graphs to run logic made with Voxel Graph nodes. "
		"Can only work with points, cannot change terrain data. "
		"It is sometimes faster or easier to alter points in VPCG graphs (like doing math with point attributes) compared to PCG graphs.";

	Result.Template = LoadObject<UVoxelPCGGraph>(nullptr, TEXT("/Voxel/PCGGraphTemplate.PCGGraphTemplate"));
	return Result;
}
#endif

UScriptStruct* UVoxelPCGGraph::GetOutputNodeStruct() const
{
	return FVoxelOutputNode_OutputPoints::StaticStruct();
}