// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelCoreEditorMinimal.h"
#include "VoxelFunctionLibraryAssetFactory.generated.h"

UCLASS()
class UVoxelFunctionLibraryAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelFunctionLibraryAssetFactory();

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ End UFactory Interface
};