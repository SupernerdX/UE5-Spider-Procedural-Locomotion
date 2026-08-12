// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/Tools/VoxelFlattenHeightTool.h"
#include "Sculpt/Height/Tools/VoxelFlattenHeightModifier.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelFlattenHeightTool::Enter()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Enter();

	PreviewActor = SpawnActor<AStaticMeshActor>();
	if (!ensure(PreviewActor))
	{
		return;
	}

	PreviewActor->SetActorEnableCollision(false);

	UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	ensure(StaticMesh);
	PreviewActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	PreviewActor->GetStaticMeshComponent()->SetMaterial(0, PreviewMaterial);
}

void UVoxelFlattenHeightTool::Exit()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Exit();
}

void UVoxelFlattenHeightTool::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	check(PreviewActor);

	PreviewActor->SetActorLocation(GetHitLocation());
	PreviewActor->SetActorScale3D(FVector(Radius / 50.f, Radius / 50.f, 1000.f));

	UMaterialInstanceDynamic* Material = PreviewMaterial;
	Brush.UpdateMaterial(Material, Radius, GetHitNormal(), GetStrokeDirection(), true);
	if (PreviewMaterial != Material)
	{
		PreviewMaterial = Material;
		PreviewActor->GetStaticMeshComponent()->SetMaterial(0, PreviewMaterial);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelFlattenHeightTool::OnEditBegin()
{
	FirstHeight.Reset();
}

void UVoxelFlattenHeightTool::OnEditEnd()
{
	FirstHeight.Reset();
}

bool UVoxelFlattenHeightTool::PrepareModifierData()
{
	FVector Position = GetHitLocation();
	FVector Normal = GetHitNormal();

	if (bUseAverage)
	{
		const FVector Direction = GetTraceDirection();

		FindProjectionAverage(
			GetWorld(),
			FlattenPosition - Direction * Radius,
			Direction,
			Radius,
			EVoxelProjectionShape::Circle,
			NumRays,
			2 * Radius,
			Position,
			Normal);
	}

	ON_SCOPE_EXIT
	{
		FlattenPosition = Position;
	};

	if (!bFreezeOnClick)
	{
		return true;
	}

	if (!FirstHeight.IsSet())
	{
		FirstHeight = Position.Z;
		return true;
	}

	const FPlane Plane(FVector(0.f, 0.f, FirstHeight.GetValue()), FVector::UpVector);
	Position = FMath::RayPlaneIntersection(GetLastRay().Origin, GetLastRay().Direction, Plane);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelHeightModifier> UVoxelFlattenHeightTool::GetModifier(const float StrengthMultiplier) const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelFlattenHeightModifier> Modifier = MakeShared<FVoxelFlattenHeightModifier>();
	Modifier->Center = FVector2D(FlattenPosition);
	Modifier->TargetHeight = FlattenPosition.Z;
	Modifier->Radius = Radius;
	Modifier->Falloff = Falloff;
	Modifier->Type = Type;
	Modifier->Brush = Brush.GetBrush(GetHitNormal(), GetStrokeDirection());
	return Modifier;
}