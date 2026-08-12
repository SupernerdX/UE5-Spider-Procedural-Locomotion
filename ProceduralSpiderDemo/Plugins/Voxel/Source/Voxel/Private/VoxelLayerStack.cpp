// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelLayerStack.h"
#include "VoxelLayer.h"
#if WITH_EDITOR
#include "ObjectTools.h"
#include "AssetRegistry/IAssetRegistry.h"
#endif

DEFINE_VOXEL_FACTORY(UVoxelLayerStack);

#if WITH_EDITOR
VOXEL_RUN_ON_STARTUP_EDITOR()
{
	IAssetRegistry::GetChecked().OnInMemoryAssetCreated().AddLambda([](UObject* Asset)
	{
		if (!Asset ||
			!Asset->IsA<UVoxelLayerStack>())
		{
			return;
		}

		UVoxelLayerStack::OnChanged.Broadcast();
	});
}
#endif

FSimpleMulticastDelegate UVoxelLayerStack::OnChanged;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelLayerStack::UVoxelLayerStack()
{
	HeightLayers.Add(UVoxelHeightLayer::Default());
	VolumeLayers.Add(UVoxelVolumeLayer::Default());
}

UVoxelLayerStack* UVoxelLayerStack::Default()
{
	static ConstructorHelpers::FObjectFinder<UVoxelLayerStack> Default(
		TEXT("/Voxel/Default/DefaultStack.DefaultStack"));
	ensure(Default.Object);
	return Default.Object;
}

void UVoxelLayerStack::PostLoad()
{
	Super::PostLoad();

	ensure(IsInGameThread());
	OnChanged.Broadcast();
}

void UVoxelLayerStack::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void UVoxelLayerStack::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!AssetIcon.bCustomIcon)
	{
		ThumbnailTools::CacheEmptyThumbnail(GetFullName(), GetPackage());
	}

	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
	{
		return;
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_OWN_MEMBER_NAME(MaxDistance))
	{
		Voxel::RefreshAll();
	}

	OnChanged.Broadcast();
}
#endif