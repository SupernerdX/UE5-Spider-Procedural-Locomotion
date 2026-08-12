// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Collision/VoxelInvokerCollisionSubsystem.h"
#include "Collision/VoxelCollider.h"
#include "Collision/VoxelCollisionInvoker.h"
#include "Collision/VoxelCollisionComponent.h"
#include "VoxelMesh.h"
#include "VoxelMesher.h"
#include "VoxelCubicMesher.h"
#include "VoxelRuntime.h"
#include "VoxelTaskContext.h"

void FVoxelInvokerCollisionSubsystem::LoadFromPrevious(FVoxelSubsystem& InPreviousSubsystem)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelInvokerCollisionSubsystem& PreviousSubsystem = CastStructChecked<FVoxelInvokerCollisionSubsystem>(InPreviousSubsystem);

	{
		PreviousChunkKeyToColliderRef = MoveTemp(PreviousSubsystem.ChunkKeyToColliderRef_RequiresLock);
		PreviousChunkKeyToCollisionComponent = MoveTemp(PreviousSubsystem.ChunkKeyToCollisionComponent);

		//ensure(PreviousSubsystem.PreviousChunkKeyToColliderRef.Num() == 0);
		ensure(PreviousSubsystem.PreviousChunkKeyToCollisionComponent.Num() == 0);

		InvokerView = PreviousSubsystem.InvokerView;
		PreviousSubsystem.InvokerView.Reset();
	}

	for (auto It = PreviousChunkKeyToColliderRef.CreateIterator(); It; ++It)
	{
		if (It.Value().DependencyTracker->IsInvalidated())
		{
			It.RemoveCurrent();
		}
	}

	if (PreviousSubsystem.GetConfig().LayerToRender != GetConfig().LayerToRender ||
		PreviousSubsystem.GetConfig().CollisionVoxelSize != GetConfig().CollisionVoxelSize ||
		PreviousSubsystem.GetConfig().MegaMaterialProxy != GetConfig().MegaMaterialProxy ||
		PreviousSubsystem.GetConfig().CollisionChunkSize != GetConfig().CollisionChunkSize ||
		PreviousSubsystem.GetConfig().BlockinessMetadata != GetConfig().BlockinessMetadata)
	{
		PreviousChunkKeyToColliderRef.Empty();
	}

	if (PreviousSubsystem.GetConfig().CollisionChunkSize != GetConfig().CollisionChunkSize ||
		PreviousSubsystem.GetConfig().CollisionVoxelSize != GetConfig().CollisionVoxelSize ||
		!PreviousSubsystem.GetConfig().LocalToWorld.Equals(GetConfig().LocalToWorld))
	{
		InvokerView = nullptr;
	}
}

void FVoxelInvokerCollisionSubsystem::Initialize()
{
	VOXEL_FUNCTION_COUNTER();

	if (!InvokerView)
	{
		InvokerView = FVoxelCollisionInvokerManager::Get(GetConfig().World)->MakeView(
			GetConfig().CollisionChunkSize * GetConfig().CollisionVoxelSize,
			GetConfig().LocalToWorld);
	}
}

void FVoxelInvokerCollisionSubsystem::Compute()
{
	VOXEL_FUNCTION_COUNTER();

	if (GetConfig().InvokerCollision.GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		return;
	}

	const TVoxelFuture<const TVoxelSet<FIntVector>> FutureChunkKeys = INLINE_LAMBDA
	{
		FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelInvokerCollisionSubsystem"));

		const TVoxelFuture<const TVoxelSet<FIntVector>> Result = GetTaskContext().Wrap(InvokerView->GetChunks(DependencyCollector));
		InvokerViewDependencyTracker = Finalize(DependencyCollector);
		return Result;
	};

	FutureChunkKeys.Then_AsyncThread([this](const TSharedRef<const TVoxelSet<FIntVector>>& ChunkKeys)
	{
		VOXEL_FUNCTION_COUNTER();

		ON_SCOPE_EXIT
		{
			// Do not empty otherwise HasCollision will crash
			//PreviousChunkKeyToColliderRef.Empty();
		};

		if (ChunkKeys->Num() == 0)
		{
			return;
		}

		{
			VOXEL_SCOPE_LOCK(CriticalSection);
			ChunkKeyToColliderRef_RequiresLock.Reserve(ChunkKeys->Num());
		}

		for (const FIntVector& ChunkKey : *ChunkKeys)
		{
			if (const FColliderRef* PreviousColliderRef = PreviousChunkKeyToColliderRef.Find(ChunkKey))
			{
				VOXEL_SCOPE_LOCK(CriticalSection);
				ChunkKeyToColliderRef_RequiresLock.Add_EnsureNew(ChunkKey, *PreviousColliderRef);
				continue;
			}

			Voxel::AsyncTask([this, ChunkKey]
			{
				{
					VOXEL_SCOPE_LOCK(CriticalSection);

					if (ChunkKeyToColliderRef_RequiresLock.Contains(ChunkKey))
					{
						// Already computed by ComputeInline
						return;
					}
				}

				const FColliderRef ColliderRef = CreateColliderRef(ChunkKey);

				VOXEL_SCOPE_LOCK(CriticalSection);
				// FindOrAdd for ComputeInline
				ChunkKeyToColliderRef_RequiresLock.FindOrAdd(ChunkKey) = ColliderRef;
			});
		}
	});
}

