// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Heightmap/VoxelHeightmap.h"
#include "Heightmap/VoxelHeightmapStamp.h"
#include "Heightmap/VoxelHeightmap_Height.h"
#include "Heightmap/VoxelHeightmap_Weight.h"
#include "Surface/VoxelSurfaceTypeInterface.h"

DEFINE_VOXEL_FACTORY(UVoxelHeightmap);

UVoxelHeightmap::UVoxelHeightmap()
{
	Height = CreateDefaultSubobject<UVoxelHeightmap_Height>("Height");
}

#if WITH_EDITOR
void UVoxelHeightmap::MigrateMaterials()
{
	UVoxelSurfaceTypeInterface::Migrate(DefaultMaterial, DefaultSurfaceType);

	for (UVoxelHeightmap_Weight* Weight : Weights)
	{
		if (Weight)
		{
			UVoxelSurfaceTypeInterface::Migrate(Weight->Material, Weight->SurfaceType);
		}
	}
}
#endif

void UVoxelHeightmap::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void UVoxelHeightmap::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	MigrateMaterials();

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		OnChanged_EditorOnly.Broadcast();
	}
}
#endif