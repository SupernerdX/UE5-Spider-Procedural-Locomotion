// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelSimpleAssetToolkit.h"
#include "VoxelFunctionLibraryAsset.h"
#include "VoxelFunctionLibraryAssetToolkit.generated.h"

USTRUCT()
struct FVoxelFunctionLibraryAssetToolkit : public FVoxelToolkit
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	UPROPERTY()
	TObjectPtr<UVoxelFunctionLibraryAsset> Asset;

public:
	//~ Begin FVoxelToolkit Interface
	virtual TArray<FMode> GetModes() const override;
	//~ End FVoxelToolkit Interface
};