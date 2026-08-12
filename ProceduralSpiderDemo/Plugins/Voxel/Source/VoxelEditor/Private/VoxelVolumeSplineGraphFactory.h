// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraphFactory.h"
#include "Spline/VoxelVolumeSplineGraph.h"
#include "VoxelVolumeSplineGraphFactory.generated.h"

UCLASS()
class UVoxelVolumeSplineGraphFactory : public UVoxelGraphBaseFactory
{
	GENERATED_BODY()

public:
	UVoxelVolumeSplineGraphFactory()
	{
		SupportedClass = UVoxelVolumeSplineGraph::StaticClass();
	}
};