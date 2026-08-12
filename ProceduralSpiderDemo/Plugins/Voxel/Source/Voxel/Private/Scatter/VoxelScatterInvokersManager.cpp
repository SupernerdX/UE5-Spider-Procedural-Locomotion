// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterInvokersManager.h"
#include "VoxelDependency.h"
#include "VoxelTaskContext.h"
#include "VoxelInvalidationCallstack.h"
#include "Scatter/VoxelScatterSubsystem.h"
#include "Scatter/VoxelScatterInvokerComponent.h"

FVoxelScatterInvokersView::FVoxelScatterInvokersView(const FVoxelScatterSubsystem& Subsystem)
	: bUseCameraInvoker(Subsystem.GetConfig().bUseCameraScatterInvoker)
	, CameraInvokerPositionPrecision(Subsystem.GetConfig().CameraScatterInvokerPositionPrecision)
	, Dependency(FVoxelDependency::Create("FVoxelScatterInvokersView"))
{
}

TSharedRef<const TVoxelArray<FVector>> FVoxelScatterInvokersView::GetInvokers(FVoxelDependencyCollector& DependencyCollector) const
{
	DependencyCollector.AddDependency(*Dependency);

	VOXEL_SCOPE_LOCK(CriticalSection);

	if (!ensure(Invokers_RequiresLock))
	{
		return MakeShared<const TVoxelArray<FVector>>();
	}

	return Invokers_RequiresLock.ToSharedRef();
}

void FVoxelScatterInvokersView::Tick(
	const TVoxelObjectPtr<UWorld>& WeakWorld,
	const TVoxelSet<UVoxelScatterInvokerComponent*>& InvokerComponents)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelArray<FVector> Invokers;
	Invokers.Reserve(InvokerComponents.Num());

#if !UE_BUILD_SHIPPING
	TVoxelArray<FInvokerComponent> NewInvokerComponents;
	NewInvokerComponents.Reserve(InvokerComponents.Num());
#endif

	for (const UVoxelScatterInvokerComponent* InvokerComponent : InvokerComponents)
	{
		if (!ensure(InvokerComponent) ||
			!InvokerComponent->bEnabled)
		{
			continue;
		}

#if !UE_BUILD_SHIPPING
		NewInvokerComponents.Add({ false, InvokerComponent });
#endif

		FVector Location = InvokerComponent->GetComponentLocation();
		Invokers.Add(FVoxelUtilities::RoundToFloat(Location / InvokerComponent->PositionPrecision) * InvokerComponent->PositionPrecision);
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
		NewInvokerComponents.Add({ true, nullptr });
#endif

		const FVector Location = Position.GetValue();
		Invokers.Add(FVoxelUtilities::RoundToFloat(Location / CameraInvokerPositionPrecision) * CameraInvokerPositionPrecision);
	};

	if (Invokers == LastInvokers)
	{
		VOXEL_SCOPE_LOCK(CriticalSection);
		if (!Invokers_RequiresLock)
		{
			Invokers_RequiresLock = MakeShared<const TVoxelArray<FVector>>();
		}
		return;
	}

	FString ComponentChanges = "FVoxelLODInvokerView changed:";
#if VOXEL_INVALIDATION_TRACKING
	if (NewInvokerComponents == LastInvokerComponents)
	{
		check(Invokers.Num() == LastInvokers.Num());

		for (int32 Index = 0; Index < LastInvokers.Num(); Index++)
		{
			if (LastInvokers[Index] == Invokers[Index])
			{
				continue;
			}

			if (!NewInvokerComponents[Index].bIsCameraInvoker &&
				!ensureVoxelSlow(NewInvokerComponents[Index].WeakComponent.Resolve()))
			{
				continue;
			}

			ComponentChanges += "\n\t" + NewInvokerComponents[Index].GetName() + ": " + LastInvokers[Index].ToString() + " -> " + Invokers[Index].ToString();
		}
	}
	else
	{
		const TVoxelSet<FInvokerComponent> NewInvokerComponentsSet(NewInvokerComponents);
		const TVoxelSet<FInvokerComponent> LastInvokerComponentsSet(LastInvokerComponents);

		for (const FInvokerComponent& InvokerComponentData : NewInvokerComponentsSet.Difference(LastInvokerComponentsSet))
		{
			ComponentChanges += "\n\t" + InvokerComponentData.GetName() + " added";
		}
		for (const FInvokerComponent& InvokerComponentData : LastInvokerComponentsSet.Difference(NewInvokerComponentsSet))
		{
			ComponentChanges += "\n\t" + InvokerComponentData.GetName() + " removed";
		}
	}

	LastInvokerComponents = NewInvokerComponents;
#endif

	LastInvokers = Invokers;

	VOXEL_SCOPE_COUNTER("FVoxelScatterInvokersView Async");

	{
		VOXEL_SCOPE_LOCK(CriticalSection);

		if (!Invokers_RequiresLock)
		{
			// Initialization, don't invalidate dependency
			Invokers_RequiresLock = MakeSharedCopy(Invokers);
			return;
		}

		if (Invokers == *Invokers_RequiresLock)
		{
			return;
		}

		Invokers_RequiresLock = MakeSharedCopy(Invokers);
	}

	FVoxelInvalidationScope Scope(ComponentChanges);
	Dependency->Invalidate();
}

bool FVoxelScatterInvokersView::Equal(
	const FVoxelScatterSubsystem& Subsystem,
	const FVoxelScatterSubsystem& PreviousSubsystem) const
{
	return
		PreviousSubsystem.GetConfig().bUseCameraScatterInvoker == Subsystem.GetConfig().bUseCameraScatterInvoker &&
		PreviousSubsystem.GetConfig().CameraScatterInvokerPositionPrecision == Subsystem.GetConfig().CameraScatterInvokerPositionPrecision;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelScatterInvokersView> FVoxelScatterInvokersManager::MakeView(const FVoxelScatterSubsystem& Subsystem)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	const TSharedRef<FVoxelScatterInvokersView> View = MakeShared<FVoxelScatterInvokersView>(Subsystem);

	// Ensure the view is up to date in the first query
	{
		FVoxelTaskScope Scope(*GVoxelGlobalTaskContext);
		View->Tick(
			GetWorld(),
			InvokerComponents);
	}

	WeakViews.Add(View);
	return View;
}

void FVoxelScatterInvokersManager::Tick()
{
	IVoxelWorldSubsystem::Tick();

	for (auto It = WeakViews.CreateIterator(); It; ++It)
	{
		const TSharedPtr<FVoxelScatterInvokersView> View = It->Pin();
		if (!View)
		{
			It.RemoveCurrentSwap();
			continue;
		}

		View->Tick(
			GetWorld(),
			InvokerComponents);
	}
}

void FVoxelScatterInvokersManager::AddReferencedObjects(FReferenceCollector& Collector)
{
	IVoxelWorldSubsystem::AddReferencedObjects(Collector);

	for (auto It = InvokerComponents.CreateIterator(); It; ++It)
	{
		UVoxelScatterInvokerComponent* InvokerComponentPtr = *It;
		TObjectPtr<UVoxelScatterInvokerComponent> InvokerComponent = ReinterpretCastRef<TObjectPtr<UVoxelScatterInvokerComponent>>(InvokerComponentPtr);
		Collector.AddReferencedObject(InvokerComponent);

		if (!ensure(InvokerComponent))
		{
			It.RemoveCurrent();
		}
	}
}