// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelWorldRootComponent.h"

UVoxelWorldRootComponent::UVoxelWorldRootComponent()
{
	// Otherwise MovementBaseUtility::UseRelativeLocation return true and characters are flipped when walking on planets
	Mobility = EComponentMobility::Static;
}

void UVoxelWorldRootComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}