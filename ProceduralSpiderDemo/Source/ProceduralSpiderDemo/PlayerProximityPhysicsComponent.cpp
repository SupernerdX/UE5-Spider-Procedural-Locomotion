#include "PlayerProximityPhysicsComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Render/VoxelLODVolumeComponent.h"
#include "SpiderPawnMovement.h"

UPlayerProximityPhysicsComponent::UPlayerProximityPhysicsComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickInterval = 0.2f;
}

void UPlayerProximityPhysicsComponent::BeginPlay()
{
    Super::BeginPlay();

    RefreshManagedPhysicsComponents();
    ResolveVoxelLODVolume();
    PrimaryComponentTick.TickInterval = FMath::Max(ProximityCheckInterval, 0.02f);
    UpdateProximityState();
}

void UPlayerProximityPhysicsComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction
)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateProximityState();
}

void UPlayerProximityPhysicsComponent::RefreshManagedPhysicsComponents()
{
    AActor* Owner = GetOwner();
    if (!IsValid(Owner))
    {
        ManagedPhysicsComponents.Reset();
        ManagedSpiderMovementStates.Reset();
        ManagedSpiderCharacterMovementState = FSpiderCharacterMovementProximityState();
        ManagedSpiderMeshStates.Reset();
        return;
    }

    TSet<UPrimitiveComponent*> ExistingManagedComponents;
    for (UPrimitiveComponent* PrimitiveComponent : ManagedPhysicsComponents)
    {
        if (IsValid(PrimitiveComponent))
        {
            ExistingManagedComponents.Add(PrimitiveComponent);
        }
    }

    ManagedPhysicsComponents.Reset();

    TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
    Owner->GetComponents(PrimitiveComponents);

    for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
    {
        if (!IsValid(PrimitiveComponent))
        {
            continue;
        }

        if (ExistingManagedComponents.Contains(PrimitiveComponent) || PrimitiveComponent->IsSimulatingPhysics())
        {
            ManagedPhysicsComponents.Add(PrimitiveComponent);
        }
    }

    RefreshManagedSpiderMovementComponents();
}

void UPlayerProximityPhysicsComponent::UpdateProximityState()
{
    const bool bShouldEnablePhysics =
        !bUsePlayerProximityActivation ||
        ActivationRadius <= 0.0f ||
        IsAnyPlayerWithinRadius(ActivationRadius);

    bIsProximityActive = bShouldEnablePhysics;

    if (bShouldEnablePhysics != bIsPhysicsCurrentlyEnabled)
    {
        AActor* Owner = GetOwner();
        if (IsValid(Owner) && Owner->HasAuthority())
        {
            SetManagedPhysicsEnabled(bShouldEnablePhysics);
            SetManagedSpiderMovementEnabled(bShouldEnablePhysics);
            SetManagedSpiderCharacterMovementEnabled(bShouldEnablePhysics);
        }

        // Blueprint actor ticks and cosmetic animation are local work on every
        // instance. This component keeps ticking so it can reactivate them.
        SetManagedSpiderActorTickEnabled(bShouldEnablePhysics);
        SetManagedSpiderMeshAnimationEnabled(bShouldEnablePhysics);
        bIsPhysicsCurrentlyEnabled = bShouldEnablePhysics;
    }

    if (UVoxelLODVolumeComponent* VoxelLODVolume = ResolveVoxelLODVolume())
    {
        if (bSetVoxelLODVolumeRadiusToActivationRadius)
        {
            VoxelLODVolume->SphereRadius = FMath::Max(ActivationRadius, 0.0f);
        }

        VoxelLODVolume->bEnabled = bShouldEnablePhysics;
    }
}

void UPlayerProximityPhysicsComponent::SetManagedSpiderActorTickEnabled(bool bEnablePhysics)
{
    if (ManagedSpiderMovementStates.IsEmpty())
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!IsValid(Owner))
    {
        return;
    }

    if (bEnablePhysics)
    {
        Owner->SetActorTickEnabled(bRestoreOwnerActorTickEnabled);
        bRestoreOwnerActorTickEnabled = false;
        return;
    }

    bRestoreOwnerActorTickEnabled = Owner->IsActorTickEnabled();
    Owner->SetActorTickEnabled(false);
}

bool UPlayerProximityPhysicsComponent::IsAnyPlayerWithinRadius(float Radius) const
{
    const UWorld* World = GetWorld();
    if (!IsValid(World) || Radius <= 0.0f)
    {
        return false;
    }

    const float RadiusSquared = FMath::Square(Radius);
    const FVector Origin = GetComponentLocation();

    for (TActorIterator<APawn> It(World); It; ++It)
    {
        const APawn* Pawn = *It;
        if (!IsValid(Pawn) || !Pawn->IsPlayerControlled())
        {
            continue;
        }

        if (FVector::DistSquared(Pawn->GetActorLocation(), Origin) <= RadiusSquared)
        {
            return true;
        }
    }

    return false;
}

