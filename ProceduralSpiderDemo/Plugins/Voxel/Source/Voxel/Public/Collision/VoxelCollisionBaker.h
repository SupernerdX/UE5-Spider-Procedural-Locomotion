// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelCollisionBaker.generated.h"

struct FVoxelCollisionChunk;
class FVoxelCollisionState;
class UVoxelCollisionComponent;

UCLASS(HideCategories = ("Rendering", "Physics", "LOD", "Activation", "Collision", "Cooking", "AssetUserData"))
class VOXEL_API UVoxelCollisionBakerRootComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UVoxelCollisionBakerRootComponent();

	//~ Begin UPrimitiveComponent Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End UPrimitiveComponent Interface
};

UCLASS()
class VOXEL_API UVoxelCollisionBakerData : public UObject
{
	GENERATED_BODY()

public:
	TSharedPtr<FVoxelCollisionState> State;

	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface
};

UCLASS()
class VOXEL_API AVoxelCollisionBaker : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	float Radius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Config")
	int32 VoxelSize = 100;

	UPROPERTY(EditAnywhere, Category = "Config")
	int32 ChunkSize = 32;

	UPROPERTY(EditAnywhere, Category = "Config")
	FVoxelStackLayer Layer;

	UPROPERTY(EditAnywhere, Category = "Config")
	FBodyInstance Collision;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bDoubleSidedCollision = false;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bGenerateOverlapEvents = false;

	UPROPERTY(EditAnywhere, Category = "Config", Transient, DuplicateTransient)
	bool bGenerate = false;

public:
	AVoxelCollisionBaker();

	void DestroyState();

	//~ Begin AActor Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void Destroyed() override;
	virtual void PostLoad() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override;
	//~ End AActor Interface

private:
	UPROPERTY()
	TObjectPtr<UVoxelCollisionBakerData> Data;

	TMap<FIntVector, TVoxelObjectPtr<UVoxelCollisionComponent>> ChunkKeyToComponent;

	void Render(const TVoxelMap<FIntVector, TSharedPtr<FVoxelCollisionChunk>>& ChunkKeyToChunk);
};