#include "SpiderPawnMovement.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

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
	GravityDir = FVector::DownVector;
	TargetGravityDir = FVector::DownVector;
	MovementVelocity = FVector::ZeroVector;
	DetachTimer = 0.0f;
	bHasSurfaceNormal = false;
}

bool USpiderPawnMovement::ProbeForAttachedSurface(FHitResult& OutHit) const
{
	if (!UpdatedPrimitive || !GetWorld() || SurfaceProbeDistance <= 0.0f)
	{
		return false;
	}

	const FVector ProbeDirection = TargetGravityDir.GetSafeNormal();
	if (ProbeDirection.IsNearlyZero())
	{
		return false;
	}

	const FVector ProbeStart = UpdatedPrimitive->GetComponentLocation();
	const FVector ProbeEnd = ProbeStart + ProbeDirection * SurfaceProbeDistance;
	FComponentQueryParams QueryParams(SCENE_QUERY_STAT(SpiderSurfaceProbe), PawnOwner);
	QueryParams.bIgnoreTouches = true;

	TArray<FHitResult> Hits;
	if (!GetWorld()->ComponentSweepMulti(
		Hits,
		UpdatedPrimitive,
		ProbeStart,
		ProbeEnd,
		UpdatedPrimitive->GetComponentQuat(),
		QueryParams))
	{
		return false;
	}

	bool bFoundSurface = false;
	float BestAlignment = 0.25f;
	for (const FHitResult& Hit : Hits)
	{
		if (!Hit.bBlockingHit)
		{
			continue;
		}

		const FVector HitNormal = Hit.ImpactNormal.IsNearlyZero() ? Hit.Normal : Hit.ImpactNormal;
		const float Alignment = FVector::DotProduct(HitNormal.GetSafeNormal(), -ProbeDirection);
		if (Alignment > BestAlignment)
		{
			BestAlignment = Alignment;
			OutHit = Hit;
			bFoundSurface = true;
		}
	}

	return bFoundSurface;
}

void USpiderPawnMovement::TickClient(float DeltaTime)
{
	ConsumeInputVector();

	// Just keep gravity smoothing visually - velocity comes from replication
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

	bool bHadSurfaceContact = false;
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
					CurrentSurfaceNormal = Hit.ImpactNormal.IsNearlyZero() ? Hit.Normal : Hit.ImpactNormal;
					TargetGravityDir = -CurrentSurfaceNormal;
					MovementVelocity = FVector::ZeroVector;
					DetachTimer = 0.0f;
					bHadSurfaceContact = true;
				}

				SlideAlongSurface(MoveDelta, 1.f - Hit.Time, Hit.Normal, Hit);
			}
			else
			{

				if (!bHasSurfaceNormal)
				{
					CurrentSurfaceNormal = Hit.ImpactNormal.IsNearlyZero() ? Hit.Normal : Hit.ImpactNormal;
					TargetGravityDir = -CurrentSurfaceNormal;
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
				bHadSurfaceContact = true;
				SlideAlongSurface(MoveDelta, 1.f - Hit.Time, Hit.Normal, Hit);


			}
		}
	}

	if (!bIsDropping && !bHadSurfaceContact && bisGrounded)
	{
		FHitResult ProbeHit;
		if (ProbeForAttachedSurface(ProbeHit))
		{
			if (!bHasSurfaceNormal)
			{
				CurrentSurfaceNormal = ProbeHit.ImpactNormal.IsNearlyZero() ? ProbeHit.Normal : ProbeHit.ImpactNormal;
				CurrentSurfaceNormal.Normalize();
				TargetGravityDir = -CurrentSurfaceNormal;
			}

			MovementVelocity = FVector::ZeroVector;
			DetachTimer = 0.0f;
			bHadSurfaceContact = true;
		}
	}

	if (!bIsDropping && !bHadSurfaceContact && bisGrounded)
	{
		DetachTimer += DeltaTime;
		if (DetachTimer >= DetachThreshold)
		{
			bisGrounded = false;
			TargetGravityDir = FVector::DownVector;
			MovementVelocity = FVector::ZeroVector;
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