void UPlayerProximityPhysicsComponent::RefreshManagedSpiderMovementComponents()
{
    AActor* Owner = GetOwner();
    if (!IsValid(Owner))
    {
        ManagedSpiderMovementStates.Reset();
        ManagedSpiderCharacterMovementState = FSpiderCharacterMovementProximityState();
        ManagedSpiderMeshStates.Reset();
        return;
    }

    TArray<FSpiderMovementProximityState> ExistingStates = ManagedSpiderMovementStates;
    FSpiderCharacterMovementProximityState ExistingCharacterMovementState = ManagedSpiderCharacterMovementState;
    TArray<FSpiderMeshProximityState> ExistingMeshStates = ManagedSpiderMeshStates;
    ManagedSpiderMovementStates.Reset();
    ManagedSpiderCharacterMovementState = FSpiderCharacterMovementProximityState();
    ManagedSpiderMeshStates.Reset();

    TInlineComponentArray<USpiderPawnMovement*> SpiderMovementComponents;
    Owner->GetComponents(SpiderMovementComponents);

    for (USpiderPawnMovement* SpiderMovement : SpiderMovementComponents)
    {
        if (!IsValid(SpiderMovement))
        {
            continue;
        }

        FSpiderMovementProximityState* ExistingState =
            ExistingStates.FindByPredicate([SpiderMovement](const FSpiderMovementProximityState& State)
                {
                    return State.SpiderMovement == SpiderMovement;
                });

        FSpiderMovementProximityState& NewState = ManagedSpiderMovementStates.AddDefaulted_GetRef();
        NewState.SpiderMovement = SpiderMovement;

        if (ExistingState != nullptr)
        {
            NewState.bRestoreMovementPaused = ExistingState->bRestoreMovementPaused;
            NewState.bRestoreTickEnabled = ExistingState->bRestoreTickEnabled;
        }
    }

    if (ManagedSpiderMovementStates.IsEmpty())
    {
        return;
    }

    if (UCharacterMovementComponent* CharacterMovement = Owner->FindComponentByClass<UCharacterMovementComponent>())
    {
        ManagedSpiderCharacterMovementState.CharacterMovement = CharacterMovement;

        if (ExistingCharacterMovementState.CharacterMovement == CharacterMovement)
        {
            ManagedSpiderCharacterMovementState.RestoreMovementMode = ExistingCharacterMovementState.RestoreMovementMode;
            ManagedSpiderCharacterMovementState.RestoreCustomMovementMode = ExistingCharacterMovementState.RestoreCustomMovementMode;
            ManagedSpiderCharacterMovementState.bRestoreTickEnabled = ExistingCharacterMovementState.bRestoreTickEnabled;
        }
    }

    TInlineComponentArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    Owner->GetComponents(SkeletalMeshComponents);

    for (USkeletalMeshComponent* SkeletalMesh : SkeletalMeshComponents)
    {
        if (!IsValid(SkeletalMesh))
        {
            continue;
        }

        FSpiderMeshProximityState* ExistingMeshState =
            ExistingMeshStates.FindByPredicate([SkeletalMesh](const FSpiderMeshProximityState& State)
                {
                    return State.SkeletalMesh == SkeletalMesh;
                });

        FSpiderMeshProximityState& NewMeshState = ManagedSpiderMeshStates.AddDefaulted_GetRef();
        NewMeshState.SkeletalMesh = SkeletalMesh;

        if (ExistingMeshState != nullptr)
        {
            NewMeshState.bRestorePauseAnims = ExistingMeshState->bRestorePauseAnims;
            NewMeshState.bRestoreTickEnabled = ExistingMeshState->bRestoreTickEnabled;
            NewMeshState.RestoreGlobalAnimRateScale = ExistingMeshState->RestoreGlobalAnimRateScale;
        }
    }
}

void UPlayerProximityPhysicsComponent::SetManagedPhysicsEnabled(bool bEnablePhysics)
{
    for (int32 Index = ManagedPhysicsComponents.Num() - 1; Index >= 0; --Index)
    {
        UPrimitiveComponent* PrimitiveComponent = ManagedPhysicsComponents[Index];
        if (!IsValid(PrimitiveComponent))
        {
            ManagedPhysicsComponents.RemoveAt(Index);
            continue;
        }

        if (bEnablePhysics)
        {
            if (!PrimitiveComponent->IsSimulatingPhysics())
            {
                PrimitiveComponent->SetSimulatePhysics(true);
            }

            PrimitiveComponent->WakeAllRigidBodies();
        }
        else if (PrimitiveComponent->IsSimulatingPhysics())
        {
            PrimitiveComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false, NAME_None);
            PrimitiveComponent->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector, false, NAME_None);
            PrimitiveComponent->SetSimulatePhysics(false);
        }
    }

    bIsPhysicsCurrentlyEnabled = bEnablePhysics;
}

