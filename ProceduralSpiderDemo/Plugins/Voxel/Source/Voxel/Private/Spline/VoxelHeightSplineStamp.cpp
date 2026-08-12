// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Spline/VoxelHeightSplineStamp.h"
#include "Spline/VoxelHeightSplineGraph.h"
#include "Spline/VoxelSplineMetadata.h"
#include "Spline/VoxelSplineComponent.h"
#include "Spline/VoxelSplineGraphParameters.h"
#include "Spline/VoxelOutputNode_OutputHeightSpline.h"
#include "VoxelStampRef.h"
#include "VoxelGraphTracker.h"
#include "VoxelGraphPositionParameter.h"
#include "VoxelQuery.h"
#include "VoxelTerminalGraph.h"
#include "VoxelStampUtilities.h"
#include "Graphs/VoxelStampGraphParameters.h"

TSharedPtr<FVoxelGraphEnvironment> FVoxelHeightSplineStamp::CreateEnvironment(
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

TSharedPtr<IVoxelParameterOverridesOwner> FVoxelHeightSplineStamp::GetShared() const
{
	return SharedThis(ConstCast(this));
}

bool FVoxelHeightSplineStamp::ShouldForceEnableOverride(const FGuid& Guid) const
{
	return true;
}

UVoxelGraph* FVoxelHeightSplineStamp::GetGraph() const
{
	return Graph;
}

FVoxelParameterOverrides& FVoxelHeightSplineStamp::GetParameterOverrides()
{
	return ParameterOverrides;
}

FProperty* FVoxelHeightSplineStamp::GetParameterOverridesProperty() const
{
	return &FindFPropertyChecked(FVoxelHeightSplineStamp, ParameterOverrides);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UObject* FVoxelHeightSplineStamp::GetAsset() const
{
	return Graph;
}

void FVoxelHeightSplineStamp::FixupProperties()
{
	Super::FixupProperties();

	FixupParameterOverrides();
}

void FVoxelHeightSplineStamp::FixupComponents(const IVoxelStampComponentInterface& Interface)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Graph)
	{
		return;
	}

	const UVoxelSplineComponent* Component = Interface.FindComponent<UVoxelSplineComponent>();
	if (!Component ||
		!ensure(Component->Metadata))
	{
		return;
	}

	Component->Metadata->Fixup(*Graph);
}

TVoxelArray<TSubclassOf<USceneComponent>> FVoxelHeightSplineStamp::GetRequiredComponents() const
{
	return { UVoxelSplineComponent::StaticClass() };
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelHeightSplineStampRuntime::Initialize(FVoxelDependencyCollector& DependencyCollector)
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

	Evaluator = FVoxelNodeEvaluator::Create<FVoxelOutputNode_OutputHeightSpline>(Environment.ToSharedRef());

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

	const UVoxelSplineComponent* Component = FindComponent<UVoxelSplineComponent>();
	if (!Component ||
		Component->GetNumberOfSplinePoints() <= 1 ||
		!ensure(Component->Metadata))
	{
		// NumPoints = 1 is annoying to deal with
		return false;
	}

	MetadataRuntime = Component->Metadata->GetRuntime();

	bClosedLoop = Component->IsClosedLoop();
	ReparamStepsPerSegment = Component->ReparamStepsPerSegment;
	SplineLength = Component->GetSplineLength();
	SplineCurves = Component->SplineCurves;

	Segments = FVoxelSplineSegment::Create(SplineCurves, *MetadataRuntime);

	return true;
}

bool FVoxelHeightSplineStampRuntime::Initialize_Parallel(FVoxelDependencyCollector& DependencyCollector)
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelGraphContext Context = Evaluator.MakeContext(DependencyCollector);
	FVoxelGraphQueryImpl& GraphQuery = Context.MakeQuery();

	MaxWidth = Evaluator->MaxWidthPin.GetSynchronous(GraphQuery);
	HeightRange = Evaluator->HeightRangePin.GetSynchronous(GraphQuery);

	bRelativeHeightRange =
		GetBlendMode() == EVoxelHeightBlendMode::Override &&
		Evaluator->RelativeHeightRangePin.GetSynchronous(GraphQuery);

	MetadataOverrides = Stamp.MetadataOverrides.CreateRuntime(
		GraphQuery,
		Evaluator);

	ensure(
		SplineCurves.Position.LoopKeyOffset == 0.f ||
		SplineCurves.Position.LoopKeyOffset == 1.f);

	for (int32 Index = 0; Index < SplineCurves.Position.Points.Num(); Index++)
	{
		ensure(SplineCurves.Position.Points[Index].InVal == Index);
		ensure(SplineCurves.Rotation.Points[Index].InVal == Index);
		ensure(SplineCurves.Scale.Points[Index].InVal == Index);

		ensure(SplineCurves.Position.Points[Index].InVal == Index);
		ensure(SplineCurves.Rotation.Points[Index].InVal == Index);
		ensure(SplineCurves.Scale.Points[Index].InVal == Index);
	}

	FVoxelHeightSplineStamp& MutableStamp = GetMutableStamp();
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

	for (FVoxelSplineSegment& Segment : Segments)
	{
		Segment.Bounds.Min.X -= MaxWidth;
		Segment.Bounds.Min.Y -= MaxWidth;

		Segment.Bounds.Max.X += MaxWidth;
		Segment.Bounds.Max.Y += MaxWidth;

		if (bRelativeHeightRange)
		{
			// Relative to previous stamps
			Segment.Bounds.Min.Z = HeightRange.Min;
			Segment.Bounds.Max.Z = HeightRange.Max;
		}
		else
		{
			Segment.Bounds.Min.Z += HeightRange.Min;
			Segment.Bounds.Max.Z += HeightRange.Max;
		}
	}

	return true;
}

