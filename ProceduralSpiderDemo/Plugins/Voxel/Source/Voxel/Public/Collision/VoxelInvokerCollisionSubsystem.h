// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelSubsystem.h"
#include "VoxelInvokerCollisionSubsystem.generated.h"

class FVoxelCollider;
class FVoxelCollisionInvokerView;
class UVoxelCollisionComponent;

USTRUCT()
struct VOXEL_API FVoxelInvokerCollisionSubsystem : public FVoxelSubsystem
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_BODY()

public:
	//~ Begin FVoxelSubsystem Interface
	virtual void LoadFromPrevious(FVoxelSubsystem& InPreviousSubsystem) override;
	virtual void Initialize() override;
	virtual void Compute() override;
	virtual void Render(FVoxelRuntime& Runtime) override;
	//~ End FVoxelSubsystem Interface

public:
	bool HasCollision(TConstVoxelArrayView<FVector> Positions) const;

	void ComputeInline(
		FVoxelRuntime& Runtime,
		TConstVoxelArrayView<FVector> Positions);

private:
	struct FColliderRef
	{
		TSharedPtr<FVoxelDependencyTracker> DependencyTracker;
		TSharedPtr<FVoxelCollider> Collider;
	};

	TSharedPtr<FVoxelCollisionInvokerView> InvokerView;
	TSharedPtr<FVoxelDependencyTracker> InvokerViewDependencyTracker;

	FVoxelCriticalSection CriticalSection;

	TVoxelMap<FIntVector, FColliderRef> ChunkKeyToColliderRef_RequiresLock;
	TVoxelMap<FIntVector, TVoxelObjectPtr<UVoxelCollisionComponent>> ChunkKeyToCollisionComponent;

	TVoxelMap<FIntVector, FColliderRef> PreviousChunkKeyToColliderRef;
	TVoxelMap<FIntVector, TVoxelObjectPtr<UVoxelCollisionComponent>> PreviousChunkKeyToCollisionComponent;

	FColliderRef CreateColliderRef(const FIntVector& ChunkKey) const;

	void SetupComponent(
		UVoxelCollisionComponent& Component,
		const FIntVector& ChunkKey,
		const TSharedRef<const FVoxelCollider>& Collider) const;
};