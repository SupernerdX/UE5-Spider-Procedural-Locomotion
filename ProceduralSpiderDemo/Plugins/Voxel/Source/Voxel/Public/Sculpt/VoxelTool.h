// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelTool.generated.h"

struct FVoxelConfig;

enum class EVoxelProjectionShape : uint8
{
	Circle,
	Square
};

UCLASS(BlueprintType, Abstract)
class VOXEL_API UVoxelTool : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Config", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (UIMin = 0, UIMax = 1))
	float AlignToMovementSmoothness = 0.75;

public:
	virtual ~UVoxelTool() override;

	virtual void Enter() {}
	virtual void Exit() {}
	virtual void Tick() {}
	virtual void OnEditBegin() {}
	virtual void OnEditEnd() {}
	virtual bool PrepareModifierData() { return true; }

public:
	UWorld* GetWorld() const
	{
		return WeakWorld.Resolve();
	}
	FVector GetHitLocation() const
	{
		return HitLocation;
	}
	FVector GetHitNormal() const
	{
		return HitNormal;
	}
	FVector GetTraceDirection() const
	{
		return TraceDirection;
	}
	FVector GetStrokeDirection() const
	{
		return StrokeDirection;
	}
	FRay GetLastRay() const
	{
		return LastRay;
	}
	bool InverseAction() const
	{
		return bInverse;
	}
	bool HasValidHit() const
	{
		return bHasValidHit;
	}

public:
	bool Initialized() const { return bIsActive; }
	void CallEnter(UWorld& NewWorld);
	void CallExit(bool bForce);
	void CallTick(const FRay& Ray, bool bInverseAction);
#if WITH_EDITOR
	void SetHiddenByMouseCapture(bool bHide);
	void RefreshToolVisibility();
#endif

protected:
	AActor* SpawnActor(UClass* Class);

	template<typename T>
	T* SpawnActor()
	{
		return CastEnsured<T>(this->SpawnActor(StaticClassFast<T>()));
	}

protected:
	static int32 GenerateRays(
		const FVector& Position,
		const FVector& Direction,
		float Radius,
		EVoxelProjectionShape Shape,
		float NumRays,
		float MaxDistance,
		TFunctionRef<void(const FVector& Start, const FVector& End, const FVector2D& PlanePosition)> Lambda);

	static void FindProjectionAverage(
		const UWorld* World,
		const FVector& Position,
		const FVector& Direction,
		float Radius,
		EVoxelProjectionShape Shape,
		float NumRays,
		float MaxDistance,
		FVector& OutPosition,
		FVector& OutNormal);

protected:
	FVoxelOptionalIntBox InvalidatedBounds;

private:
	bool bIsActive = false;
	TVoxelArray<TVoxelObjectPtr<AActor>> Actors;

	TVoxelObjectPtr<UWorld> WeakWorld;

	FVector HitLocation = FVector(ForceInit);
	FVector HitNormal = FVector::UpVector;

	FVector TraceDirection = FVector::DownVector;
	FVector StrokeDirection = FVector::ForwardVector;

	FRay LastRay;

	bool bInverse = false;

	FHitResult LastHitResult;
	bool bHasValidHit = false;

#if WITH_EDITOR
	bool bHiddenByMouseCapture = false;
#endif
};