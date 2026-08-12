// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelVolumeGraphStamp.h"
#include "Graphs/VoxelVolumeGraph.h"
#include "Graphs/VoxelStampGraphParameters.h"
#include "Graphs/VoxelOutputNode_OutputVolume.h"
#include "VoxelQuery.h"
#include "VoxelStampRef.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "VoxelStampUtilities.h"
#include "VoxelGraphPositionParameter.h"

TSharedPtr<FVoxelGraphEnvironment> FVoxelVolumeGraphStamp::CreateEnvironment(
	const FVoxelStampRuntime& Runtime,
	FVoxelDependencyCollector& DependencyCollector) const
{
	VOXEL_FUNCTION_COUNTER();

	const TSharedPtr<FVoxelGraphEnvironment> Environment = FVoxelGraphEnvironment::Create(
		Runtime.GetComponent(),
		*this,
		Runtime.GetLocalToWorld(),
		DependencyCollector);

	if (!Environment)
	{
		return nullptr;
	}

	FVoxelGraphParameters::FVolumeStamp& Parameter = Environment->AddParameter<FVoxelGraphParameters::FVolumeStamp>();
	Parameter.Smoothness = Smoothness;
	Parameter.BlendMode = BlendMode;

	Environment->AddParameter<FVoxelGraphParameters::FStampSeed>(StampSeed.GetSeed());

	return Environment;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<IVoxelParameterOverridesOwner> FVoxelVolumeGraphStamp::GetShared() const
{
	return SharedThis(ConstCast(this));
}

bool FVoxelVolumeGraphStamp::ShouldForceEnableOverride(const FGuid& Guid) const
{
	return true;
}

UVoxelGraph* FVoxelVolumeGraphStamp::GetGraph() const
{
	return Graph;
}

FVoxelParameterOverrides& FVoxelVolumeGraphStamp::GetParameterOverrides()
{
	return ParameterOverrides;
}

FProperty* FVoxelVolumeGraphStamp::GetParameterOverridesProperty() const
{
	return &FindFPropertyChecked(FVoxelVolumeGraphStamp, ParameterOverrides);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UObject* FVoxelVolumeGraphStamp::GetAsset() const
{
	return Graph;
}

void FVoxelVolumeGraphStamp::FixupProperties()
{
	Super::FixupProperties();

	FixupParameterOverrides();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelVolumeGraphStampRuntime::Initialize(FVoxelDependencyCollector& DependencyCollector)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!Stamp.Graph)
	{
		return false;
	}

	const TSharedPtr<const FVoxelGraphEnvironment> Environment = Stamp.CreateEnvironment(
		*this,
		DependencyCollector);

	if (!Environment)
	{
		return false;
	}

	Evaluator = FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputVolume>(Environment.ToSharedRef());

	if (!Evaluator)
	{
		return false;
	}

#if WITH_EDITOR
	GVoxelGraphTracker->OnParameterValueChanged(*Stamp.Graph).Add(FOnVoxelGraphChanged::Make(this, [this]
	{
		RequestUpdate();
	}));
#endif

	return true;
}

bool FVoxelVolumeGraphStampRuntime::Initialize_Parallel(FVoxelDependencyCollector& DependencyCollector)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelGraphContext Context = Evaluator.MakeContext(DependencyCollector);
	const FVoxelGraphQueryImpl& GraphQuery = Context.MakeQuery();

	LocalBounds = Evaluator->BoundsPin.GetSynchronous(GraphQuery);

	if (!LocalBounds.IsValidAndNotEmpty())
	{
		return false;
	}

	MetadataOverrides = Stamp.MetadataOverrides.CreateRuntime(GraphQuery, Evaluator);

	FVoxelVolumeGraphStamp& MutableStamp = GetMutableStamp();
	MutableStamp.bDisableEditingLayers = Evaluator->EnableLayerOverridePin.GetSynchronous(GraphQuery);
	MutableStamp.bDisableEditingBlendMode = Evaluator->EnableBlendModeOverridePin.GetSynchronous(GraphQuery);

	if (MutableStamp.bDisableEditingLayers)
	{
		MutableStamp.Layer = Evaluator->LayerOverridePin.GetSynchronous(GraphQuery).Layer.Resolve_Ensured();
		MutableStamp.AdditionalLayers = {};
	}

	if (MutableStamp.bDisableEditingBlendMode)
	{
		MutableStamp.BlendMode = Evaluator->BlendModeOverridePin.GetSynchronous(GraphQuery);
	}

	return true;
}

FVoxelBox FVoxelVolumeGraphStampRuntime::GetLocalBounds() const
{
	return LocalBounds;
}

bool FVoxelVolumeGraphStampRuntime::ShouldUseQueryPrevious() const
{
	return GetBlendMode() == EVoxelVolumeBlendMode::Override;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeGraphStampRuntime::Apply(
	const FVoxelVolumeBulkQuery& Query,
	const FVoxelVolumeTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	if (!AffectShape())
	{
		if (ShouldUseQueryPrevious() &&
			ensure(Query.QueryPrevious))
		{
			Query.QueryPrevious->Query(Query);
		}

		return;
	}

	const FVoxelDoubleVectorBuffer Positions = FVoxelStampUtilities::ComputePositions(
		Query,
		StampToQuery);

	FVoxelGraphContext Context = Evaluator.MakeContext(Query.Query.DependencyCollector);

	FVoxelGraphQueryImpl& GraphQuery = Context.MakeQuery();
	GraphQuery.AddParameter<FVoxelGraphParameters::FLOD>().Value = Query.Query.LOD;
	GraphQuery.AddParameter<FVoxelGraphParameters::FQuery>(Query.Query);
	GraphQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(Positions);
	GraphQuery.AddParameter<FVoxelGraphParameters::FVolumeQuery>(Query, StampToQuery);

	FVoxelStampUtilities::ApplyDistances(
		*this,
		Query,
		StampToQuery,
		*Evaluator->DistancePin.GetSynchronous(GraphQuery));
}

void FVoxelVolumeGraphStampRuntime::Apply(
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const FVoxelDoubleVectorBuffer Positions = FVoxelStampUtilities::ComputePositions(
		Query,
		StampToQuery);

	FVoxelGraphContext Context = Evaluator.MakeContext(Query.Query.DependencyCollector);

	FVoxelGraphQueryImpl& GraphQuery = Context.MakeQuery();
	GraphQuery.AddParameter<FVoxelGraphParameters::FLOD>().Value = Query.Query.LOD;
	GraphQuery.AddParameter<FVoxelGraphParameters::FQuery>(Query.Query);
	GraphQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(Positions);
	GraphQuery.AddParameter<FVoxelGraphParameters::FVolumeQuery>(Query, StampToQuery);

	FVoxelStampUtilities::ComputeVolumeStamp(
		*this,
		Query,
		StampToQuery,
		*MetadataOverrides,
		GraphQuery,
		Evaluator,
		Positions);
}