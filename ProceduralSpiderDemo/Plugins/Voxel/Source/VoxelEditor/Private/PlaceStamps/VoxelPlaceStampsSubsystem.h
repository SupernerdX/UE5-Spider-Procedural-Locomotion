// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelPlaceStampsDefaults.h"
#include "VoxelPlaceStampsSubsystem.generated.h"

class SVoxelPlaceStampsTab;

UCLASS()
class UVoxelPlaceStampsSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin UEditorSubsystem Interface
	virtual void Deinitialize() override final;
	//~ End UEditorSubsystem Interface

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

public:
	TSharedPtr<SVoxelPlaceStampsTab> GetDrawerWidget();
	TSharedPtr<TStructOnScope<FVoxelPlaceStampDefaults>> GetStructOnScope();

public:
	static void UpdateActors(const TArray<AActor*>& Actors);
	static void SaveDefaults();
	static void RegisterDrawer();
	static void UnregisterDrawer();

private:
	TSharedPtr<SVoxelPlaceStampsTab> DrawerWidget;
	TSharedPtr<TStructOnScope<FVoxelPlaceStampDefaults>> StructOnScope;
};