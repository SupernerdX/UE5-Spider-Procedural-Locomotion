// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelSurfaceVolumeTool.h"
#include "Sculpt/Volume/Tools/VoxelSurfaceVolumeModifier.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelSurfaceVolumeTool::Enter()
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

void UVoxelSurfaceVolumeTool::Tick()
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

TSharedPtr<FVoxelVolumeModifier> UVoxelSurfaceVolumeTool::GetModifier(const float StrengthMultiplier) const
{
	const TSharedRef<FVoxelSurfaceVolumeModifier> Modifier = MakeShared<FVoxelSurfaceVolumeModifier>();
	Modifier->Center = GetHitLocation();
	Modifier->Radius = Radius;
	Modifier->Strength = (InverseAction() ? Strength : -Strength) * StrengthMultiplier;
	Modifier->Brush = Brush.GetBrush(GetHitNormal(), GetStrokeDirection());
	return Modifier;
}