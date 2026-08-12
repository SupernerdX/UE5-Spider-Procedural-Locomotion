// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Spline/VoxelSplineComponent.h"
#include "Spline/VoxelSplineMetadata.h"

UVoxelSplineComponent::UVoxelSplineComponent()
{
	Metadata = CreateDefaultSubobject<UVoxelSplineMetadata>("Metadata");
}

void UVoxelSplineComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

USplineMetadata* UVoxelSplineComponent::GetSplinePointsMetadata()
{
	return Metadata;
}

const USplineMetadata* UVoxelSplineComponent::GetSplinePointsMetadata() const
{
	return Metadata;
}