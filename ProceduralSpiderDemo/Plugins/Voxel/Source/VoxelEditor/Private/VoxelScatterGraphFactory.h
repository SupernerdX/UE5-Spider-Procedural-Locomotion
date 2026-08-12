// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraphFactory.h"
#include "Scatter/VoxelScatterGraph.h"
#include "VoxelScatterGraphFactory.generated.h"

UCLASS()
class UVoxelScatterGraphFactory : public UVoxelGraphBaseFactory
{
	GENERATED_BODY()

public:
	UVoxelScatterGraphFactory()
	{
		SupportedClass = UVoxelScatterGraph::StaticClass();
	}
};