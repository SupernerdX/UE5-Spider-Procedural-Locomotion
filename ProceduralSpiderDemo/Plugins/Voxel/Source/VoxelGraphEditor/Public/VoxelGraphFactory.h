// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "Factories/Factory.h"
#include "VoxelGraphFactory.generated.h"

class UVoxelGraph;

UCLASS()
class VOXELGRAPHEDITOR_API UVoxelGraphFactory : public UFactory
{
	GENERATED_BODY()

	UVoxelGraphFactory();

	UPROPERTY(Transient)
	TObjectPtr<UVoxelGraph> BaseGraph;

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ End UFactory Interface
};

UCLASS(Abstract)
class VOXELGRAPHEDITOR_API UVoxelGraphBaseFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelGraphBaseFactory();

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ End UFactory Interface
};