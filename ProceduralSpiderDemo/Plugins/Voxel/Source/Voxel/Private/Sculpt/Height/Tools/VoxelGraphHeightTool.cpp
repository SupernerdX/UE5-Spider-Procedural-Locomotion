// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/Tools/VoxelGraphHeightTool.h"
#include "Sculpt/Height/Tools/VoxelGraphHeightModifier.h"
#include "Sculpt/Height/VoxelHeightSculptGraph.h"
#include "Sculpt/VoxelOutputNode_OutputSculptHeight.h"
#include "Sculpt/VoxelToolBrush.h"
#include "VoxelNodeEvaluator.h"
#include "VoxelGraphEnvironment.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVoxelGraphHeightTool::Enter()
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
}

void UVoxelGraphHeightTool::Exit()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Exit();
}

void UVoxelGraphHeightTool::Tick()
{
	VOXEL_FUNCTION_COUNTER();

	Super::Tick();

	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptHeight> Evaluator = GetEvaluator();
	if (!Evaluator)
	{
		return;
	}

	FVoxelGraphContext Context = Evaluator.MakeContext(FVoxelDependencyCollector::Null);

	const float Radius = Evaluator->RadiusPin.GetSynchronous(Context.MakeQuery());

	check(PreviewActor);
	PreviewActor->SetActorLocation(GetHitLocation());

	PreviewActor->SetActorScale3D(FVector(Radius / 50.f, Radius / 50.f, 1000.f));

	UMaterialInstanceDynamic* Material = PreviewMaterial;
	FVoxelToolBrush Brush;
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

TSharedPtr<FVoxelHeightModifier> UVoxelGraphHeightTool::GetModifier(const float StrengthMultiplier) const
{
	VOXEL_FUNCTION_COUNTER();

	const TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptHeight> Evaluator = GetEvaluator();
	if (!Evaluator)
	{
		return {};
	}

	FVoxelGraphContext Context = Evaluator.MakeContext(FVoxelDependencyCollector::Null);

	const TSharedRef<FVoxelGraphHeightModifier> Modifier = MakeShared<FVoxelGraphHeightModifier>();
	Modifier->Center = FVector2D(GetHitLocation());
	Modifier->Radius = Evaluator->RadiusPin.GetSynchronous(Context.MakeQuery());
	Modifier->Graph.Graph = Graph;
	Modifier->Graph.ParameterOverrides = ParameterOverrides;
	return Modifier;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelGraphHeightTool::ShouldForceEnableOverride(const FGuid& Guid) const
{
	return true;
}

UVoxelGraph* UVoxelGraphHeightTool::GetGraph() const
{
	return Graph;
}

FVoxelParameterOverrides& UVoxelGraphHeightTool::GetParameterOverrides()
{
	return ParameterOverrides;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelNodeEvaluator<FVoxelOutputNode_OutputSculptHeight> UVoxelGraphHeightTool::GetEvaluator() const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedPtr<FVoxelGraphEnvironment> Environment = FVoxelGraphEnvironment::Create(
		ConstCast(this),
		*this,
		FTransform(FVector(FVector2D(GetHitLocation()), 0.f)),
		FVoxelDependencyCollector::Null);

	if (!Environment)
	{
		return {};
	}

	return FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputSculptHeight>(Environment.ToSharedRef());
}