FVoxelBox FVoxelHeightSplineStampRuntime::GetLocalBounds() const
{
	return FVoxelBox::FromBounds(GetChildren());
}

bool FVoxelHeightSplineStampRuntime::CanPartiallyInvalidate() const
{
	return true;
}

bool FVoxelHeightSplineStampRuntime::ShouldUseQueryPrevious() const
{
	return GetBlendMode() == EVoxelHeightBlendMode::Override;
}

bool FVoxelHeightSplineStampRuntime::HasRelativeHeightRange() const
{
	return bRelativeHeightRange;
}

TVoxelInlineArray<FVoxelBox, 1> FVoxelHeightSplineStampRuntime::GetChildren() const
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelInlineArray<FVoxelBox, 1> Result;
	Result.Reserve(Segments.Num());

	for (const FVoxelSplineSegment& Segment : Segments)
	{
		Result.Add_EnsureNoGrow(Segment.Bounds);
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelHeightSplineStampRuntime::TryToPartiallyInvalidate(
	const FVoxelStampRuntime& PreviousRuntime,
	TVoxelArray<FVoxelBox>& OutLocalBoundsToInvalidate) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelHeightSplineStampRuntime& Other = CastStructChecked<FVoxelHeightSplineStampRuntime>(PreviousRuntime);

	if (!Evaluator.Equals_SkipEnvironment(Other.Evaluator) ||
		// Technically unsafe as no AddReferencedObjects but fine for !=
		Stamp.ParameterOverrides != Other.Stamp.ParameterOverrides ||
		MaxWidth != Other.MaxWidth ||
		*MetadataOverrides != *Other.MetadataOverrides)
	{
		return false;
	}

	const TVoxelSet<FVoxelSplineSegment> OldSegments(Other.Segments);
	const TVoxelSet<FVoxelSplineSegment> NewSegments(Segments);

	for (const FVoxelSplineSegment& OldSegment : OldSegments)
	{
		if (!NewSegments.Contains(OldSegment))
		{
			OutLocalBoundsToInvalidate.Add(OldSegment.Bounds);
		}
	}

	for (const FVoxelSplineSegment& NewSegment : NewSegments)
	{
		if (!OldSegments.Contains(NewSegment))
		{
			OutLocalBoundsToInvalidate.Add(NewSegment.Bounds);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightSplineStampRuntime::Apply(
	const FVoxelHeightBulkQuery& Query,
	const FVoxelHeightTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER();

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

	{
		FVoxelGraphParameters::FSplineStamp& Parameter = GraphQuery.AddParameter<FVoxelGraphParameters::FSplineStamp>();
		Parameter.bIs2D = true;
		Parameter.bClosedLoop = bClosedLoop;
		Parameter.SplineLength = SplineLength;
		Parameter.ReparamStepsPerSegment = ReparamStepsPerSegment;
		Parameter.SplineCurves = &SplineCurves;
		Parameter.MetadataRuntime = MetadataRuntime.Get();
		Parameter.Segments = Segments;
	}

	FVoxelStampUtilities::ApplyHeights(
		*this,
		Query,
		StampToQuery,
		*Evaluator->HeightPin.GetSynchronous(GraphQuery),
		Positions);
}

void FVoxelHeightSplineStampRuntime::Apply(
	const FVoxelHeightSparseQuery& Query,
	const FVoxelHeightTransform& StampToQuery) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVoxelDoubleVector2DBuffer Positions = FVoxelStampUtilities::ComputePositions(
		Query,
		StampToQuery);

	FVoxelGraphContext Context = Evaluator.MakeContext(Query.Query.DependencyCollector);

	FVoxelGraphQueryImpl& GraphQuery = Context.MakeQuery();
	GraphQuery.AddParameter<FVoxelGraphParameters::FLOD>().Value = Query.Query.LOD;
	GraphQuery.AddParameter<FVoxelGraphParameters::FQuery>(Query.Query);
	GraphQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetLocalPosition(Positions);
	GraphQuery.AddParameter<FVoxelGraphParameters::FHeightQuery>(Query, StampToQuery);

	{
		FVoxelGraphParameters::FSplineStamp& Parameter = GraphQuery.AddParameter<FVoxelGraphParameters::FSplineStamp>();
		Parameter.bIs2D = true;
		Parameter.bClosedLoop = bClosedLoop;
		Parameter.SplineLength = SplineLength;
		Parameter.ReparamStepsPerSegment = ReparamStepsPerSegment;
		Parameter.SplineCurves = &SplineCurves;
		Parameter.MetadataRuntime = MetadataRuntime.Get();
		Parameter.Segments = Segments;
	}

	FVoxelStampUtilities::ComputeHeightStamp(
		*this,
		Query,
		StampToQuery,
		*MetadataOverrides,
		GraphQuery,
		Evaluator,
		Positions);
}