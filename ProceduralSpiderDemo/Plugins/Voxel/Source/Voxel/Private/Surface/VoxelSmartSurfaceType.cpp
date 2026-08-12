// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSmartSurfaceType.h"
#include "Surface/VoxelSmartSurfaceTypeGraph.h"
#include "Surface/VoxelOutputNode_OutputSurface.h"
#include "VoxelDependency.h"
#include "VoxelGraphTracker.h"
#include "VoxelGraphEnvironment.h"

DEFINE_VOXEL_FACTORY(UVoxelSmartSurfaceType);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelSmartSurfaceType::ShouldForceEnableOverride(const FGuid& ParameterGuid) const
{
	return true;
}

UVoxelGraph* UVoxelSmartSurfaceType::GetGraph() const
{
	return Graph;
}

FVoxelParameterOverrides& UVoxelSmartSurfaceType::GetParameterOverrides()
{
	return ParameterOverrides;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSmartSurfaceType::PostLoad()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostLoad();

	FixupParameterOverrides();
}

void UVoxelSmartSurfaceType::PostInitProperties()
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostInitProperties();

	FixupParameterOverrides();
}

#if WITH_EDITOR
void UVoxelSmartSurfaceType::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	VOXEL_FUNCTION_COUNTER();

	Super::PostEditChangeProperty(PropertyChangedEvent);

	FixupParameterOverrides();

	if (ParameterOverrides != LastParameterOverrides)
	{
		LastParameterOverrides = ParameterOverrides;
		Dependency->Invalidate();
	}

	OnChangedPtr = MakeSharedVoid();

	if (Graph)
	{
		GVoxelGraphTracker->OnParameterValueChanged(*Graph).Add(FOnVoxelGraphChanged::Make(OnChangedPtr, MakeWeakObjectPtrLambda(this, [this]
		{
			Dependency->Invalidate();
		})));
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDependency& UVoxelSmartSurfaceType::GetDependency()
{
	if (!Dependency)
	{
		Dependency = FVoxelDependency::Create(GetName());
	}

	return *Dependency;
}

TVoxelNodeEvaluator<FVoxelOutputNode_OutputSurface> UVoxelSmartSurfaceType::CreateEvaluator(FVoxelDependencyCollector& DependencyCollector) const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedPtr<FVoxelGraphEnvironment> Environment = FVoxelGraphEnvironment::Create(
		ConstCast(this),
		*this,
		FTransform::Identity,
		DependencyCollector);

	if (!Environment)
	{
		return {};
	}

	return FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputSurface>(Environment.ToSharedRef());
}