// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelGraphFactory.h"
#include "Sculpt/Height/VoxelHeightSculptGraph.h"
#include "Sculpt/Volume/VoxelVolumeSculptGraph.h"
#include "VoxelSculptGraphFactories.generated.h"

UCLASS()
class UVoxelHeightSculptGraphFactory : public UVoxelGraphBaseFactory
{
	GENERATED_BODY()

public:
	UVoxelHeightSculptGraphFactory()
	{
		SupportedClass = UVoxelHeightSculptGraph::StaticClass();
	}
};

UCLASS()
class UVoxelVolumeSculptGraphFactory : public UVoxelGraphBaseFactory
{
	GENERATED_BODY()

public:
	UVoxelVolumeSculptGraphFactory()
	{
		SupportedClass = UVoxelVolumeSculptGraph::StaticClass();
	}
};