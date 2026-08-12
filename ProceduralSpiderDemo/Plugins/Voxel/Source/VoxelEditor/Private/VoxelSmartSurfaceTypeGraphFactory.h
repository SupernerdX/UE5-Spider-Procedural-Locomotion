// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraphFactory.h"
#include "Surface/VoxelSmartSurfaceTypeGraph.h"
#include "VoxelSmartSurfaceTypeGraphFactory.generated.h"

UCLASS()
class UVoxelSmartSurfaceTypeGraphFactory : public UVoxelGraphBaseFactory
{
	GENERATED_BODY()

public:
	UVoxelSmartSurfaceTypeGraphFactory()
	{
		SupportedClass = UVoxelSmartSurfaceTypeGraph::StaticClass();
	}
};