void FVoxelInvokerCollisionSubsystem::Render(FVoxelRuntime& Runtime)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);
	check(IsInGameThread());

	ChunkKeyToCollisionComponent.Reserve(ChunkKeyToColliderRef_RequiresLock.Num());

	const bool bHasRenderedInline = ChunkKeyToCollisionComponent.Num() > 0;

	for (const auto& It : ChunkKeyToColliderRef_RequiresLock)
	{
		const TSharedPtr<FVoxelCollider>& Collider = It.Value.Collider;
		if (!Collider)
		{
			continue;
		}

		UVoxelCollisionComponent* Component = nullptr;
		if (bHasRenderedInline)
		{
			// Check for an existing component
			TVoxelObjectPtr<UVoxelCollisionComponent> WeakComponent;
			if (ChunkKeyToCollisionComponent.RemoveAndCopyValue(It.Key, WeakComponent))
			{
				Component = WeakComponent.Resolve();
			}
		}
		if (!Component)
		{
			TVoxelObjectPtr<UVoxelCollisionComponent> WeakComponent;
			if (PreviousChunkKeyToCollisionComponent.RemoveAndCopyValue(It.Key, WeakComponent))
			{
				Component = WeakComponent.Resolve();
			}
		}
		if (!Component)
		{
			Component = Runtime.NewComponent<UVoxelCollisionComponent>();
		}
		if (!ensure(Component))
		{
			continue;
		}

		ChunkKeyToCollisionComponent.Add_EnsureNew(It.Key, Component);

		if (Component->GetCollider() == Collider)
		{
			// Already set
			continue;
		}

		SetupComponent(
			*Component,
			It.Key,
			Collider.ToSharedRef());
	}

	for (const auto& It : PreviousChunkKeyToCollisionComponent)
	{
		UVoxelCollisionComponent* Component = It.Value.Resolve();
		if (!ensureVoxelSlow(Component))
		{
			continue;
		}

		Component->ClearCollider();
		Runtime.RemoveComponent(Component);
	}
	PreviousChunkKeyToCollisionComponent.Empty();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelInvokerCollisionSubsystem::HasCollision(const TConstVoxelArrayView<FVector> Positions) const
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);
	// We need ChunkKeyToColliderRef_RequiresLock to be valid
	ensure(!IsPreviousSubsystem());

	const FVoxelConfig& Config = GetConfig();
	const int32 ChunkSize = Config.CollisionChunkSize * Config.CollisionVoxelSize;

	for (const FVector& Position : Positions)
	{
		const FVector LocalPosition = GetConfig().LocalToWorld.InverseTransformPosition(Position);
		const FIntVector ChunkKey = FVoxelUtilities::FloorToInt(LocalPosition / ChunkSize);

		// If PreviousChunkKeyToColliderRef contains ChunkPosition, the old state has collision
		if (!PreviousChunkKeyToColliderRef.Contains(ChunkKey) &&
			!ChunkKeyToColliderRef_RequiresLock.Contains(ChunkKey))
		{
			return false;
		}
	}

	return true;
}

