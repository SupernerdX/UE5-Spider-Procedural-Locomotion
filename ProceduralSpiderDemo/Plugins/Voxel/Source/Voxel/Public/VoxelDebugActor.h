// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStackLayer.h"
#include "VoxelDebugActor.generated.h"

class FVoxelLayers;
class FVoxelSurfaceTypeTable;

class VOXEL_API FVoxelDebugActorState
{
public:
	const TSharedRef<FVoxelLayers> Layers;
	const TSharedRef<FVoxelSurfaceTypeTable> SurfaceTypeTable;
	const int32 LOD;
	const FMatrix IndexToWorld;
	const FVoxelWeakStackLayer WeakLayer;

	static constexpr int32 Resolution = 128;

	FVoxelDebugActorState(
		const TSharedRef<FVoxelLayers>& Layers,
		const TSharedRef<FVoxelSurfaceTypeTable>& SurfaceTypeTable,
		const int32 LOD,
		const FMatrix& IndexToWorld,
		const FVoxelWeakStackLayer& WeakLayer)
		: Layers(Layers)
		, SurfaceTypeTable(SurfaceTypeTable)
		, LOD(LOD)
		, IndexToWorld(IndexToWorld)
		, WeakLayer(WeakLayer)
	{
	}

public:
	void Generate();

	bool IsInvalidated() const
	{
		return DependencyTracker->IsInvalidated();
	}
	TConstVoxelArrayView<FColor> GetColors() const
	{
		return Colors;
	}

private:
	TVoxelArray<FColor> Colors;
	TSharedPtr<FVoxelDependencyTracker> DependencyTracker;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS(NotBlueprintable)
class VOXEL_API AVoxelDebugActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 LOD = 0;

	UPROPERTY(EditAnywhere, Category = "Config")
	FVoxelStackLayer Layer;

public:
	AVoxelDebugActor();

	void Refresh();

	//~ Begin AActor Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
	//~ End AActor Interface

private:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> Material;

	UPROPERTY()
	TObjectPtr<UTexture2D> Texture;

	TVoxelOptional<TVoxelFuture<FVoxelDebugActorState>> FutureState;

	FVoxelFuture ApplyState(const TSharedRef<FVoxelDebugActorState>& State);
};