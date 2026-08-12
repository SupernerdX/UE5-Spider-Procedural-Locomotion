// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Graphs/VoxelHeightGraphStamp.h"
#include "Graphs/VoxelHeightGraph.h"
#include "Graphs/VoxelStampGraphParameters.h"
#include "Graphs/VoxelOutputNode_OutputHeight.h"
#include "VoxelQuery.h"
#include "VoxelStampRef.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"
#include "VoxelStampUtilities.h"
#include "VoxelGraphPositionParameter.h"

TSharedPtr<FVoxelGraphEnvironment> FVoxelHeightGraphStamp::CreateEnvironment(
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

	FVoxelGraphParameters::FHeightStamp& Parameter = Environment->AddParameter<FVoxelGraphParameters::FHeightStamp>();
	Parameter.Smoothness = Smoothness;
	Parameter.BlendMode = BlendMode;

	Environment->AddParameter<FVoxelGraphParameters::FStampSeed>(StampSeed.GetSeed());

	return Environment;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedPtr<IVoxelParameterOverridesOwner> FVoxelHeightGraphStamp::GetShared() const
{
	return SharedThis(ConstCast(this));
}

bool FVoxelHeightGraphStamp::ShouldForceEnableOverride(const FGuid& Guid) const
{
	return true;
}

UVoxelGraph* FVoxelHeightGraphStamp::GetGraph() const
{
	return Graph;
}

FVoxelParameterOverrides& FVoxelHeightGraphStamp::GetParameterOverrides()
{
	return ParameterOverrides;
}

FProperty* FVoxelHeightGraphStamp::GetParameterOverridesProperty() const
{
	return &FindFPropertyChecked(FVoxelHeightGraphStamp, ParameterOverrides);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UObject* FVoxelHeightGraphStamp::GetAsset() const
{
	return Graph;
}

void FVoxelHeightGraphStamp::FixupProperties()
{
	Super::FixupProperties();

	FixupParameterOverrides();

	// Migrate old Add blend mode
	if (int32(BlendMode) >= StaticEnum<EVoxelHeightBlendMode>()->GetMaxEnumValue())
	{
		BlendMode = EVoxelHeightBlendMode::Override;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelHeightGraphStampRuntime::Initialize(FVoxelDependencyCollector& DependencyCollector)
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

	Evaluator = FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputHeight>(Environment.ToSharedRef());

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

bool FVoxelHeightGraphStampRuntime::Initialize_Parallel(FVoxelDependencyCollector& DependencyCollector)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelGraphContext Context = Evaluator.MakeContext(DependencyCollector);
	const FVoxelGraphQueryImpl& GraphQuery = Context.MakeQuery();

	LocalBounds = Evaluator->BoundsPin.GetSynchronous(GraphQuery);
	HeightRange = Evaluator->HeightRangePin.GetSynchronous(GraphQuery);

	bRelativeHeightRange =
		GetBlendMode() == EVoxelHeightBlendMode::Override &&
		Evaluator->RelativeHeightRangePin.GetSynchronous(GraphQuery);

	if (!LocalBounds.IsValidAndNotEmpty())
	{
		return false;
	}

	MetadataOverrides = Stamp.MetadataOverrides.CreateRuntime(GraphQuery, Evaluator);

	FVoxelHeightGraphStamp& MutableStamp = GetMutableStamp();
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

FVoxelBox FVoxelHeightGraphStampRuntime::GetLocalBounds() const
{
	return LocalBounds.ToBox3D(HeightRange.Min, HeightRange.Max);
}

bool FVoxelHeightGraphStampRuntime::ShouldUseQueryPrevious() const
{
	return GetBlendMode() == EVoxelHeightBlendMode::Override;
}

bool FVoxelHeightGraphStampRuntime::HasRelativeHeightRange() const
{
	return bRelativeHeightRange;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightGraphStampRuntime::Apply(
	const FVoxelHeightBulkQuery& Query,
	const FVoxelHeightTransform& StampToQuery) const
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

	const FVoxelDoubleVector2DBuffer Positions = FVoxelStampUtilities::ComputePositions(
		Query,
		StampToQuery);

	FVoxelGraphContext Context = Evaluator.MakeContext(Query.Query.DependencyCollector);

	FVoxelGraphQueryImpl& GraphQuery = Context.MakeQuery();
	GraphQuery.AddParameter<FVoxelGraphParameters::FLOD>().Value = Query.Query.LOD;
	GraphQuery.AddParameter<FVoxelGraphParameters::FQuery>(Query.Query);
	GraphQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetLocalPosition(Positions);
	GraphQuery.AddParameter<FVoxelGraphParameters::FHeightQuery>(Query, StampToQuery);

	FVoxelStampUtilities::ApplyHeights(
		*this,
		Query,
		StampToQuery,
		*Evaluator->HeightPin.GetSynchronous(GraphQuery),
		Positions);
}

void FVoxelHeightGraphStampRuntime::Apply(
	const FVoxelHeightSparseQuery& Query,
	const FVoxelHeightTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	const FVoxelDoubleVector2DBuffer Positions = FVoxelStampUtilities::ComputePositions(
		Query,
		StampToQuery);

	FVoxelGraphContext Context = Evaluator.MakeContext(Query.Query.DependencyCollector);

	FVoxelGraphQueryImpl& GraphQuery = Context.MakeQuery();
	GraphQuery.AddParameter<FVoxelGraphParameters::FLOD>().Value = Query.Query.LOD;
	GraphQuery.AddParameter<FVoxelGraphParameters::FQuery>(Query.Query);
	GraphQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetLocalPosition(Positions);
	GraphQuery.AddParameter<FVoxelGraphParameters::FHeightQuery>(Query, StampToQuery);

	FVoxelStampUtilities::ComputeHeightStamp(
		*this,
		Query,
		StampToQuery,
		*MetadataOverrides,
		GraphQuery,
		Evaluator,
		Positions);
}