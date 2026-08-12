// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelGraph.h"
#include "VoxelVolumeSculptGraph.generated.h"

UCLASS(BlueprintType, meta = (AssetSubMenu = "Graph"))
class VOXEL_API UVoxelVolumeSculptGraph : public UVoxelGraph
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