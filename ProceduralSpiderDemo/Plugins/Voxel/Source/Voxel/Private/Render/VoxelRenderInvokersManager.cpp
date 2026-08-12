// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelRenderInvokersManager.h"
#include "VoxelDependency.h"
#include "VoxelTaskContext.h"
#include "VoxelInvalidationCallstack.h"
#include "Render/VoxelRenderSubsystem.h"
#include "Render/VoxelLODVolumeComponent.h"
#include "Render/VoxelCameraInvokerComponent.h"

FVoxelRenderInvokersView::FVoxelRenderInvokersView(const FVoxelRenderSubsystem& Subsystem)
	: bUseCameraInvoker(Subsystem.GetConfig().bUseCameraInvoker)
	, CameraInvokerPositionPrecision(Subsystem.GetConfig().CameraInvokerPositionPrecision)
	, VoxelSize(Subsystem.GetConfig().VoxelSize)
	, ChunkSize(Subsystem.GetConfig().RenderChunkSize)
	, LocalToWorld(Subsystem.GetConfig().LocalToWorld)
	, Dependency(FVoxelDependency::Create("FVoxelRenderInvokersView"))
{
	ensure(ChunkSize > 0);
}

bool FVoxelRenderInvokersView::HasInvokers() const
{
	return
		LastLODVolumes.Num() > 0 ||
		LastCameraInvokers.Num() > 0;
}

TVoxelFuture<const FVoxelRenderInvokersContainer> FVoxelRenderInvokersView::GetInvokers(FVoxelDependencyCollector& DependencyCollector) const
{
	DependencyCollector.AddDependency(*Dependency);

	VOXEL_SCOPE_LOCK(CriticalSection);

	if (!Invokers_RequiresLock)
	{
		// Async to avoid recursive locks
		return Future.Then_AsyncThread(MakeStrongPtrLambda(this, [this]
		{
			VOXEL_SCOPE_LOCK(CriticalSection);
			return Invokers_RequiresLock.ToSharedRef();
		}));
	}

	return Invokers_RequiresLock.ToSharedRef();
}

void FVoxelRenderInvokersView::Tick(
	const TVoxelObjectPtr<UWorld>& World,
	const TVoxelSet<UVoxelLODVolumeComponent*>& InvokerComponents,
	const TVoxelSet<UVoxelCameraInvokerComponent*>& CameraInvokerComponents)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Future.IsComplete())
	{
		return;
	}

	FString ComponentChanges = "FVoxelRenderInvokersView changed:";

	TVoxelArray<FVoxelLODVolume> LODVolumes;
	const bool bLODVolumesChanged = TickLODVolumes(
		InvokerComponents,
		LODVolumes,
		ComponentChanges);

	TVoxelArray<FVector> CameraInvokers;
	const bool bCameraInvokersChanged = TickCameraInvokers(
		World,
		CameraInvokerComponents,
		CameraInvokers,
		ComponentChanges);

	if (!bCameraInvokersChanged &&
		!bLODVolumesChanged)
	{
		VOXEL_SCOPE_LOCK(CriticalSection);
		if (!Invokers_RequiresLock)
		{
			Invokers_RequiresLock = MakeShared<const FVoxelRenderInvokersContainer>();
		}
		return;
	}

	ensure(Future.IsComplete());

	Future = Voxel::AsyncTask(MakeWeakPtrLambda(this, [this, CameraInvokers = MakeSharedCopy(MoveTemp(CameraInvokers)), LODVolumes = MakeSharedCopy(MoveTemp(LODVolumes)), ComponentChanges = MoveTemp(ComponentChanges)]
	{
		VOXEL_SCOPE_COUNTER("FVoxelRenderInvokersView Async");

		const TSharedRef<FVoxelRenderInvokersContainer> NewInvokers = MakeShared<FVoxelRenderInvokersContainer>();
		NewInvokers->CameraInvokers = *CameraInvokers;
		NewInvokers->LODVolumes = *LODVolumes;

		FVoxelAABBTree::FElementArray Elements;
		Elements.SetNum(LODVolumes->Num());
		for (int32 Index = 0; Index < LODVolumes->Num(); Index++)
		{
			Elements.Set(Index, (*LODVolumes)[Index].GetBounds(), Index);
		}

		NewInvokers->Tree.Initialize(MoveTemp(Elements));

		{
			VOXEL_SCOPE_LOCK(CriticalSection);

			if (!Invokers_RequiresLock)
			{
				// Initialization, don't invalidate dependency
				Invokers_RequiresLock = NewInvokers;
				return;
			}

			if (*NewInvokers == *Invokers_RequiresLock)
			{
				return;
			}

			Invokers_RequiresLock = NewInvokers;
		}

		FVoxelInvalidationScope Scope(ComponentChanges);
		Dependency->Invalidate();
	}));
}

