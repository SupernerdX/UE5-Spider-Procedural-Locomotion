// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelCubeVolumeTool.h"
#include "Sculpt/Volume/Tools/VoxelCubeVolumeModifier.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelCubeVolumeTool::Enter()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Enter();

	PreviewActor = SpawnActor<AStaticMeshActor>();
	if (!ensure(PreviewActor))
	{
		return;
	}

	PreviewActor->SetActorEnableCollision(false);

	UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	ensure(StaticMesh);
	PreviewActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);

	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Tools/M_ToolCube.M_ToolCube"));
	ensure(Material);

	PreviewMaterial = UMaterialInstanceDynamic::Create(Material, nullptr);
	PreviewMaterial->SetVectorParameterValue(STATIC_FNAME("Size"), Size);
	PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Roundness"), Roundness);

	PreviewActor->GetStaticMeshComponent()->SetMaterial(0, PreviewMaterial);
}

void UVoxelCubeVolumeTool::Exit()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Exit();
}

void UVoxelCubeVolumeTool::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	check(PreviewActor);

	PreviewActor->SetActorTransform(FTransform(
		Rotation,
		GetHitLocation() + Offset,
		FVector(Size / 100.f)));
}

#if WITH_EDITOR
void UVoxelCubeVolumeTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_STATIC(UVoxelCubeVolumeTool, Size))
	{
		PreviewMaterial->SetVectorParameterValue(STATIC_FNAME("Size"), Size);
	}
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_STATIC(UVoxelCubeVolumeTool, Roundness))
	{
		PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Roundness"), Roundness);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelVolumeModifier> UVoxelCubeVolumeTool::GetModifier(const float StrengthMultiplier) const
{
	const TSharedRef<FVoxelCubeVolumeModifier> Modifier = MakeShared<FVoxelCubeVolumeModifier>();
	Modifier->Center = GetHitLocation() + Offset;
	Modifier->Size = Size;
	Modifier->Rotation = Rotation;
	Modifier->Roundness = Roundness;
	Modifier->Smoothness = Smoothness;
	Modifier->Mode = InverseAction() ? EVoxelSculptMode::Remove : EVoxelSculptMode::Add;
	return Modifier;
}