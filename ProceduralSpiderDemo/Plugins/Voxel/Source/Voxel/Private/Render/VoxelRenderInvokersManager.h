// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelRenderInvokersContainer.h"

class UVoxelLODVolumeComponent;
class UVoxelCameraInvokerComponent;
struct FVoxelRenderSubsystem;

class VOXEL_API FVoxelRenderInvokersView : public TSharedFromThis<FVoxelRenderInvokersView>
{
public:
	FVoxelRenderInvokersView(const FVoxelRenderSubsystem& Subsystem);

	bool HasInvokers() const;
	TVoxelFuture<const FVoxelRenderInvokersContainer> GetInvokers(FVoxelDependencyCollector& DependencyCollector) const;

	void Tick(
		const TVoxelObjectPtr<UWorld>& World,
		const TVoxelSet<UVoxelLODVolumeComponent*>& InvokerComponents,
		const TVoxelSet<UVoxelCameraInvokerComponent*>& CameraInvokerComponents);

	bool Equal(
		const FVoxelRenderSubsystem& Subsystem,
		const FVoxelRenderSubsystem& PreviousSubsystem) const;

private:
	bool TickLODVolumes(
		const TVoxelSet<UVoxelLODVolumeComponent*>& LODVolumeComponents,
		TVoxelArray<FVoxelLODVolume>& OutLODVolumes,
		FString& OutComponentChanges);
	bool TickCameraInvokers(
		const TVoxelObjectPtr<UWorld>& WeakWorld,
		const TVoxelSet<UVoxelCameraInvokerComponent*>& CameraInvokerComponents,
		TVoxelArray<FVector>& OutCameraInvokers,
		FString& OutComponentChanges);

private:
	const bool bUseCameraInvoker;
	const float CameraInvokerPositionPrecision;
	const int32 VoxelSize;
	const int32 ChunkSize;
	const FTransform LocalToWorld;
	const TSharedRef<FVoxelDependency> Dependency;

	FVoxelFuture Future;
	TVoxelArray<FVector> LastCameraInvokers;

	struct FCachedLODVolume
	{
		FVector Position = FVector::ZeroVector;
		FVector Rotation = FVector::ZeroVector;
		int32 MinLOD = 0;
		EVoxelLODInvokerType Type = EVoxelLODInvokerType::Sphere;
		double SphereRadius = 0.f;
		FVector BoxExtent = FVector::ZeroVector;

		bool operator==(const FCachedLODVolume& Other) const;
		FString ToString() const;
	};
	TVoxelArray<FCachedLODVolume> LastLODVolumes;

	struct FCameraInvokerComponent
	{
		bool bIsCameraInvoker;
		TVoxelObjectPtr<const UVoxelCameraInvokerComponent> WeakComponent;

		FString GetName() const
		{
			if (bIsCameraInvoker)
			{
				return "Camera Invoker";
			}

			return WeakComponent.GetPathName();
		}
		bool operator==(const FCameraInvokerComponent& Other) const
		{
			return
				bIsCameraInvoker == Other.bIsCameraInvoker &&
				WeakComponent == Other.WeakComponent;
		}
		FORCEINLINE friend uint32 GetTypeHash(const FCameraInvokerComponent& Component)
		{
			return HashCombine(
				GetTypeHash(Component.bIsCameraInvoker),
				GetTypeHash(Component.WeakComponent));
		}
	};
#if VOXEL_INVALIDATION_TRACKING
	TVoxelArray<TVoxelObjectPtr<const UVoxelLODVolumeComponent>> LastLODVolumeComponents;
	TVoxelArray<FCameraInvokerComponent> LastCameraInvokerComponents;
#endif

	FVoxelCriticalSection CriticalSection;
	TSharedPtr<const FVoxelRenderInvokersContainer> Invokers_RequiresLock;
};

class VOXEL_API FVoxelRenderInvokersManager : public IVoxelWorldSubsystem
{
public:
	GENERATED_VOXEL_WORLD_SUBSYSTEM_BODY(FVoxelRenderInvokersManager);

	TSharedRef<FVoxelRenderInvokersView> MakeView(const FVoxelRenderSubsystem& Subsystem);

	//~ Begin IVoxelWorldSubsystem Interface
	virtual void Tick() override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End IVoxelWorldSubsystem Interface

private:
	TVoxelSet<UVoxelLODVolumeComponent*> LODVolumeComponents;
	TVoxelSet<UVoxelCameraInvokerComponent*> CameraInvokerComponents;
	TVoxelArray<TWeakPtr<FVoxelRenderInvokersView>> WeakViews;

	friend class UVoxelLODVolumeComponent;
	friend class UVoxelCameraInvokerComponent;
};