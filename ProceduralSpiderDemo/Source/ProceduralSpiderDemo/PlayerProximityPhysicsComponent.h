#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "PlayerProximityPhysicsComponent.generated.h"

class UPrimitiveComponent;
class USkeletalMeshComponent;
class USpiderPawnMovement;
class UVoxelLODVolumeComponent;

USTRUCT()
struct FSpiderMovementProximityState
{
    GENERATED_BODY()

    UPROPERTY(Transient)
    TObjectPtr<USpiderPawnMovement> SpiderMovement = nullptr;

    UPROPERTY(Transient)
    bool bRestoreMovementPaused = false;

    UPROPERTY(Transient)
    bool bRestoreTickEnabled = false;
};

USTRUCT()
struct FSpiderCharacterMovementProximityState
{
    GENERATED_BODY()

    UPROPERTY(Transient)
    TObjectPtr<UCharacterMovementComponent> CharacterMovement = nullptr;

    UPROPERTY(Transient)
    TEnumAsByte<EMovementMode> RestoreMovementMode = MOVE_None;

    UPROPERTY(Transient)
    uint8 RestoreCustomMovementMode = 0;

    UPROPERTY(Transient)
    bool bRestoreTickEnabled = false;
};

USTRUCT()
struct FSpiderMeshProximityState
{
    GENERATED_BODY()

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh = nullptr;

    UPROPERTY(Transient)
    bool bRestorePauseAnims = false;

    UPROPERTY(Transient)
    bool bRestoreTickEnabled = false;

    UPROPERTY(Transient)
    float RestoreGlobalAnimRateScale = 1.0f;
};

UCLASS(ClassGroup = (OnTheNose), meta = (BlueprintSpawnableComponent))
class PROCEDURALSPIDERDEMO_API UPlayerProximityPhysicsComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UPlayerProximityPhysicsComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction
    ) override;

    UFUNCTION(BlueprintCallable, Category = "Player Proximity")
    void RefreshManagedPhysicsComponents();

    UFUNCTION(BlueprintCallable, Category = "Player Proximity")
    void UpdateProximityState();

    UFUNCTION(BlueprintPure, Category = "Player Proximity")
    bool IsProximityActive() const { return bIsProximityActive; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Proximity")
    bool bUsePlayerProximityActivation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Proximity")
    bool bKeepMeshAnimationActiveWhenInactive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Proximity", meta = (ClampMin = "0.0"))
    float ActivationRadius = 3000.0f;

    /** Proximity does not need to be evaluated every frame. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Proximity", meta = (ClampMin = "0.02"))
    float ProximityCheckInterval = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Proximity")
    bool bSetVoxelLODVolumeRadiusToActivationRadius = true;

private:
    bool IsAnyPlayerWithinRadius(float Radius) const;
    void RefreshManagedSpiderMovementComponents();
    void SetManagedPhysicsEnabled(bool bEnablePhysics);
    void SetManagedSpiderActorTickEnabled(bool bEnablePhysics);
    void SetManagedSpiderMovementEnabled(bool bEnablePhysics);
    void SetManagedSpiderCharacterMovementEnabled(bool bEnablePhysics);
    void SetManagedSpiderMeshAnimationEnabled(bool bEnablePhysics);
    UVoxelLODVolumeComponent* ResolveVoxelLODVolume();

    UPROPERTY(Transient)
    TArray<TObjectPtr<UPrimitiveComponent>> ManagedPhysicsComponents;

    UPROPERTY(Transient)
    TArray<FSpiderMovementProximityState> ManagedSpiderMovementStates;

    UPROPERTY(Transient)
    FSpiderCharacterMovementProximityState ManagedSpiderCharacterMovementState;

    UPROPERTY(Transient)
    TArray<FSpiderMeshProximityState> ManagedSpiderMeshStates;

    UPROPERTY(Transient)
    TObjectPtr<UVoxelLODVolumeComponent> CachedVoxelLODVolume = nullptr;

    UPROPERTY(Transient)
    bool bIsPhysicsCurrentlyEnabled = true;

    UPROPERTY(Transient)
    bool bRestoreOwnerActorTickEnabled = false;

    UPROPERTY(Transient)
    bool bIsProximityActive = true;
};