void FVoxelInvokerCollisionSubsystem::ComputeInline(
	FVoxelRuntime& Runtime,
	const TConstVoxelArrayView<FVector> Positions)
{
	VOXEL_FUNCTION_COUNTER();
	VOXEL_SCOPE_LOCK(CriticalSection);
	// We need ChunkKeyToColliderRef_RequiresLock to be valid
	ensure(!IsPreviousSubsystem());
	check(IsInGameThread());

	const FVoxelConfig& Config = GetConfig();
	const int32 ChunkSize = Config.CollisionChunkSize * Config.CollisionVoxelSize;

	TVoxelSet<FIntVector> ChunkKeys;
	ChunkKeys.Reserve(Positions.Num());

	for (const FVector& Position : Positions)
	{
		const FVector LocalPosition = GetConfig().LocalToWorld.InverseTransformPosition(Position);
		const FIntVector ChunkKey = FVoxelUtilities::FloorToInt(LocalPosition / ChunkSize);

		// If PreviousChunkKeyToColliderRef contains ChunkPosition, the old state has collision
		if (!PreviousChunkKeyToColliderRef.Contains(ChunkKey) &&
			!ChunkKeyToColliderRef_RequiresLock.Contains(ChunkKey))
		{
			ChunkKeys.Add(ChunkKey);
		}
	}

	if (ChunkKeys.Num() == 0)
	{
		return;
	}

	VOXEL_FUNCTION_COUNTER_NUM(ChunkKeys.Num());

	for (const FIntVector& ChunkKey : ChunkKeys)
	{
		ChunkKeyToColliderRef_RequiresLock.Add_EnsureNew(ChunkKey);
	}

	Voxel::ParallelFor(ChunkKeys, [&](const FIntVector& ChunkKey)
	{
		ChunkKeyToColliderRef_RequiresLock[ChunkKey] = CreateColliderRef(ChunkKey);
	});

	ChunkKeyToCollisionComponent.ReserveGrow(ChunkKeys.Num());

	for (const FIntVector& ChunkKey : ChunkKeys)
	{
		const TSharedPtr<FVoxelCollider>& Collider = ChunkKeyToColliderRef_RequiresLock[ChunkKey].Collider;
		if (!Collider)
		{
			continue;
		}

		UVoxelCollisionComponent* Component = nullptr;
		{
			TVoxelObjectPtr<UVoxelCollisionComponent> WeakComponent;
			if (PreviousChunkKeyToCollisionComponent.RemoveAndCopyValue(ChunkKey, WeakComponent))
			{
				Component = WeakComponent.Resolve();
			}
		}
		if (!Component)
		{
			Component = Runtime.NewComponent<UVoxelCollisionComponent>();
		}
		if (!ensure(Component))
		{
			continue;
		}

		SetupComponent(
			*Component,
			ChunkKey,
			Collider.ToSharedRef());

		ChunkKeyToCollisionComponent.Add_EnsureNew(ChunkKey, Component);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelInvokerCollisionSubsystem::FColliderRef FVoxelInvokerCollisionSubsystem::CreateColliderRef(const FIntVector& ChunkKey) const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelDependencyCollector DependencyCollector(STATIC_FNAME("FVoxelCollider"));

	TSharedPtr<FVoxelMesh> Mesh;
	if (GetConfig().bEnableCubicMesher)
	{
		FVoxelCubicMesher CubicMesher(
			GetLayers(),
			GetSurfaceTypeTable(),
			DependencyCollector,
			GetConfig().LayerToRender,
			FInt64Vector3(ChunkKey) * GetConfig().CollisionChunkSize,
			GetConfig().CollisionVoxelSize,
			GetConfig().CollisionChunkSize,
			GetConfig().LocalToWorld,
			*GetConfig().MegaMaterialProxy);

		Mesh = CubicMesher.CreateMesh();
	}
	else
	{
		FVoxelMesher Mesher(
			GetLayers(),
			GetSurfaceTypeTable(),
			DependencyCollector,
			GetConfig().LayerToRender,
			0,
			FInt64Vector3(ChunkKey) * GetConfig().CollisionChunkSize,
			GetConfig().CollisionVoxelSize,
			GetConfig().CollisionChunkSize,
			GetConfig().LocalToWorld,
			*GetConfig().MegaMaterialProxy,
			GetConfig().BlockinessMetadata,
			false,
			0.f,
			0);

		Mesher.bQueryMetadata = false;

		Mesh = Mesher.CreateMesh(nullptr);
	}

	FColliderRef ColliderRef;
	ColliderRef.DependencyTracker = Finalize(DependencyCollector);

	if (Mesh)
	{
		ColliderRef.Collider = FVoxelCollider::Create(*Mesh);
	}

	return ColliderRef;
}

void FVoxelInvokerCollisionSubsystem::SetupComponent(
	UVoxelCollisionComponent& Component,
	const FIntVector& ChunkKey,
	const TSharedRef<const FVoxelCollider>& Collider) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelConfig& Config = GetConfig();

	Component.SetCollider(
		Collider,
		Config.InvokerCollision,
		Config.bDoubleSidedCollision,
		Config.bGenerateOverlapEvents,
		FTransform(
			FQuat::Identity,
			FVector(ChunkKey) * Config.CollisionChunkSize * Config.CollisionVoxelSize,
			FVector(Config.CollisionVoxelSize)));
}