// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelGraphVolumeTool.h"
#include "Sculpt/Volume/Tools/VoxelGraphVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptGraph.h"
#include "Sculpt/VoxelOutputNode_OutputSculptDistance.h"
#include "Sculpt/VoxelToolBrush.h"
#include "VoxelNodeEvaluator.h"
#include "VoxelGraphEnvironment.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelGraphVolumeTool::Enter()
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

void UVoxelGraphVolumeTool::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptDistance> Evaluator = GetEvaluator();
	if (!Evaluator)
	{
		return;
	}

	FVoxelGraphContext Context = Evaluator.MakeContext(FVoxelDependencyCollector::Null);

	const float Radius = Evaluator->RadiusPin.GetSynchronous(Context.MakeQuery());

	check(PreviewActor);
	PreviewActor->SetActorLocation(GetHitLocation());

	PreviewActor->SetActorScale3D(FVector(Radius / 50.f));

	UMaterialInstanceDynamic* Material = PreviewMaterial;
	FVoxelToolBrush Brush;
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

TSharedPtr<FVoxelVolumeModifier> UVoxelGraphVolumeTool::GetModifier(const float StrengthMultiplier) const
{
	VOXEL_FUNCTION_COUNTER();

	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptDistance> Evaluator = GetEvaluator();
	if (!Evaluator)
	{
		return {};
	}

	FVoxelGraphContext Context = Evaluator.MakeContext(FVoxelDependencyCollector::Null);

	const TSharedRef<FVoxelGraphVolumeModifier> Modifier = MakeShared<FVoxelGraphVolumeModifier>();
	Modifier->Transform = FTransform(
		FRotationMatrix::MakeFromXZ(GetTraceDirection(), GetHitNormal()).ToQuat(),
		GetHitLocation());

	Modifier->Radius = Evaluator->RadiusPin.GetSynchronous(Context.MakeQuery());
	Modifier->Graph.Graph = Graph;
	Modifier->Graph.ParameterOverrides = ParameterOverrides;
	return Modifier;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelGraphVolumeTool::ShouldForceEnableOverride(const FGuid& Guid) const
{
	return true;
}

UVoxelGraph* UVoxelGraphVolumeTool::GetGraph() const
{
	return Graph;
}

FVoxelParameterOverrides& UVoxelGraphVolumeTool::GetParameterOverrides()
{
	return ParameterOverrides;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptDistance> UVoxelGraphVolumeTool::GetEvaluator() const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedPtr<FVoxelGraphEnvironment> Environment = FVoxelGraphEnvironment::Create(
		ConstCast(this),
		*this,
		FTransform(
			FRotationMatrix::MakeFromXZ(GetTraceDirection(), GetHitNormal()).ToQuat(),
			GetHitLocation()),
		FVoxelDependencyCollector::Null);

	if (!Environment)
	{
		return {};
	}

	return FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputSculptDistance>(Environment.ToSharedRef());
}