bool FVoxelRenderInvokersView::Equal(
	const FVoxelRenderSubsystem& Subsystem,
	const FVoxelRenderSubsystem& PreviousSubsystem) const
{
	return
		PreviousSubsystem.GetConfig().bUseCameraInvoker == Subsystem.GetConfig().bUseCameraInvoker &&
		PreviousSubsystem.GetConfig().CameraInvokerPositionPrecision == Subsystem.GetConfig().CameraInvokerPositionPrecision &&
		PreviousSubsystem.GetConfig().VoxelSize == Subsystem.GetConfig().VoxelSize &&
		PreviousSubsystem.GetConfig().RenderChunkSize == Subsystem.GetConfig().RenderChunkSize &&
		PreviousSubsystem.GetConfig().LocalToWorld.Equals(Subsystem.GetConfig().LocalToWorld);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelRenderInvokersView::TickLODVolumes(
	const TVoxelSet<UVoxelLODVolumeComponent*>& LODVolumeComponents,
	TVoxelArray<FVoxelLODVolume>& OutLODVolumes,
	FString& OutComponentChanges)
{
	VOXEL_FUNCTION_COUNTER();

	VOXEL_SCOPE_COUNTER_FORMAT("LODVolumeComponents.Num() = %d", LODVolumeComponents.Num());

	TVoxelArray<FCachedLODVolume> NewLODVolumes;
	NewLODVolumes.Reserve(LODVolumeComponents.Num());

	OutLODVolumes.Reserve(LODVolumeComponents.Num());

#if !UE_BUILD_SHIPPING
	TVoxelArray<TVoxelObjectPtr<const UVoxelLODVolumeComponent>> NewLODVolumeComponents;
	NewLODVolumeComponents.Reserve(LODVolumeComponents.Num());
#endif

	for (const UVoxelLODVolumeComponent* LODVolumeComponent : LODVolumeComponents)
	{
		if (!ensure(LODVolumeComponent) ||
			!LODVolumeComponent->bEnabled)
		{
			continue;
		}

#if !UE_BUILD_SHIPPING
		NewLODVolumeComponents.Add(LODVolumeComponent);
#endif

		const FTransform Transform = LocalToWorld.Inverse() * LODVolumeComponent->GetComponentTransform();

		const FVector Location = FVoxelUtilities::RoundToFloat(Transform.GetLocation() / LODVolumeComponent->PositionPrecision) * LODVolumeComponent->PositionPrecision;
		const FVector CachedRotation = INLINE_LAMBDA
		{
			if (LODVolumeComponent->Type != EVoxelLODInvokerType::Box)
			{
				return FVector::ZeroVector;
			}

			return FVoxelUtilities::RoundToFloat(Transform.Rotator().Euler() / LODVolumeComponent->RotationPrecision) * LODVolumeComponent->RotationPrecision;
		};
		const double SphereRadius = INLINE_LAMBDA -> double
		{
			if (LODVolumeComponent->Type != EVoxelLODInvokerType::Sphere)
			{
				return 0.f;
			}

#if WITH_EDITOR
			if (!Transform.GetScale3D().IsUniform())
			{
				VOXEL_MESSAGE(Error, "Sphere LOD Volume component scale must be uniform");
			}
#endif

			return LODVolumeComponent->SphereRadius * Transform.GetScale3D().X;
		};
		const FVector BoxExtent = INLINE_LAMBDA
		{
			if (LODVolumeComponent->Type != EVoxelLODInvokerType::Box)
			{
				return FVector::ZeroVector;
			}

			return LODVolumeComponent->BoxExtent * Transform.GetScale3D();
		};

		NewLODVolumes.Add(FCachedLODVolume
		{
			Location,
			CachedRotation,
			LODVolumeComponent->MinLOD,
			LODVolumeComponent->Type,
			SphereRadius,
			BoxExtent
		});

		switch (LODVolumeComponent->Type)
		{
		case EVoxelLODInvokerType::Sphere:
		{
			OutLODVolumes.Add(FVoxelLODVolume::MakeSphere(
				Location / VoxelSize / ChunkSize,
				LODVolumeComponent->SphereRadius / VoxelSize / ChunkSize,
				LODVolumeComponent->MinLOD));
			break;
		}
		case EVoxelLODInvokerType::Box:
		{
			OutLODVolumes.Add(FVoxelLODVolume::MakeBox(
				Location / VoxelSize / ChunkSize,
				Transform.GetRotation(),
				LODVolumeComponent->BoxExtent / VoxelSize / ChunkSize,
				LODVolumeComponent->MinLOD));
			break;
		}
		default: ensure(false); break;
		}
	}

	if (NewLODVolumes == LastLODVolumes)
	{
		return false;
	}

#if VOXEL_INVALIDATION_TRACKING
	if (NewLODVolumeComponents == LastLODVolumeComponents)
	{
		check(NewLODVolumes.Num() == LastLODVolumes.Num());

		for (int32 Index = 0; Index < LastLODVolumes.Num(); Index++)
		{
			if (LastLODVolumes[Index] == NewLODVolumes[Index])
			{
				continue;
			}

			const UVoxelLODVolumeComponent* InvokerComponent = NewLODVolumeComponents[Index].Resolve();
			if (!ensureVoxelSlow(InvokerComponent))
			{
				continue;
			}

			OutComponentChanges += "\n\t" + InvokerComponent->GetPathName() + ": " + LastLODVolumes[Index].ToString() + " -> " + NewLODVolumes[Index].ToString();
		}
	}
	else
	{
		const TVoxelSet<TVoxelObjectPtr<const UVoxelLODVolumeComponent>> NewInvokerComponentsSet(NewLODVolumeComponents);
		const TVoxelSet<TVoxelObjectPtr<const UVoxelLODVolumeComponent>> LastInvokerComponentsSet(LastLODVolumeComponents);

		for (const TVoxelObjectPtr<const UVoxelLODVolumeComponent> InvokerComponent : NewInvokerComponentsSet.Difference(LastInvokerComponentsSet))
		{
			OutComponentChanges += "\n\t" + InvokerComponent.GetPathName() + " added";
		}
		for (const TVoxelObjectPtr<const UVoxelLODVolumeComponent> InvokerComponent : LastInvokerComponentsSet.Difference(NewInvokerComponentsSet))
		{
			OutComponentChanges += "\n\t" + InvokerComponent.GetPathName() + " removed";
		}
	}

	LastLODVolumeComponents = NewLODVolumeComponents;
#endif

	LastLODVolumes = NewLODVolumes;

	return true;
}

bool FVoxelRenderInvokersView::TickCameraInvokers(
	const TVoxelObjectPtr<UWorld>& WeakWorld,
	const TVoxelSet<UVoxelCameraInvokerComponent*>& CameraInvokerComponents,
	TVoxelArray<FVector>& OutCameraInvokers,
	FString& OutComponentChanges)
{
	VOXEL_FUNCTION_COUNTER();

	VOXEL_SCOPE_COUNTER_FORMAT("CameraInvokerComponents.Num() = %d", CameraInvokerComponents.Num());

	TVoxelArray<FVector> NewCameraInvokers;
	NewCameraInvokers.Reserve(CameraInvokerComponents.Num());

	OutCameraInvokers.Reserve(CameraInvokerComponents.Num());

#if !UE_BUILD_SHIPPING
	TVoxelArray<FCameraInvokerComponent> NewCameraInvokerComponents;
	NewCameraInvokerComponents.Reserve(CameraInvokerComponents.Num());
#endif

	for (const UVoxelCameraInvokerComponent* InvokerComponent : CameraInvokerComponents)
	{
		if (!ensure(InvokerComponent) ||
			!InvokerComponent->bEnabled)
		{
			continue;
		}

#if !UE_BUILD_SHIPPING
		NewCameraInvokerComponents.Add({ false, InvokerComponent });
#endif

		FVector Location = FVoxelUtilities::RoundToFloat(InvokerComponent->GetComponentLocation() / InvokerComponent->PositionPrecision) * InvokerComponent->PositionPrecision;
		FVector ChunkLocation = LocalToWorld.InverseTransformPosition(Location) / VoxelSize / ChunkSize;
		OutCameraInvokers.Add(ChunkLocation);
		NewCameraInvokers.Add(Location);
	}

	INLINE_LAMBDA
	{
		if (!bUseCameraInvoker)
		{
			return;
		}

		const UWorld* World = WeakWorld.Resolve();
		if (!World)
		{
			return;
		}

		TOptional<FVector> Position = FVoxelUtilities::GetCameraPosition(World);
		if (!Position.IsSet())
		{
			return;
		}

#if !UE_BUILD_SHIPPING
		NewCameraInvokerComponents.Add({ true, nullptr });
#endif

		const FVector Location = FVoxelUtilities::RoundToFloat(Position.GetValue() / CameraInvokerPositionPrecision) * CameraInvokerPositionPrecision;
		const FVector ChunkLocation = LocalToWorld.InverseTransformPosition(Location) / VoxelSize / ChunkSize;
		OutCameraInvokers.Add(ChunkLocation);
		NewCameraInvokers.Add(Location);
	};

	if (NewCameraInvokers == LastCameraInvokers)
	{
		return false;
	}

#if VOXEL_INVALIDATION_TRACKING
	if (NewCameraInvokerComponents == LastCameraInvokerComponents)
	{
		check(NewCameraInvokers.Num() == LastCameraInvokers.Num());

		for (int32 Index = 0; Index < LastCameraInvokers.Num(); Index++)
		{
			if (LastCameraInvokers[Index] == NewCameraInvokers[Index])
			{
				continue;
			}

			if (!NewCameraInvokerComponents[Index].bIsCameraInvoker &&
				!ensureVoxelSlow(NewCameraInvokerComponents[Index].WeakComponent.Resolve()))
			{
				continue;
			}

			OutComponentChanges += "\n\t" + NewCameraInvokerComponents[Index].GetName() + ": " + LastCameraInvokers[Index].ToString() + " -> " + NewCameraInvokers[Index].ToString();
		}
	}
	else
	{
		const TVoxelSet<FCameraInvokerComponent> NewInvokerComponentsSet(NewCameraInvokerComponents);
		const TVoxelSet<FCameraInvokerComponent> LastInvokerComponentsSet(LastCameraInvokerComponents);

		for (const FCameraInvokerComponent& InvokerComponentData : NewInvokerComponentsSet.Difference(LastInvokerComponentsSet))
		{
			OutComponentChanges += "\n\t" + InvokerComponentData.GetName() + " added";
		}
		for (const FCameraInvokerComponent& InvokerComponentData : LastInvokerComponentsSet.Difference(NewInvokerComponentsSet))
		{
			OutComponentChanges += "\n\t" + InvokerComponentData.GetName() + " removed";
		}
	}

	LastCameraInvokerComponents = NewCameraInvokerComponents;
#endif

	LastCameraInvokers = NewCameraInvokers;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelRenderInvokersView::FCachedLODVolume::operator==(const FCachedLODVolume& Other) const
{
	return
		Position == Other.Position &&
		Rotation == Other.Rotation &&
		MinLOD == Other.MinLOD &&
		Type == Other.Type &&
		SphereRadius == Other.SphereRadius &&
		BoxExtent == Other.BoxExtent;
}

FString FVoxelRenderInvokersView::FCachedLODVolume::ToString() const
{
	switch (Type)
	{
	case EVoxelLODInvokerType::Sphere:
	{
		return FString::Printf(
			TEXT("P: %s, MinLOD: %d, Type: Sphere, Radius: %f"),
			*Position.ToString(),
			MinLOD,
			SphereRadius);
	}
	case EVoxelLODInvokerType::Box:
	{
		return FString::Printf(
			TEXT("P: %s, R: %s, MinLOD: %d, Type: Box, Extents: %s"),
			*Position.ToString(),
			*Rotation.ToString(),
			MinLOD,
			*BoxExtent.ToString());
	}
	}

	return "Invalid";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelRenderInvokersView> FVoxelRenderInvokersManager::MakeView(const FVoxelRenderSubsystem& Subsystem)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedRef<FVoxelRenderInvokersView> View = MakeShared<FVoxelRenderInvokersView>(Subsystem);

	// Ensure the view is up to date in the first query
	{
		FVoxelTaskScope Scope(*GVoxelGlobalTaskContext);
		View->Tick(
			GetWorld(),
			LODVolumeComponents,
			CameraInvokerComponents);
	}

	WeakViews.Add(View);
	return View;
}

void FVoxelRenderInvokersManager::Tick()
{
	IVoxelWorldSubsystem::Tick();

	for (auto It = WeakViews.CreateIterator(); It; ++It)
	{
		const TSharedPtr<FVoxelRenderInvokersView> View = It->Pin();
		if (!View)
		{
			It.RemoveCurrentSwap();
			continue;
		}

		View->Tick(
			GetWorld(),
			LODVolumeComponents,
			CameraInvokerComponents);
	}
}

void FVoxelRenderInvokersManager::AddReferencedObjects(FReferenceCollector& Collector)
{
	IVoxelWorldSubsystem::AddReferencedObjects(Collector);

	for (auto It = LODVolumeComponents.CreateIterator(); It; ++It)
	{
		UVoxelLODVolumeComponent* InvokerComponentPtr = *It;
		TObjectPtr<UVoxelLODVolumeComponent> InvokerComponent = ReinterpretCastRef<TObjectPtr<UVoxelLODVolumeComponent>>(InvokerComponentPtr);
		Collector.AddReferencedObject(InvokerComponent);

		if (!ensure(InvokerComponent))
		{
			It.RemoveCurrent();
		}
	}

	for (auto It = CameraInvokerComponents.CreateIterator(); It; ++It)
	{
		UVoxelCameraInvokerComponent* InvokerComponentPtr = *It;
		TObjectPtr<UVoxelLODVolumeComponent> InvokerComponent = ReinterpretCastRef<TObjectPtr<UVoxelLODVolumeComponent>>(InvokerComponentPtr);
		Collector.AddReferencedObject(InvokerComponent);

		if (!ensure(InvokerComponent))
		{
			It.RemoveCurrent();
		}
	}
}