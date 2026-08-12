// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelActorFactories.h"
#include "VoxelWorld.h"
#include "VoxelSettings.h"
#include "VoxelStampActor.h"
#include "VoxelDebugActor.h"
#include "Shape/VoxelShape.h"
#include "VoxelStampComponent.h"
#include "Shape/VoxelShapeStamp.h"
#include "Heightmap/VoxelHeightmap.h"
#include "Heightmap/VoxelHeightmapStamp.h"
#include "Scatter/VoxelScatterActor.h"
#include "Scatter/VoxelScatterGraph.h"
#include "Graphs/VoxelHeightGraph.h"
#include "Graphs/VoxelVolumeGraph.h"
#include "Graphs/VoxelHeightGraphStamp.h"
#include "Graphs/VoxelVolumeGraphStamp.h"
#include "Spline/VoxelHeightSplineGraph.h"
#include "Spline/VoxelHeightSplineStamp.h"
#include "Spline/VoxelVolumeSplineGraph.h"
#include "Spline/VoxelVolumeSplineStamp.h"
#include "StaticMesh/VoxelMeshStamp.h"
#include "StaticMesh/VoxelStaticMesh.h"
#include "Sculpt/Height/VoxelSculptHeight.h"
#include "Sculpt/Height/VoxelSculptHeightAsset.h"
#include "Sculpt/Volume/VoxelSculptVolume.h"
#include "Sculpt/Volume/VoxelSculptVolumeAsset.h"

DEFINE_VOXEL_PLACEABLE_ITEM_FACTORY(UActorFactory_VoxelWorld);
DEFINE_VOXEL_PLACEABLE_ITEM_FACTORY(UActorFactory_VoxelStampActor);
DEFINE_VOXEL_PLACEABLE_ITEM_FACTORY(UActorFactory_VoxelScatterActor);
DEFINE_VOXEL_PLACEABLE_ITEM_FACTORY(UActorFactory_VoxelDebugActor);
DEFINE_VOXEL_PLACEABLE_ITEM_FACTORY(UActorFactory_VoxelHeightSculptActor);
DEFINE_VOXEL_PLACEABLE_ITEM_FACTORY(UActorFactory_VoxelVolumeSculptActor);

UActorFactory_VoxelWorld::UActorFactory_VoxelWorld()
{
	DisplayName = INVTEXT("Voxel World");
	NewActorClass = AVoxelWorld::StaticClass();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UActorFactory_VoxelStampActor::UActorFactory_VoxelStampActor()
{
	DisplayName = INVTEXT("Voxel Stamp");
	NewActorClass = AVoxelStampActor::StaticClass();
}

bool UActorFactory_VoxelStampActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	// When dragging actor from spawn actor menu, the AssetData will be the actor class to spawn
	if (AssetData.GetAsset() == AVoxelStampActor::StaticClass())
	{
		return true;
	}

	const UClass* Class = AssetData.GetClass();
	if (!Class)
	{
		// Will make an empty stamp actor
		return true;
	}

	return
		Class->IsChildOf<UVoxelHeightmap>() ||
		Class->IsChildOf<UVoxelHeightGraph>() ||
		Class->IsChildOf<UVoxelVolumeGraph>() ||
		Class->IsChildOf<UVoxelHeightSplineGraph>() ||
		Class->IsChildOf<UVoxelVolumeSplineGraph>() ||
		Class->IsChildOf<UVoxelStaticMesh>();
}

