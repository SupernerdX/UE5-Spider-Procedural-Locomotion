// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/Tools/VoxelSculptHeightTool.h"
#include "Sculpt/Height/Tools/VoxelSculptHeightModifier.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelSculptHeightTool::Enter()
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

void UVoxelSculptHeightTool::Exit()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Exit();
}

void UVoxelSculptHeightTool::Tick()
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

TSharedPtr<FVoxelHeightModifier> UVoxelSculptHeightTool::GetModifier(const float StrengthMultiplier) const
{
	const TSharedRef<FVoxelSculptHeightModifier> Modifier = MakeShared<FVoxelSculptHeightModifier>();
	Modifier->Center = FVector2D(GetHitLocation());
	Modifier->Radius = Radius;
	Modifier->Strength = Strength * StrengthMultiplier;
	Modifier->Mode = InverseAction() ? EVoxelSculptMode::Remove : EVoxelSculptMode::Add;
	Modifier->Brush = Brush.GetBrush(GetHitNormal(), GetStrokeDirection());
	return Modifier;
}