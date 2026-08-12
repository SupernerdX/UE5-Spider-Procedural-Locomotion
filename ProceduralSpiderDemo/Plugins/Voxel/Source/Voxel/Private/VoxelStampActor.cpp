// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelStampActor.h"
#include "VoxelSettings.h"
#include "VoxelStampComponent.h"

AVoxelStampActor::AVoxelStampActor()
{
	StampComponent = CreateDefaultSubobject<UVoxelStampComponent>("Stamp");
	RootComponent = StampComponent;

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif
}

UVoxelStampComponent& AVoxelStampActor::GetStampComponent() const
{
	check(StampComponent);
	return *StampComponent;
}

void AVoxelStampActor::PostLoad()
{
	Super::PostLoad();

	if (RootComponent != StampComponent)
	{
		StampComponent->SetRelativeLocation(RootComponent->GetRelativeLocation());
		StampComponent->SetRelativeRotation(RootComponent->GetRelativeRotation());
		StampComponent->SetRelativeScale3D(RootComponent->GetRelativeScale3D());

		RootComponent = StampComponent;
	}

	// Fixup any glitched label prefixes that could cause issues with FNames
	if (LabelPrefix.Len() > 200)
	{
		LabelPrefix = LabelPrefix.Left(200);
	}
}

void AVoxelStampActor::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

#if WITH_EDITOR
void AVoxelStampActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property == &FindFPropertyChecked_ByName(AActor, "ActorLabel"))
	{
		GetStampComponent().OnActorLabelChanged();
	}

	// PostEditChangeProperty will reregister and call UpdateStampActorLabel, call it last
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AVoxelStampActor::PostDuplicate(const EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	if (DuplicateMode != EDuplicateMode::Normal ||
		!StampComponent ||
		!StampComponent->GetStamp().IsValid())
	{
		return;
	}

	// Do not update priorities if there's more than one stamp component
	int32 NumStampComponents = 0;
	for (UActorComponent* Component : GetComponents())
	{
		if (Cast<UVoxelStampComponent>(Component))
		{
			NumStampComponents++;
			if (NumStampComponents > 1)
			{
				return;
			}
		}
	}

	StampComponent->GetStamp()->Priority = UVoxelStampComponent::GetNewStampPriority(GetWorld(), StampComponent->GetStamp());

	if (GetDefault<UVoxelSettings>()->bRegenerateSeedOnStampDuplicate)
	{
		StampComponent->GetStamp()->StampSeed.Randomize();
		StampComponent->UpdateStamp();
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelStampRef AVoxelStampActor::GetStamp() const
{
	return GetStampComponent().GetStamp();
}

FVoxelStampRef AVoxelStampActor::SetStamp(const FVoxelStampRef& NewStamp)
{
	GetStampComponent().SetStamp(NewStamp);
	return GetStampComponent().GetStamp();
}

void AVoxelStampActor::SetStamp(const FVoxelStamp& NewStamp)
{
	GetStampComponent().SetStamp(NewStamp);
}

void AVoxelStampActor::UpdateStamp()
{
	GetStampComponent().UpdateStamp();
}