void UPlayerProximityPhysicsComponent::SetManagedSpiderMovementEnabled(bool bEnablePhysics)
{
    for (int32 Index = ManagedSpiderMovementStates.Num() - 1; Index >= 0; --Index)
    {
        FSpiderMovementProximityState& State = ManagedSpiderMovementStates[Index];
        USpiderPawnMovement* SpiderMovement = State.SpiderMovement.Get();
        if (!IsValid(SpiderMovement))
        {
            ManagedSpiderMovementStates.RemoveAt(Index);
            continue;
        }

        if (bEnablePhysics)
        {
            SpiderMovement->SetMovementPaused(State.bRestoreMovementPaused);
            SpiderMovement->SetComponentTickEnabled(State.bRestoreTickEnabled);

            State.bRestoreMovementPaused = false;
            State.bRestoreTickEnabled = false;
            continue;
        }

        State.bRestoreMovementPaused = SpiderMovement->IsMovementPaused();
        State.bRestoreTickEnabled = SpiderMovement->IsComponentTickEnabled();

        SpiderMovement->SetMovementPaused(true);
        SpiderMovement->SetComponentTickEnabled(false);
    }
}

void UPlayerProximityPhysicsComponent::SetManagedSpiderCharacterMovementEnabled(bool bEnablePhysics)
{
    UCharacterMovementComponent* CharacterMovement = ManagedSpiderCharacterMovementState.CharacterMovement.Get();
    if (!IsValid(CharacterMovement))
    {
        ManagedSpiderCharacterMovementState = FSpiderCharacterMovementProximityState();
        return;
    }

    if (bEnablePhysics)
    {
        CharacterMovement->SetComponentTickEnabled(ManagedSpiderCharacterMovementState.bRestoreTickEnabled);
        CharacterMovement->SetMovementMode(
            ManagedSpiderCharacterMovementState.RestoreMovementMode,
            ManagedSpiderCharacterMovementState.RestoreCustomMovementMode
        );

        ManagedSpiderCharacterMovementState.RestoreMovementMode = MOVE_None;
        ManagedSpiderCharacterMovementState.RestoreCustomMovementMode = 0;
        ManagedSpiderCharacterMovementState.bRestoreTickEnabled = false;
        return;
    }

    ManagedSpiderCharacterMovementState.RestoreMovementMode = CharacterMovement->MovementMode;
    ManagedSpiderCharacterMovementState.RestoreCustomMovementMode = CharacterMovement->CustomMovementMode;
    ManagedSpiderCharacterMovementState.bRestoreTickEnabled = CharacterMovement->IsComponentTickEnabled();

    CharacterMovement->StopMovementImmediately();
    CharacterMovement->StopActiveMovement();
    CharacterMovement->DisableMovement();
    CharacterMovement->SetComponentTickEnabled(false);
}

void UPlayerProximityPhysicsComponent::SetManagedSpiderMeshAnimationEnabled(bool bEnablePhysics)
{
    for (int32 Index = ManagedSpiderMeshStates.Num() - 1; Index >= 0; --Index)
    {
        FSpiderMeshProximityState& State = ManagedSpiderMeshStates[Index];
        USkeletalMeshComponent* SkeletalMesh = State.SkeletalMesh.Get();
        if (!IsValid(SkeletalMesh))
        {
            ManagedSpiderMeshStates.RemoveAt(Index);
            continue;
        }

        if (bEnablePhysics)
        {
            SkeletalMesh->bPauseAnims = State.bRestorePauseAnims;
            SkeletalMesh->GlobalAnimRateScale = State.RestoreGlobalAnimRateScale;
            SkeletalMesh->SetComponentTickEnabled(State.bRestoreTickEnabled);
            continue;
        }

        State.bRestorePauseAnims = SkeletalMesh->bPauseAnims;
        State.bRestoreTickEnabled = SkeletalMesh->IsComponentTickEnabled();
        State.RestoreGlobalAnimRateScale = SkeletalMesh->GlobalAnimRateScale;

        if (bKeepMeshAnimationActiveWhenInactive)
        {
            // Preserve animation evaluation for rigs that need it, while keeping physics and movement disabled.
            continue;
        }

        SkeletalMesh->bPauseAnims = true;
        SkeletalMesh->GlobalAnimRateScale = 0.0f;
        SkeletalMesh->SetComponentTickEnabled(false);
    }
}

UVoxelLODVolumeComponent* UPlayerProximityPhysicsComponent::ResolveVoxelLODVolume()
{
    AActor* Owner = GetOwner();

    if (!IsValid(Owner))
    {
        CachedVoxelLODVolume = nullptr;
        return nullptr;
    }

    if (IsValid(CachedVoxelLODVolume) && CachedVoxelLODVolume->GetOwner() == Owner)
    {
        return CachedVoxelLODVolume;
    }

    CachedVoxelLODVolume = Owner->FindComponentByClass<UVoxelLODVolumeComponent>();
    return CachedVoxelLODVolume;
}