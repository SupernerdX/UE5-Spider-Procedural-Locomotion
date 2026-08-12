// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelSphereVolumeTool.h"
#include "Sculpt/Volume/Tools/VoxelSphereVolumeModifier.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelSphereVolumeTool::Enter()
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

	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Tools/M_ToolSphere.M_ToolSphere"));
	ensure(Material);

	PreviewMaterial = UMaterialInstanceDynamic::Create(Material, nullptr);
	PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);

	PreviewActor->GetStaticMeshComponent()->SetMaterial(0, PreviewMaterial);
}

void UVoxelSphereVolumeTool::Exit()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Exit();
}

void UVoxelSphereVolumeTool::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	check(PreviewActor);
	PreviewActor->SetActorLocation(GetHitLocation() + Offset);

	PreviewActor->SetActorScale3D(FVector(Radius / 50.f));
}

#if WITH_EDITOR
void UVoxelSphereVolumeTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_STATIC(UVoxelSphereVolumeTool, Radius))
	{
		PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelVolumeModifier> UVoxelSphereVolumeTool::GetModifier(const float StrengthMultiplier) const
{
	const TSharedRef<FVoxelSphereVolumeModifier> Modifier = MakeShared<FVoxelSphereVolumeModifier>();
	Modifier->Center = GetHitLocation() + Offset;
	Modifier->Radius = Radius;
	Modifier->Smoothness = Smoothness;
	Modifier->Mode = InverseAction() ? EVoxelSculptMode::Remove : EVoxelSculptMode::Add;
	return Modifier;
}