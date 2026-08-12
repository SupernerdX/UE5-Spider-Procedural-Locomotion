// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class UVoxelScatterInvokerComponent;
struct FVoxelScatterSubsystem;

class VOXEL_API FVoxelScatterInvokersView : public TSharedFromThis<FVoxelScatterInvokersView>
{
public:
	FVoxelScatterInvokersView(const FVoxelScatterSubsystem& Subsystem);

	TSharedRef<const TVoxelArray<FVector>> GetInvokers(FVoxelDependencyCollector& DependencyCollector) const;

	void Tick(
		const TVoxelObjectPtr<UWorld>& WeakWorld,
		const TVoxelSet<UVoxelScatterInvokerComponent*>& InvokerComponents);

	bool Equal(
		const FVoxelScatterSubsystem& Subsystem,
		const FVoxelScatterSubsystem& PreviousSubsystem) const;

private:
	const bool bUseCameraInvoker;
	const float CameraInvokerPositionPrecision;
	const TSharedRef<FVoxelDependency> Dependency;

	TVoxelArray<FVector> LastInvokers;

	struct FInvokerComponent
	{
		bool bIsCameraInvoker;
		TVoxelObjectPtr<const UVoxelScatterInvokerComponent> WeakComponent;

		FString GetName() const
		{
			if (bIsCameraInvoker)
			{
				return "Camera Invoker";
			}

			return WeakComponent.GetPathName();
		}
		bool operator==(const FInvokerComponent& Other) const
		{
			return
				bIsCameraInvoker == Other.bIsCameraInvoker &&
				WeakComponent == Other.WeakComponent;
		}
		FORCEINLINE friend uint32 GetTypeHash(const FInvokerComponent& Component)
		{
			return HashCombine(
				GetTypeHash(Component.bIsCameraInvoker),
				GetTypeHash(Component.WeakComponent));
		}
	};
#if VOXEL_INVALIDATION_TRACKING
	TVoxelArray<FInvokerComponent> LastInvokerComponents;
#endif

	FVoxelCriticalSection CriticalSection;
	TSharedPtr<const TVoxelArray<FVector>> Invokers_RequiresLock;
};

class VOXEL_API FVoxelScatterInvokersManager : public IVoxelWorldSubsystem
{
public:
	GENERATED_VOXEL_WORLD_SUBSYSTEM_BODY(FVoxelScatterInvokersManager);

	TSharedRef<FVoxelScatterInvokersView> MakeView(const FVoxelScatterSubsystem& Subsystem);

	//~ Begin IVoxelWorldSubsystem Interface
	virtual void Tick() override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End IVoxelWorldSubsystem Interface

private:
	TVoxelSet<UVoxelScatterInvokerComponent*> InvokerComponents;
	TVoxelArray<TWeakPtr<FVoxelScatterInvokersView>> WeakViews;

	friend class UVoxelScatterInvokerComponent;
};