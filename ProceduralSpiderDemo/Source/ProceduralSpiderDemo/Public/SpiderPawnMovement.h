// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "SpiderEnums.h"
#include "SpiderPawnMovement.generated.h"

/**
 * 
 */


UCLASS(meta = (BlueprintSpawnableComponent))
class PROCEDURALSPIDERDEMO_API USpiderPawnMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	USpiderPawnMovement();

	virtual void TickComponent(float DeltaTime, enum ELevelTick tickType, FActorComponentTickFunction *thisTickFunction) override;
	
	virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Spider Movement")
	void SetMovementPaused(bool bPaused);

	UFUNCTION(BlueprintPure, Category = "Spider Movement")
	bool IsMovementPaused() const { return bMovementPaused; }

	UFUNCTION(BlueprintCallable, Category = "Spider Movement")
	void SetSurfaceNormal(FVector NewNormal);

	UFUNCTION(BlueprintCallable, Category = "Spider Movement")
	void SetMoveDirection(FVector WorldDirection);

	UFUNCTION(BlueprintCallable, Category = "Spider Movement")
	void StartDrop();

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Spider Movement")
	FVector CurrentSurfaceNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Spider Movement")
	FVector GravityDir = FVector::DownVector;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Spider Movement")
	ESpiderSurfaceState SurfaceState = ESpiderSurfaceState::Grounded; 

	// ─── Tweakable in Blueprint/Editor ──────────────────────────────────────

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Spider Movement")
	FVector MovementVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Spider Movement")
	FVector AIFacingDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Spider Movement")
	bool bUseAIRotation = false;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Spider Movement")
	bool bIsDropping = false;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Spider Movement")
	bool bisGrounded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider Movement")
	float GravityTransitionSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider Movement")
	float StickyForce = 980.0f;

	// How long before world gravity kicks back in after losing surface contact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider Movement")
	float DetachThreshold = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Spider Movement")
	float MaxMovementSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spider Movement")
	float GravityAccel = 980.0f;


private: 
	FVector TargetGravityDir = FVector::DownVector; 
	FVector LastSurfaceNormal = FVector::UpVector; 
	FQuat CurrentCapsuleRotation = FQuat::Identity;
	
	float DetachTimer = 0.0f;

	bool bHasSurfaceNormal = false;
	bool bMovementPaused = false;
	void TickServer(float DeltaTime);
	void TickClient(float DeltaTime);
	
	




	
};
