// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelGraph.h"
#include "VoxelFunctionLibraryAsset.generated.h"

UCLASS(meta = (VoxelAssetType, AssetColor = Blue))
class VOXELGRAPH_API UVoxelFunctionLibraryAsset : public UVoxelAsset
{
	GENERATED_BODY()

public:
	UVoxelFunctionLibraryAsset();

	UVoxelGraph& GetGraph()
	{
		return *Graph;
	}
	const UVoxelGraph& GetGraph() const
	{
		return *Graph;
	}

public:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface

private:
	UPROPERTY()
	TObjectPtr<UVoxelGraph> Graph;
};