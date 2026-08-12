// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/VoxelTool.h"
#include "Collision/VoxelCollisionChannels.h"
#include "Landscape.h"
#include "Engine/StaticMeshActor.h"
#include "Framework/Application/SlateApplication.h"

UVoxelTool::~UVoxelTool()
{
	ensure(!bIsActive);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelTool::CallEnter(UWorld& NewWorld)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(!bIsActive);
	bIsActive = true;

	WeakWorld = &NewWorld;

	Enter();
}

void UVoxelTool::CallExit(const bool bForce)
{
	VOXEL_FUNCTION_COUNTER();

	Exit();

	ensure(bIsActive || bForce);
	bIsActive = false;

	for (const TVoxelObjectPtr<AActor>& WeakActor : Actors)
	{
		if (AActor* Actor = WeakActor.Resolve())
		{
			Actor->Destroy();
		}
	}
	Actors.Empty();
}

void UVoxelTool::CallTick(const FRay& Ray, const bool bInverseAction)
{
	VOXEL_FUNCTION_COUNTER();

	UWorld* World = WeakWorld.Resolve();
	if (!ensure(bIsActive) ||
		!ensure(World))
	{
		return;
	}

	for (const TVoxelObjectPtr<AActor>& WeakActor : Actors)
	{
		if (!ensure(WeakActor.IsValid_Slow()))
		{
			return;
		}
	}

	FCollisionQueryParams QueryParams;
	for (const ALandscape* Actor : TActorRange<ALandscape>(World))
	{
		QueryParams.AddIgnoredActor(Actor);
	}

	FHitResult HitResult;

	const bool bValidHit = World->LineTraceSingleByObjectType(
		HitResult,
		Ray.Origin,
		Ray.Origin + Ray.Direction * 1e6,
		!World->IsGameWorld() ? FCollisionObjectQueryParams(ECC_VoxelEditor) : FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects),
		QueryParams);

	if (bValidHit)
	{
		if (LastHitResult.IsValidBlockingHit())
		{
			const FVector NewStrokeDirection = (HitResult.ImpactPoint - LastHitResult.ImpactPoint).GetSafeNormal();

			StrokeDirection =
				FMath::Lerp(
					StrokeDirection,
					NewStrokeDirection,
					FMath::Clamp((1.f - AlignToMovementSmoothness) * 0.1f, 0.f, 1.f)).GetSafeNormal();
		}

		LastHitResult = HitResult;

		HitLocation = HitResult.Location;
		HitNormal = HitResult.ImpactNormal;

		TraceDirection = Ray.Direction;
	}

	LastRay = Ray;

	bInverse = bInverseAction;

	Tick();

	if (bValidHit != bHasValidHit)
	{
		bHasValidHit = bValidHit;
#if WITH_EDITOR
		RefreshToolVisibility();
#endif
	}
}

#if WITH_EDITOR
void UVoxelTool::SetHiddenByMouseCapture(const bool bHide)
{
	bHiddenByMouseCapture = bHide;
	RefreshToolVisibility();
}

