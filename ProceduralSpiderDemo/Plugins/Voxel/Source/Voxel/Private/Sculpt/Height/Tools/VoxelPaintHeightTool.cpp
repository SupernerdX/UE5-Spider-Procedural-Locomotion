// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/Tools/VoxelPaintHeightTool.h"
#include "Sculpt/Height/Tools/VoxelPaintHeightModifier.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelPaintHeightTool::Enter()
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

void UVoxelPaintHeightTool::Exit()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Exit();
}

void UVoxelPaintHeightTool::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	check(PreviewActor);
	PreviewActor->SetActorLocation(GetHitLocation());

	PreviewActor->SetActorScale3D(FVector(Radius / 50.f, Radius / 50.f, 1000.f));

	UMaterialInstanceDynamic* MaterialInstance = PreviewMaterial;
	Brush.UpdateMaterial(MaterialInstance, Radius, GetHitNormal(), GetStrokeDirection(), true);
	if (PreviewMaterial != MaterialInstance)
	{
		PreviewMaterial = MaterialInstance;
		PreviewActor->GetStaticMeshComponent()->SetMaterial(0, PreviewMaterial);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelHeightModifier> UVoxelPaintHeightTool::GetModifier(const float StrengthMultiplier) const
{
	const TSharedRef<FVoxelPaintHeightModifier> Modifier = MakeShared<FVoxelPaintHeightModifier>();
	Modifier->Center = FVector2D(GetHitLocation());
	Modifier->SurfaceTypeToPaint = SurfaceTypeToPaint.SurfaceType;
	Modifier->MetadatasToPaint = MetadatasToPaint;
	Modifier->Radius = Radius;
	Modifier->Strength = Strength * StrengthMultiplier;
	Modifier->Mode = InverseAction() ? EVoxelSculptMode::Remove : EVoxelSculptMode::Add;
	Modifier->Brush = Brush.GetBrush(GetHitNormal(), GetStrokeDirection());
	return Modifier;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelPaintHeightTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	MetadatasToPaint.Fixup();
}
#endif