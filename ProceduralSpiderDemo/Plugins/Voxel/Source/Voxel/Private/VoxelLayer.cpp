// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelLayer.h"
#include "VoxelStackLayer.h"
#if WITH_EDITOR
#include "ObjectTools.h"
#endif

DEFINE_VOXEL_FACTORY(UVoxelHeightLayer);
DEFINE_VOXEL_FACTORY(UVoxelVolumeLayer);

EVoxelLayerType UVoxelLayer::GetType() const
{
	if (IsA<UVoxelHeightLayer>())
	{
		return EVoxelLayerType::Height;
	}
	else
	{
		ensure(IsA<UVoxelVolumeLayer>());
		return EVoxelLayerType::Volume;
	}
}

void UVoxelLayer::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void UVoxelLayer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!AssetIcon.bCustomIcon)
	{
		ThumbnailTools::CacheEmptyThumbnail(GetFullName(), GetPackage());
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelHeightLayer* UVoxelHeightLayer::Default()
{
	static ConstructorHelpers::FObjectFinder<UVoxelHeightLayer> Default(
		TEXT("/Voxel/Default/DefaultHeightLayer.DefaultHeightLayer"));
	ensure(Default.Object);
	return Default.Object;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelVolumeLayer* UVoxelVolumeLayer::Default()
{
	static ConstructorHelpers::FObjectFinder<UVoxelVolumeLayer> Default(
		TEXT("/Voxel/Default/DefaultVolumeLayer.DefaultVolumeLayer"));
	ensure(Default.Object);
	return Default.Object;
}