void UVoxelTool::RefreshToolVisibility()
{
	const bool bHidden = !bHasValidHit || bHiddenByMouseCapture;

	for (const TVoxelObjectPtr<AActor>& WeakActor : Actors)
	{
		if (AActor* Actor = WeakActor.Resolve())
		{
			Actor->SetIsTemporarilyHiddenInEditor(bHidden);
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AActor* UVoxelTool::SpawnActor(UClass* Class)
{
	VOXEL_FUNCTION_COUNTER();

	UWorld* World = WeakWorld.Resolve();
	if (!ensure(World))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.ObjectFlags |= RF_Transient;
#if WITH_EDITOR
	if (!World->IsGameWorld())
	{
		SpawnParameters.bTemporaryEditorActor = true;
		SpawnParameters.bHideFromSceneOutliner = true;
	}
#endif

	AActor* Actor = World->SpawnActor<AActor>(Class, SpawnParameters);
	if (!ensure(Actor))
	{
		return nullptr;
	}

	if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
	{
		StaticMeshActor->SetMobility(EComponentMobility::Movable);
	}

	Actors.Add(Actor);
	return Actor;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 UVoxelTool::GenerateRays(
	const FVector& Position,
	const FVector& Direction,
	const float Radius,
	const EVoxelProjectionShape Shape,
	const float NumRays,
	const float MaxDistance,
	TFunctionRef<void(const FVector& Start, const FVector& End, const FVector2D& PlanePosition)> Lambda)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(Direction.IsNormalized()) ||
		NumRays <= 0)
	{
		return 0;
	}

	const auto GetTangent = [](const FVector& N)
	{
		// Compute tangent
		// N dot T = 0
		// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
		// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y) if N.Z != 0
		if (N.Z != 0)
		{
			return FVector(1.f, 1.f, -1.f / double(N.Z) * (N.X + N.Y)).GetSafeNormal();
		}
		else
		{
			return FVector::UpVector;
		}
	};

	const FVector Tangent = GetTangent(Direction);
	const FVector BiTangent = FVector::CrossProduct(Tangent, Direction).GetSafeNormal();
	// NumRays is the area; get the radius we would get from such area
	float NumRaysInRadius = 0.f;
	switch (Shape)
	{
	case EVoxelProjectionShape::Circle: NumRaysInRadius = FMath::Sqrt(NumRays / PI); break;
	case EVoxelProjectionShape::Square: NumRaysInRadius = FMath::Sqrt(NumRays) / 2; break;
	}

	const int32 Count = FMath::CeilToInt(NumRaysInRadius);
	const float RadiusSquared = FMath::Square(Radius);

	int32 NumRaysActuallyTraced = 0;
	for (int32 U = -Count; U <= Count; U++)
	{
		for (int32 V = -Count; V <= Count; V++)
		{
			const FVector2D PlanePosition = FVector2D(U, V) * Radius / Count;
			if (Shape == EVoxelProjectionShape::Circle &&
				PlanePosition.SizeSquared() >= RadiusSquared)
			{
				continue;
			}

			const FVector Start = Position + (Tangent * PlanePosition.X + BiTangent * PlanePosition.Y);
			const FVector End = Start + Direction * MaxDistance;
			Lambda(Start, End, PlanePosition);
			NumRaysActuallyTraced++;
		}
	}
	return NumRaysActuallyTraced;
}

void UVoxelTool::FindProjectionAverage(
	const UWorld* World,
	const FVector& Position,
	const FVector& Direction,
	const float Radius,
	const EVoxelProjectionShape Shape,
	const float NumRays,
	const float MaxDistance,
	FVector& OutPosition,
	FVector& OutNormal)
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(World))
	{
		return;
	}

	TArray<FHitResult> Hits;

	GenerateRays(Position, Direction, Radius, Shape, NumRays, MaxDistance, [&](const FVector& Start, const FVector& End, const FVector2D& PlanePosition)
	{
		VOXEL_SCOPE_COUNTER("Linetrace");

		FHitResult OutHit;
		if (World->LineTraceSingleByObjectType(
			OutHit,
			Start,
			End,
			!World->IsGameWorld() ? FCollisionObjectQueryParams(ECC_VoxelEditor) : FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects)))
		{
			Hits.Add(OutHit);
		}
	});

	if (Hits.Num() == 0)
	{
		return;
	}

	FVector PositionsSum = Hits[0].ImpactPoint;
	FVector NormalsSum = Hits[0].ImpactNormal;
	for (int32 Index = 1; Index < Hits.Num(); Index++)
	{
		PositionsSum += Hits[Index].ImpactPoint;
		NormalsSum += Hits[Index].ImpactNormal;
	}

	if (ensure(!FMath::IsNaN(PositionsSum.X + PositionsSum.Y + PositionsSum.Z)))
	{
		OutPosition = PositionsSum / Hits.Num();
	}

	if (ensure(!FMath::IsNaN(NormalsSum.X + NormalsSum.Y + NormalsSum.Z)))
	{
		OutNormal = NormalsSum.GetSafeNormal();
	}
}