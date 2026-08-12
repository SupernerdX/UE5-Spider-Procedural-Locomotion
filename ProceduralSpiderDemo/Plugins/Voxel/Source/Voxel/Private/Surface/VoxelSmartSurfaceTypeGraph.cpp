// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSmartSurfaceTypeGraph.h"
#include "Surface/VoxelOutputNode_OutputSurface.h"
#include "Surface/VoxelSmartSurfaceType.h"

#if WITH_EDITOR
UVoxelGraph::FFactoryInfo UVoxelSmartSurfaceTypeGraph::GetFactoryInfo()
{
	FFactoryInfo Result;
	Result.Category = "Misc";
	Result.Description = "Used to decide which surface to render per vertex";
	Result.Template = LoadObject<UVoxelSmartSurfaceTypeGraph>(nullptr, TEXT("/Voxel/SmartSurfaceGraphTemplate.SmartSurfaceGraphTemplate"));
	return Result;
}
#endif

UScriptStruct* UVoxelSmartSurfaceTypeGraph::GetOutputNodeStruct() const
{
	return FVoxelOutputNode_OutputSurface::StaticStruct();
}

UVoxelSmartSurfaceType* UVoxelSmartSurfaceTypeGraph::GetPreviewSurface()
{
	if (!PreviewSurfaceType)
	{
		PreviewSurfaceType = NewObject<UVoxelSmartSurfaceType>();
		PreviewSurfaceType->Graph = this;
	}

	return PreviewSurfaceType;
}