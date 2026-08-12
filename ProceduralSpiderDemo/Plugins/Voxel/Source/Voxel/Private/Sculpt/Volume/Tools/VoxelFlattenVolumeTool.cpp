// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelFlattenVolumeTool.h"
#include "Sculpt/Volume/Tools/VoxelFlattenVolumeModifier.h"
#include "VoxelConfig.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelFlattenVolumeTool::Enter()
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

	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Tools/M_ToolFlatten.M_ToolFlatten"));
	ensure(Material);

	PreviewMaterial = UMaterialInstanceDynamic::Create(Material, nullptr);
	PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);
	PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Height"), Height);
	PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff);

	PreviewActor->GetStaticMeshComponent()->SetMaterial(0, PreviewMaterial);
}

void UVoxelFlattenVolumeTool::Exit()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Exit();
}

void UVoxelFlattenVolumeTool::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	check(PreviewActor);

	FVector PreviewPosition = GetHitLocation();
	FVector PreviewNormal = GetHitNormal();
	if (bUseAverage)
	{
		const FVector Direction = GetTraceDirection();
		FindProjectionAverage(
			GetWorld(),
			PreviewPosition - Direction * Radius,
			Direction,
			Radius,
			EVoxelProjectionShape::Circle,
			NumRays,
			2 * Radius,
			PreviewPosition,
			PreviewNormal);
	}

	if (bFreezeOnClick &&
		FirstHitPlane.IsSet())
	{
		const FPlane FirstPlane = FirstHitPlane.GetValue();
		PreviewPosition = FMath::RayPlaneIntersection(GetLastRay().Origin, GetLastRay().Direction, FirstPlane);
		PreviewNormal = FirstPlane.GetNormal();
	}

	PreviewActor->SetActorLocation(PreviewPosition);
	if (bUseSlopeFlatten)
	{
		PreviewActor->SetActorRotation(FRotationMatrix::MakeFromZ(PreviewNormal).Rotator());
	}
	else
	{
		PreviewActor->SetActorRotation(FRotator::ZeroRotator);
	}

	PreviewActor->SetActorScale3D(FVector(Radius / 50.f, Radius / 50.f, Height / 50.f));
}

#if WITH_EDITOR
void UVoxelFlattenVolumeTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_STATIC(UVoxelFlattenVolumeTool, Radius))
	{
		PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);
	}
	else if (PropertyName == GET_MEMBER_NAME_STATIC(UVoxelFlattenVolumeTool, Height))
	{
		PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Height"), Height);
	}
	else if (PropertyName == GET_MEMBER_NAME_STATIC(UVoxelFlattenVolumeTool, Falloff))
	{
		PreviewMaterial->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff);
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelFlattenVolumeTool::OnEditBegin()
{
	FirstHitPlane.Reset();
}

void UVoxelFlattenVolumeTool::OnEditEnd()
{
	FirstHitPlane.Reset();
}

bool UVoxelFlattenVolumeTool::PrepareModifierData()
{
	Position = GetHitLocation();
	Normal = GetHitNormal();

	if (bUseAverage)
	{
		const FVector Direction = GetTraceDirection();
		FindProjectionAverage(
			GetWorld(),
			Position - Direction * Radius,
			Direction,
			Radius,
			EVoxelProjectionShape::Circle,
			NumRays,
			2 * Radius,
			Position,
			Normal);
	}

	if (!bUseSlopeFlatten)
	{
		Normal = FVector::UpVector;
	}

	if (!bFreezeOnClick)
	{
		return true;
	}

	if (!FirstHitPlane.IsSet())
	{
		FirstHitPlane = FPlane(Position, Normal);
		return true;
	}

	const FPlane FirstPlane = FirstHitPlane.GetValue();
	Position = FMath::RayPlaneIntersection(GetLastRay().Origin, GetLastRay().Direction, FirstPlane);
	Normal = FirstPlane.GetNormal();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<FVoxelVolumeModifier> UVoxelFlattenVolumeTool::GetModifier(const float StrengthMultiplier) const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedRef<FVoxelFlattenVolumeModifier> Modifier = MakeShared<FVoxelFlattenVolumeModifier>();
	Modifier->Center = Position;
	Modifier->Normal = Normal;
	Modifier->Radius = Radius;
	Modifier->Height = Height;
	Modifier->Falloff = Falloff;
	Modifier->Type = Type;
	return Modifier;
}