void UActorFactory_VoxelStampActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	AVoxelStampActor& StampActor = *CastChecked<AVoxelStampActor>(NewActor);

	// Reset label set by FActorLabelUtilities::SetActorLabelUnique
	StampActor.LabelPrefix = {};

	const auto UpdateStamp = [&](FVoxelStamp& Stamp)
	{
		if (FVoxelHeightStamp* HeightStamp = Stamp.As<FVoxelHeightStamp>())
		{
			HeightStamp->Layer = GetDefault<UVoxelSettings>()->DefaultHeightLayer.LoadSynchronous();
		}
		if (FVoxelVolumeStamp* VolumeStamp = Stamp.As<FVoxelVolumeStamp>())
		{
			VolumeStamp->Layer = GetDefault<UVoxelSettings>()->DefaultVolumeLayer.LoadSynchronous();
		}

		Stamp.Priority = UVoxelStampComponent::GetNewStampPriority(NewActor->GetWorld(), Stamp);
		Stamp.StampSeed.Randomize();
	};

	if (UVoxelHeightmap* Heightmap = Cast<UVoxelHeightmap>(Asset))
	{
		FVoxelHeightmapStamp Stamp;
		Stamp.Heightmap = Heightmap;
		UpdateStamp(Stamp);
		StampActor.SetStamp(Stamp);
	}

	if (UVoxelHeightGraph* Graph = Cast<UVoxelHeightGraph>(Asset))
	{
		FVoxelHeightGraphStamp Stamp;
		Stamp.Graph = Graph;
		UpdateStamp(Stamp);
		StampActor.SetStamp(Stamp);
	}

	if (UVoxelVolumeGraph* Graph = Cast<UVoxelVolumeGraph>(Asset))
	{
		FVoxelVolumeGraphStamp Stamp;
		Stamp.Graph = Graph;
		UpdateStamp(Stamp);
		StampActor.SetStamp(Stamp);
	}

	if (UVoxelHeightSplineGraph* Graph = Cast<UVoxelHeightSplineGraph>(Asset))
	{
		FVoxelHeightSplineStamp Stamp;
		Stamp.Graph = Graph;
		UpdateStamp(Stamp);
		StampActor.SetStamp(Stamp);
	}

	if (UVoxelVolumeSplineGraph* Graph = Cast<UVoxelVolumeSplineGraph>(Asset))
	{
		FVoxelVolumeSplineStamp Stamp;
		Stamp.Graph = Graph;
		UpdateStamp(Stamp);
		StampActor.SetStamp(Stamp);
	}

	if (UVoxelStaticMesh* Mesh = Cast<UVoxelStaticMesh>(Asset))
	{
		FVoxelMeshStamp Stamp;
		Stamp.NewMesh = Mesh;
		UpdateStamp(Stamp);
		StampActor.SetStamp(Stamp);
	}

	if (!StampActor.GetStamp())
	{
		FVoxelMeshStamp Stamp;
		UpdateStamp(Stamp);
		StampActor.SetStamp(Stamp);
	}

	// Ensure preview & label is up to date
	StampActor.GetStampComponent().PostEditChange();
	AVoxelWorld::CreateNewIfNeeded_EditorOnly(NewActor);
}

UObject* UActorFactory_VoxelStampActor::GetAssetFromActorInstance(AActor* ActorInstance)
{
	const AVoxelStampActor* TypedActor = Cast<AVoxelStampActor>(ActorInstance);
	if (!ensure(TypedActor))
	{
		return nullptr;
	}

	if (const TSharedPtr<const FVoxelHeightmapStamp> Stamp = TypedActor->GetStamp().ToSharedPtr<FVoxelHeightmapStamp>())
	{
		return Stamp->Heightmap;
	}

	if (const TSharedPtr<const FVoxelMeshStamp> Stamp = TypedActor->GetStamp().ToSharedPtr<FVoxelMeshStamp>())
	{
		return Stamp->NewMesh;
	}

	if (const TSharedPtr<const FVoxelHeightGraphStamp> Stamp = TypedActor->GetStamp().ToSharedPtr<FVoxelHeightGraphStamp>())
	{
		return Stamp->Graph;
	}

	if (const TSharedPtr<const FVoxelVolumeGraphStamp> Stamp = TypedActor->GetStamp().ToSharedPtr<FVoxelVolumeGraphStamp>())
	{
		return Stamp->Graph;
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UActorFactory_VoxelScatterActor::UActorFactory_VoxelScatterActor()
{
	DisplayName = INVTEXT("Voxel Scatter Actor");
	NewActorClass = AVoxelScatterActor::StaticClass();
}

bool UActorFactory_VoxelScatterActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	// When dragging actor from spawn actor menu, the AssetData will be the actor class to spawn
	if (AssetData.GetAsset() == AVoxelScatterActor::StaticClass())
	{
		return true;
	}

	const UClass* Class = AssetData.GetClass();
	if (!Class)
	{
		// Will make an empty stamp actor
		return true;
	}

	return Class->IsChildOf<UVoxelScatterGraph>();
}

void UActorFactory_VoxelScatterActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	AVoxelScatterActor& ScatterActor = *CastChecked<AVoxelScatterActor>(NewActor);

	if (UVoxelScatterGraph* Graph = Cast<UVoxelScatterGraph>(Asset))
	{
		ScatterActor.Graph = Graph;
	}
}

UObject* UActorFactory_VoxelScatterActor::GetAssetFromActorInstance(AActor* ActorInstance)
{
	const AVoxelScatterActor* TypedActor = Cast<AVoxelScatterActor>(ActorInstance);
	if (!ensure(TypedActor))
	{
		return nullptr;
	}

	return TypedActor->Graph;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UActorFactory_VoxelDebugActor::UActorFactory_VoxelDebugActor()
{
	DisplayName = INVTEXT("Voxel Debug Actor");
	NewActorClass = AVoxelDebugActor::StaticClass();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UActorFactory_VoxelHeightSculptActor::UActorFactory_VoxelHeightSculptActor()
{
	DisplayName = INVTEXT("Voxel Height Sculpt Actor");
	NewActorClass = AVoxelSculptHeight::StaticClass();
}

bool UActorFactory_VoxelHeightSculptActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	// When dragging actor from spawn actor menu, the AssetData will be the actor class to spawn
	if (AssetData.GetAsset() == AVoxelSculptHeight::StaticClass())
	{
		return true;
	}

	const UClass* Class = AssetData.GetClass();
	if (!Class)
	{
		// Will make an empty sculpt actor
		return true;
	}

	return Class->IsChildOf<UVoxelSculptHeightAsset>();
}

void UActorFactory_VoxelHeightSculptActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	AVoxelSculptHeight& SculptActor = *CastChecked<AVoxelSculptHeight>(NewActor);
	SculptActor.GetStamp()->Layer = GetDefault<UVoxelSettings>()->DefaultHeightLayer.LoadSynchronous();
	SculptActor.GetStamp()->Priority = UVoxelStampComponent::GetNewStampPriority(NewActor->GetWorld(), SculptActor.GetStamp());
	SculptActor.GetComponent().ExternalAsset = Cast<UVoxelSculptHeightAsset>(Asset);

	AVoxelWorld::CreateNewIfNeeded_EditorOnly(NewActor);
}

