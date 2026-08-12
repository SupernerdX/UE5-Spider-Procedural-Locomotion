// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNoClippingComponent.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "Surface/VoxelSurfaceTypeTable.h"

UVoxelNoClippingComponent::UVoxelNoClippingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelNoClippingComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	CheckLocation = Owner->GetActorLocation();
	LastCorrectLocation = CheckLocation;

	Future = Voxel::AsyncTask_NoQueue(ENamedThreads::AnyBackgroundThreadNormalTask, [
		WeakComponent = MakeVoxelObjectPtr(this),
		PositionToCheck = GetComponentLocation(),
		WeakLayer = FVoxelWeakStackLayer(Layer),
		SurfaceTypeTable = FVoxelSurfaceTypeTable::Get(),
		Layers = FVoxelLayers::Get(GetWorld())]
	{
		const FVoxelDoubleVectorBuffer PositionsBuffer(PositionToCheck);

		const FVoxelQuery Query(
			0,
			*Layers,
			*SurfaceTypeTable,
			FVoxelDependencyCollector::Null);

		return Query.SampleVolumeLayer(WeakLayer, PositionsBuffer);
	});
}

void UVoxelNoClippingComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Future.IsSet() &&
		!Future->IsComplete())
	{
		return;
	}

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (!Future.IsSet())
	{
		CheckLocation = Owner->GetActorLocation();
		LastCorrectLocation = CheckLocation;
	}
	else
	{
		const TSharedRef<FVoxelFloatBuffer> Value = Future->GetSharedValueChecked();
		if (ensure(Value->Num() == 1))
		{
			Execute(Owner, (*Value)[0]);
		}

		CheckLocation = Owner->GetActorLocation();
	}

	Future = Voxel::AsyncTask_NoQueue(ENamedThreads::AnyBackgroundThreadNormalTask, [
		PositionToCheck = GetComponentLocation(),
		WeakLayer = FVoxelWeakStackLayer(Layer),
		SurfaceTypeTable = FVoxelSurfaceTypeTable::Get(),
		Layers = FVoxelLayers::Get(GetWorld())]
	{
		const FVoxelDoubleVectorBuffer PositionsBuffer(PositionToCheck);

		const FVoxelQuery Query(
			0,
			*Layers,
			*SurfaceTypeTable,
			FVoxelDependencyCollector::Null);

		return Query.SampleVolumeLayer(WeakLayer, PositionsBuffer);
	});
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void UVoxelNoClippingComponent::Execute(const AActor* Owner, const float Distance)
{
	if (Distance >= 0.f ||
		FMath::IsNaN(Distance))
	{
		LastCorrectLocation = CheckLocation;
		return;
	}

	LOG_VOXEL(Log, "NoClippingComponent: teleporting %s to %s (Distance: %f)",
		*Owner->GetName(),
		*LastCorrectLocation.ToString(),
		Distance);

	const FVector ActorLocation = Owner->GetActorLocation();
	if (bAutoAdjustPlayer)
	{
		const FVector Delta = LastCorrectLocation - ActorLocation;
		if (USceneComponent* RootComponent = Owner->GetRootComponent())
		{
			RootComponent->MoveComponent(
				Delta,
				Owner->GetActorQuat(),
				false,
				nullptr,
				MOVECOMP_NoFlags,
				ETeleportType::ResetPhysics);
		}
	}

	OnTeleported.Broadcast(ActorLocation, LastCorrectLocation, Distance);
}