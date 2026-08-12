// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "PCGManagedVoxelInstancedStampComponent.h"
#include "PCGComponent.h"
#include "VoxelInstancedStampComponent.h"

void UPCGManagedVoxelInstancedStampComponent::PostLoad()
{
	Super::PostLoad();

	GetComponent();
}

void UPCGManagedVoxelInstancedStampComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	SerializeVoxelVersion(Ar);
}

void UPCGManagedVoxelInstancedStampComponent::ForgetComponent()
{
	Super::ForgetComponent();
	CachedRawComponentPtr = nullptr;
}

bool UPCGManagedVoxelInstancedStampComponent::ReleaseIfUnused(TSet<TSoftObjectPtr<AActor>>& OutActorsToDelete)
{
	if (Super::ReleaseIfUnused(OutActorsToDelete) ||
		!GetComponent())
	{
		return true;
	}

	if (GetComponent()->NumStamps() == 0)
	{
		GeneratedComponent->DestroyComponent();
		ForgetComponent();
		return true;
	}

	return false;
}

void UPCGManagedVoxelInstancedStampComponent::ResetComponent()
{
	if (UVoxelInstancedStampComponent* InstancedStampComponent = GetComponent())
	{
		InstancedStampComponent->ClearStamps();
	}
}

void UPCGManagedVoxelInstancedStampComponent::MarkAsUsed()
{
	const bool bWasMarkedUnused = bIsMarkedUnused;
	Super::MarkAsUsed();

	if (!bWasMarkedUnused)
	{
		return;
	}

	if (UVoxelInstancedStampComponent* InstancedStampComponent = GetComponent())
	{
		const bool bHasPreviousRootLocation = bHasRootLocation;

		// Keep track of the current root location so if we reuse this later we are able to update this appropriately
		if (USceneComponent* RootComponent = InstancedStampComponent->GetAttachmentRoot())
		{
			bHasRootLocation = true;
			RootLocation = RootComponent->GetComponentLocation();
		}
		else
		{
			bHasRootLocation = false;
			RootLocation = FVector::ZeroVector;
		}

		if (bHasPreviousRootLocation != bHasRootLocation ||
			(InstancedStampComponent->GetComponentLocation() - RootLocation).SquaredLength() > UE_DOUBLE_SMALL_NUMBER)
		{
			// Reset the rotation/scale to be identity otherwise if the root component transform has changed, the final transform will be wrong
			// Since this is technically 'moving' the ISM, we need to unregister it before moving otherwise we could get a warning that we're moving a component with static mobility
			InstancedStampComponent->UnregisterComponent();
			InstancedStampComponent->SetWorldTransform(FTransform(FQuat::Identity, RootLocation, FVector::OneVector));
			InstancedStampComponent->RegisterComponent();
		}
	}
}

void UPCGManagedVoxelInstancedStampComponent::MarkAsReused()
{
	Super::MarkAsReused();

	if (UVoxelInstancedStampComponent* InstancedStampComponent = GetComponent())
	{
		// Reset the rotation/scale to be identity otherwise if the root component transform has changed, the final transform will be wrong
		FVector TentativeRootLocation = RootLocation;

		if (!bHasRootLocation)
		{
			if (USceneComponent* RootComponent = InstancedStampComponent->GetAttachmentRoot())
			{
				TentativeRootLocation = RootComponent->GetComponentLocation();
			}
		}

		if ((InstancedStampComponent->GetComponentLocation() - TentativeRootLocation).SquaredLength() > UE_DOUBLE_SMALL_NUMBER)
		{
			// Since this is technically 'moving' the ISM, we need to unregister it before moving otherwise we could get a warning that we're moving a component with static mobility
			InstancedStampComponent->UnregisterComponent();
			InstancedStampComponent->SetWorldTransform(FTransform(FQuat::Identity, TentativeRootLocation, FVector::OneVector));
			InstancedStampComponent->RegisterComponent();
		}
	}
}

void UPCGManagedVoxelInstancedStampComponent::SetRootLocation(const FVector& InRootLocation)
{
	bHasRootLocation = true;
	RootLocation = InRootLocation;
}

UVoxelInstancedStampComponent* UPCGManagedVoxelInstancedStampComponent::GetComponent() const
{
	if (!CachedRawComponentPtr)
	{
		UVoxelInstancedStampComponent* GeneratedComponentPtr = Cast<UVoxelInstancedStampComponent>(GeneratedComponent.Get());

		// Implementation note:
		// There is no surefire way to make sure that we can use the raw pointer UNLESS it is from the same owner
		if (GeneratedComponentPtr &&
			Cast<UPCGComponent>(GetOuter()) &&
			GeneratedComponentPtr->GetOwner() == Cast<UPCGComponent>(GetOuter())->GetOwner())
		{
			CachedRawComponentPtr = GeneratedComponentPtr;
		}

		return GeneratedComponentPtr;
	}

	return CachedRawComponentPtr;
}

void UPCGManagedVoxelInstancedStampComponent::SetComponent(UVoxelInstancedStampComponent* InComponent)
{
	GeneratedComponent = InComponent;
	CachedRawComponentPtr = InComponent;
}