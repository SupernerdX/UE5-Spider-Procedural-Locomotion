// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Scatter/VoxelScatterInvokerComponent.h"
#include "VoxelScatterInvokersManager.h"

UVoxelScatterInvokerComponent::UVoxelScatterInvokerComponent()
{
	// Make sure to disable collision so that GetComponentsBoundingBox ignores us
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UVoxelScatterInvokerComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

void UVoxelScatterInvokerComponent::OnRegister()
{
	Super::OnRegister();

	const UWorld* World = GetWorld();
	if (ensure(World))
	{
		FVoxelScatterInvokersManager::Get(World)->InvokerComponents.Add_EnsureNew(this);
	}
}

void UVoxelScatterInvokerComponent::OnUnregister()
{
	const UWorld* World = GetWorld();
	if (ensure(World))
	{
		FVoxelScatterInvokersManager::Get(World)->InvokerComponents.Remove_Ensure(this);
	}

	Super::OnUnregister();
}