UObject* UActorFactory_VoxelHeightSculptActor::GetAssetFromActorInstance(AActor* ActorInstance)
{
	const AVoxelSculptHeight* TypedActor = Cast<AVoxelSculptHeight>(ActorInstance);
	if (!ensure(TypedActor))
	{
		return nullptr;
	}

	return TypedActor->GetComponent().ExternalAsset;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UActorFactory_VoxelVolumeSculptActor::UActorFactory_VoxelVolumeSculptActor()
{
	DisplayName = INVTEXT("Voxel Volume Sculpt Actor");
	NewActorClass = AVoxelSculptVolume::StaticClass();
}

bool UActorFactory_VoxelVolumeSculptActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	// When dragging actor from spawn actor menu, the AssetData will be the actor class to spawn
	if (AssetData.GetAsset() == AVoxelSculptVolume::StaticClass())
	{
		return true;
	}

	const UClass* Class = AssetData.GetClass();
	if (!Class)
	{
		// Will make an empty sculpt actor
		return true;
	}

	return Class->IsChildOf<UVoxelSculptVolumeAsset>();
}

void UActorFactory_VoxelVolumeSculptActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	AVoxelSculptVolume& SculptActor = *CastChecked<AVoxelSculptVolume>(NewActor);
	SculptActor.GetStamp()->Layer = GetDefault<UVoxelSettings>()->DefaultVolumeLayer.LoadSynchronous();
	SculptActor.GetStamp()->Priority = UVoxelStampComponent::GetNewStampPriority(NewActor->GetWorld(), SculptActor.GetStamp());
	SculptActor.GetComponent().ExternalAsset = Cast<UVoxelSculptVolumeAsset>(Asset);

	AVoxelWorld::CreateNewIfNeeded_EditorOnly(NewActor);
}

UObject* UActorFactory_VoxelVolumeSculptActor::GetAssetFromActorInstance(AActor* ActorInstance)
{
	const AVoxelSculptVolume* TypedActor = Cast<AVoxelSculptVolume>(ActorInstance);
	if (!ensure(TypedActor))
	{
		return nullptr;
	}

	return TypedActor->GetComponent().ExternalAsset;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UActorFactory_VoxelPlaceStampActor::UActorFactory_VoxelPlaceStampActor()
{
	DisplayName = INVTEXT("Voxel Stamp Actor");
	NewActorClass = AVoxelStampActor::StaticClass();
}

bool UActorFactory_VoxelPlaceStampActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid())
	{
		return true;
	}

	if (!AssetData.GetClass()->IsChildOf<UScriptStruct>())
	{
		return false;
	}

	const UScriptStruct* Struct = Cast<UScriptStruct>(AssetData.GetAsset());
	return
		Struct->IsChildOf<FVoxelStamp>() ||
		Struct->IsChildOf<FVoxelShape>();
}

void UActorFactory_VoxelPlaceStampActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	AVoxelStampActor& StampActor = *CastChecked<AVoxelStampActor>(NewActor);

	if (UScriptStruct* Struct = Cast<UScriptStruct>(Asset))
	{
		FVoxelStampRef StampRef;
		if (Struct->IsChildOf<FVoxelStamp>())
		{
			StampRef.SetStruct_Editor(Struct);
		}
		else if (Struct->IsChildOf<FVoxelShape>())
		{
			FVoxelShapeStamp Stamp;
			Stamp.Shape = TVoxelInstancedStruct<FVoxelShape>(Struct);

			StampRef = FVoxelStampRef::New(Stamp);
		}

		if (StampRef)
		{
			StampRef->Priority = UVoxelStampComponent::GetNewStampPriority(NewActor->GetWorld(), StampRef);
			StampRef->StampSeed.Randomize();
		}

		StampActor.SetStamp(StampRef);
	}

	// Ensure preview & label is up to date
	StampActor.GetStampComponent().PostEditChange();
	AVoxelWorld::CreateNewIfNeeded_EditorOnly(NewActor);
}