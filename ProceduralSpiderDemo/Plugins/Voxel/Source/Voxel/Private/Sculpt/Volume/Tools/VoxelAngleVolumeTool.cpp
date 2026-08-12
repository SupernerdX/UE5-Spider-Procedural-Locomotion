// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelAngleVolumeTool.h"
#include "VoxelConfig.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelAngleVolumeTool::Enter()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Enter();

	PreviewActor = SpawnActor<AStaticMeshActor>();
	if (!ensure(PreviewActor))
	{
		return;
	}

	PreviewActor->SetActorEnableCollision(false);

	UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	ensure(StaticMesh);
	PreviewActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
	PreviewActor->GetStaticMeshComponent()->SetMaterial(0, PreviewMaterial);
}

void UVoxelAngleVolumeTool::Exit()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Exit();
}

void UVoxelAngleVolumeTool::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	check(PreviewActor);
	PreviewActor->SetActorLocation(GetHitLocation());

	PreviewActor->SetActorScale3D(FVector(Radius / 50.f));

	UMaterialInstanceDynamic* Material = PreviewMaterial;
	Brush.UpdateMaterial(Material, Radius, GetHitNormal(), GetStrokeDirection(), false);
	if (PreviewMaterial != Material)
	{
		PreviewMaterial = Material;
		PreviewActor->GetStaticMeshComponent()->SetMaterial(0, PreviewMaterial);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelAngleVolumeTool::OnEditBegin()
{
	bHasStoredValue = false;
}

bool UVoxelAngleVolumeTool::PrepareModifierData()
{
	if (bFreezeOnClick &&
		bHasStoredValue)
	{
		return false;
	}

	LastClickFlattenPosition = GetHitLocation();
	LastClickFlattenNormal = GetHitNormal();

	if (bUseAverage)
	{
		const FVector Direction = GetTraceDirection();

		FindProjectionAverage(
			GetWorld(),
			LastClickFlattenPosition - Direction * Radius,
			Direction,
			Radius,
			EVoxelProjectionShape::Circle,
			100.f,
			2 * Radius,
			LastClickFlattenPosition,
			LastClickFlattenNormal);
	}

	if (bUseFixedRotation)
	{
		LastClickFlattenNormal = FixedRotation.RotateVector(FVector::UpVector).GetSafeNormal();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelVolumeModifier> UVoxelAngleVolumeTool::GetModifier(const float StrengthMultiplier) const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelAngleVolumeModifier> Modifier = MakeShared<FVoxelAngleVolumeModifier>();
	Modifier->Center = GetHitLocation();
	Modifier->Radius = Radius;
	Modifier->Strength = Strength * StrengthMultiplier;
	Modifier->Plane = FPlane(LastClickFlattenPosition, LastClickFlattenNormal.GetSafeNormal());
	Modifier->MergeMode = MergeMode;
	Modifier->Brush = Brush.GetBrush(GetHitNormal(), GetStrokeDirection());
	return Modifier;
}