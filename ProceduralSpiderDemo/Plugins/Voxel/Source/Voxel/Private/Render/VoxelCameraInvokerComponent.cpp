// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelCameraInvokerComponent.h"
#include "Render/VoxelRenderInvokersManager.h"

UVoxelCameraInvokerComponent::UVoxelCameraInvokerComponent()
{
	// Make sure to disable collision so that GetComponentsBoundingBox ignores us
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UVoxelCameraInvokerComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

void UVoxelCameraInvokerComponent::OnRegister()
{
	Super::OnRegister();

	const UWorld* World = GetWorld();
	if (ensure(World))
	{
		FVoxelRenderInvokersManager::Get(World)->CameraInvokerComponents.Add_EnsureNew(this);
	}
}

void UVoxelCameraInvokerComponent::OnUnregister()
{
	const UWorld* World = GetWorld();
	if (ensure(World))
	{
		FVoxelRenderInvokersManager::Get(World)->CameraInvokerComponents.Remove_Ensure(this);
	}

	Super::OnUnregister();
}