// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelGraph.h"
#include "VoxelMinimal.h"
#include "VoxelScatterGraph.generated.h"

UCLASS(BlueprintType, meta = (AssetSubMenu = "Graph"))
class VOXEL_API UVoxelScatterGraph : public UVoxelGraph
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGraph Interface
#if WITH_EDITOR
	virtual FFactoryInfo GetFactoryInfo() override;
#endif
	//~ End UVoxelGraph Interface
};