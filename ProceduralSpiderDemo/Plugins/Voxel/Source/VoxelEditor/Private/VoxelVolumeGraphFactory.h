// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraphFactory.h"
#include "Graphs/VoxelVolumeGraph.h"
#include "VoxelVolumeGraphFactory.generated.h"

UCLASS()
class UVoxelVolumeGraphFactory : public UVoxelGraphBaseFactory
{
	GENERATED_BODY()

public:
	UVoxelVolumeGraphFactory()
	{
		SupportedClass = UVoxelVolumeGraph::StaticClass();
	}
};