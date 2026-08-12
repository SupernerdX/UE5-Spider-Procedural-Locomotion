#include "SpiderPawnMovement.h"

USpiderPawnMovement::USpiderPawnMovement()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void USpiderPawnMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpiderPawnMovement, CurrentSurfaceNormal);
	DOREPLIFETIME(USpiderPawnMovement, GravityDir);
	DOREPLIFETIME(USpiderPawnMovement, SurfaceState);
	DOREPLIFETIME(USpiderPawnMovement, AIFacingDirection);
	DOREPLIFETIME(USpiderPawnMovement, MaxMovementSpeed);
	DOREPLIFETIME(USpiderPawnMovement, MovementVelocity);
	DOREPLIFETIME(USpiderPawnMovement, bUseAIRotation);
	DOREPLIFETIME(USpiderPawnMovement, bIsDropping);
	DOREPLIFETIME(USpiderPawnMovement, bisGrounded);
}

void USpiderPawnMovement::SetMovementPaused(bool bPaused)
{
	if (bMovementPaused == bPaused)
	{
		return;
	}

	bMovementPaused = bPaused;

	if (bMovementPaused)
	{
		StopMovementImmediately();
		ConsumeInputVector();
		MovementVelocity = FVector::ZeroVector;
		AIFacingDirection = FVector::ZeroVector;
		bUseAIRotation = false;
	}
}

void USpiderPawnMovement::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	if (!PawnOwner || !PawnOwner->HasAuthority() || bMovementPaused || MoveVelocity.SizeSquared() < KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FVector MoveDir = MoveVelocity.GetSafeNormal();
	AddInputVector(MoveDir);
}

void USpiderPawnMovement::SetSurfaceNormal(FVector NewNormal)
{
	if (!PawnOwner || !PawnOwner->HasAuthority() || bMovementPaused || bIsDropping)
	{
		return;
	}

	if (!NewNormal.IsNearlyZero())
	{
		CurrentSurfaceNormal = NewNormal.GetSafeNormal();
		TargetGravityDir = -CurrentSurfaceNormal;
		bHasSurfaceNormal = true;
	}
}
void USpiderPawnMovement::SetMoveDirection(FVector WorldDirection)
{
	if (!PawnOwner || !PawnOwner->HasAuthority() || bMovementPaused)
	{
		return;
	}

	FVector SurfaceDirection = FVector::VectorPlaneProject(WorldDirection, CurrentSurfaceNormal);
	if (!SurfaceDirection.IsNearlyZero())
	{
		SurfaceDirection.Normalize();
		AddInputVector(SurfaceDirection);
		AIFacingDirection = SurfaceDirection;
		bUseAIRotation = true;
	}
}
void USpiderPawnMovement::StartDrop()
{
	if (!PawnOwner || !PawnOwner->HasAuthority() || bMovementPaused)
	{
		return;
	}

	bIsDropping = true;
	bisGrounded = false;
	StickyForce = 0.0;
	GravityDir = FVector::DownVector;
	MovementVelocity = FVector::ZeroVector;

}
void USpiderPawnMovement::TickClient(float DeltaTime)
{
	ConsumeInputVector();

	// Just keep gravity smoothing visually — velocity comes from replication
	GravityDir = FMath::VInterpTo(GravityDir, TargetGravityDir, DeltaTime, GravityTransitionSpeed);
}

void USpiderPawnMovement::TickServer(float DeltaTime)
{
	if (bMovementPaused)
	{
		ConsumeInputVector();
		return;
	}


	GravityDir = FMath::VInterpTo(GravityDir, TargetGravityDir, DeltaTime, GravityTransitionSpeed);

	float UpDot = FVector::DotProduct(CurrentSurfaceNormal, FVector::UpVector);
	if (UpDot > 0.7f)
		SurfaceState = ESpiderSurfaceState::Grounded;
	else if (UpDot < -0.3f)
		SurfaceState = ESpiderSurfaceState::Ceiling;
	else
		SurfaceState = ESpiderSurfaceState::Climbing;




	if (!bisGrounded)
	{
		MovementVelocity += GravityDir * GravityAccel * DeltaTime;

		DetachTimer += DeltaTime;

		if (DetachTimer > DetachThreshold)
		{
			TargetGravityDir = FVector::DownVector;
		}
	}
	else
	{
		MovementVelocity += TargetGravityDir * StickyForce * DeltaTime;
	}

	const FVector  InputDir = ConsumeInputVector().GetClampedToMaxSize(1.0f);
	FVector MoveDelta = (InputDir * MaxMovementSpeed + MovementVelocity) * DeltaTime;

	FVector ForwardDir = FVector::ZeroVector;

	if (APawn* Pawn = Cast<APawn>(PawnOwner))
	{
		if (bUseAIRotation)
		{
			ForwardDir = AIFacingDirection;
			bUseAIRotation = false;
		}

		else if (AController* Controller = Pawn->GetController())
		{
			FRotator ControlRot = Controller->GetControlRotation();
			ForwardDir = FVector::VectorPlaneProject(ControlRot.Vector(), CurrentSurfaceNormal);
		}
		if (!ForwardDir.IsNearlyZero())
		{
			ForwardDir.Normalize();

			FRotator TargetRot = FRotationMatrix::MakeFromXZ(ForwardDir, CurrentSurfaceNormal).Rotator();
			CurrentCapsuleRotation = FQuat::Slerp(CurrentCapsuleRotation, TargetRot.Quaternion(), DeltaTime * GravityTransitionSpeed);
			CurrentCapsuleRotation.Normalize();
		}


	}

	PawnOwner->SetActorRotation(CurrentCapsuleRotation.Rotator());

	if (!MoveDelta.IsNearlyZero())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(MoveDelta, CurrentCapsuleRotation.Rotator(), true, Hit);

		if (Hit.IsValidBlockingHit())
		{

			if (bIsDropping)
			{
				float HitDot = FVector::DotProduct(Hit.Normal, FVector::UpVector);
				if (HitDot > 0.3f)
				{
					bIsDropping = false;
					bisGrounded = true;
					CurrentSurfaceNormal = Hit.Normal;
					TargetGravityDir = -Hit.Normal;
					MovementVelocity = FVector::ZeroVector;
				}

				SlideAlongSurface(MoveDelta, 1.f - Hit.Time, Hit.Normal, Hit);
			}
			else
			{

				if (!bHasSurfaceNormal)
				{
					CurrentSurfaceNormal = Hit.Normal;
					TargetGravityDir = -Hit.Normal;
				}

				MovementVelocity = FVector::ZeroVector;
				float SurfaceChange = FVector::DotProduct(Hit.Normal, LastSurfaceNormal);
				if (SurfaceChange < 0.95f)
				{
					MovementVelocity = FVector::ZeroVector;
				}
				LastSurfaceNormal = Hit.Normal;

				bisGrounded = true;
				DetachTimer = 0.0f;
				SlideAlongSurface(MoveDelta, 1.f - Hit.Time, Hit.Normal, Hit);


			}
		}
		else
		{
			if (!bIsDropping)
				bisGrounded = false;
		}
	}
	bHasSurfaceNormal = false;
}

void USpiderPawnMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	if (PawnOwner->HasAuthority())
	{
		TickServer(DeltaTime);
	}
	else
	{
		TickClient(DeltaTime);
	}
}

