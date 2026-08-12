// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelFunctionLibraryAsset.h"

DEFINE_VOXEL_FACTORY(UVoxelFunctionLibraryAsset);

UVoxelFunctionLibraryAsset::UVoxelFunctionLibraryAsset()
{
	Graph = CreateDefaultSubobject<UVoxelGraph>("Graph");
}

void UVoxelFunctionLibraryAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}