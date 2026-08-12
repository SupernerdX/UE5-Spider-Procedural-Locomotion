// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "VoxelActorFactories.generated.h"

class AVoxelStampActor;

UCLASS()
class UActorFactory_VoxelWorld : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactory_VoxelWorld();
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UActorFactory_VoxelStampActor : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactory_VoxelStampActor();

	//~ Begin UActorFactory Interface
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	//~ End UActorFactory Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UActorFactory_VoxelScatterActor : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactory_VoxelScatterActor();

	//~ Begin UActorFactory Interface
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	//~ End UActorFactory Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UActorFactory_VoxelDebugActor : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactory_VoxelDebugActor();
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UActorFactory_VoxelHeightSculptActor : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactory_VoxelHeightSculptActor();

	//~ Begin UActorFactory Interface
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	//~ End UActorFactory Interface
};

UCLASS()
class UActorFactory_VoxelVolumeSculptActor : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactory_VoxelVolumeSculptActor();

	//~ Begin UActorFactory Interface
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	//~ End UActorFactory Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UActorFactory_VoxelPlaceStampActor : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactory_VoxelPlaceStampActor();

	//~ Begin UActorFactory Interface
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	//~ End UActorFactory Interface
};