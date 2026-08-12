// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelGraph.h"
#include "VoxelPCGGraph.generated.h"

UCLASS(BlueprintType, meta = (DisplayName = "Voxel PCG Graph", AssetSubMenu = "Graph"))
class VOXELPCG_API UVoxelPCGGraph : public UVoxelGraph
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGraph Interface
#if WITH_EDITOR
	virtual FFactoryInfo GetFactoryInfo() override;
#endif
	virtual UScriptStruct* GetOutputNodeStruct() const override;
	//~ End UVoxelGraph Interface
};