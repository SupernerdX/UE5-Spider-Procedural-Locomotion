// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampComponentUtilities.h"
#include "VoxelHeightStamp.h"
#include "VoxelVolumeStamp.h"
#include "VoxelStampComponent.h"
#include "VoxelInstancedStampComponent.h"
#include "Scatter/VoxelScatterActor.h"

bool FVoxelStampComponentUtilities::ShouldRender(const USceneComponent* Component)
{
	if (!IsValid(Component) ||
		Component->IsTemplate() ||
		!Component->IsVisible() ||
		!Component->IsRegistered())
	{
		return false;
	}

	const AActor* Owner = Component->GetOwner();
	if (!Owner)
	{
		return false;
	}

	if (Owner->HasAnyFlags(RF_ClassDefaultObject))
	{
		// Likely in a blueprint CDO
		return false;
	}

#if WITH_EDITOR
	if (Owner->IsHiddenEd())
	{
		return false;
	}
#endif

	return true;
}

FVoxelBox FVoxelStampComponentUtilities::GetLocalBounds(const FVoxelStampRuntime& Stamp)
{
	VOXEL_FUNCTION_COUNTER();

	if (Stamp.FailedToInitialize())
	{
		return {};
	}

	return Stamp.GetLocalBounds();
}

void FVoxelStampComponentUtilities::DispatchBeginPlay(const UWorld* World)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(World))
	{
		return;
	}

	TVoxelSet<AActor*> Actors;
	Actors.Reserve(128);

	ForEachObjectOfClass<UVoxelStampComponentBase>([&](const UVoxelStampComponentBase& Component)
	{
		AActor* Owner = Component.GetOwner();
		if (!Owner ||
			Owner->IsActorBeginningPlay() ||
			Owner->HasActorBegunPlay() ||
			Owner->GetWorld() != World)
		{
			return;
		}

		Actors.Add(Owner);
	});

	ForEachObjectOfClass<UVoxelInstancedStampComponent>([&](const UVoxelInstancedStampComponent& Component)
	{
		AActor* Owner = Component.GetOwner();
		if (!Owner ||
			Owner->IsActorBeginningPlay() ||
			Owner->HasActorBegunPlay() ||
			Owner->GetWorld() != World)
		{
			return;
		}

		Actors.Add(Owner);
	});

	ForEachObjectOfClass<AVoxelScatterActor>([&](AVoxelScatterActor& Actor)
	{
		if (Actor.IsActorBeginningPlay() ||
			Actor.HasActorBegunPlay() ||
			Actor.GetWorld() != World)
		{
			return;
		}

		Actors.Add(&Actor);
	});

	VOXEL_SCOPE_COUNTER("DispatchBeginPlay");

	for (AActor* Actor : Actors)
	{
		Actor->DispatchBeginPlay();
	}
}