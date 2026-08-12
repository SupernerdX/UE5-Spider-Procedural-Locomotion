// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelCollisionInvoker.generated.h"

class UVoxelCollisionInvokerComponent;

class VOXEL_API FVoxelCollisionInvokerView : public TSharedFromThis<FVoxelCollisionInvokerView>
{
public:
	FVoxelCollisionInvokerView(
		int32 ChunkSize,
		const FTransform& LocalToWorld);

	TVoxelFuture<const TVoxelSet<FIntVector>> GetChunks(FVoxelDependencyCollector& DependencyCollector) const;

	void Tick(const TVoxelSet<UVoxelCollisionInvokerComponent*>& InvokerComponents);

private:
	const int32 ChunkSize;
	const FTransform LocalToWorld;
	const TSharedRef<FVoxelDependency> Dependency;

	FVoxelFuture Future;
	TVoxelArray<FSphere> LastInvokers;

#if VOXEL_INVALIDATION_TRACKING
	TVoxelArray<TVoxelObjectPtr<const UVoxelCollisionInvokerComponent>> LastInvokerComponents;
#endif

	FVoxelCriticalSection CriticalSection;
	TSharedPtr<const TVoxelSet<FIntVector>> Chunks_RequiresLock;
};

class VOXEL_API FVoxelCollisionInvokerManager : public IVoxelWorldSubsystem
{
public:
	GENERATED_VOXEL_WORLD_SUBSYSTEM_BODY(FVoxelCollisionInvokerManager);

	void LogInvokers();

	TSharedRef<FVoxelCollisionInvokerView> MakeView(
		int32 ChunkSize,
		const FTransform& LocalToWorld);

	//~ Begin IVoxelWorldSubsystem Interface
	virtual void Tick() override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End IVoxelWorldSubsystem Interface

private:
	TVoxelSet<UVoxelCollisionInvokerComponent*> InvokerComponents;
	TVoxelArray<TWeakPtr<FVoxelCollisionInvokerView>> WeakViews;

	void CheckVoxelWorlds();

	friend UVoxelCollisionInvokerComponent;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS(ClassGroup = Voxel, HideCategories = ("Physics", "LOD", "Activation", "Collision", "Cooking", "AssetUserData"), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelCollisionInvokerComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker")
	bool bEnabled = true;

	// In world space, not affected by scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker")
	float Radius = 1000.f;

	// If true will check on tick if a voxel world has not computed chunks at our location, and will compute them inline
	// This might cause hitches but will prevent players from falling through the ground
	// Ignores Radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invoker")
	bool bWaitForVoxelWorld = true;

	UVoxelCollisionInvokerComponent();

	//~ Begin UPrimitiveComponent Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End UPrimitiveComponent Interface
};