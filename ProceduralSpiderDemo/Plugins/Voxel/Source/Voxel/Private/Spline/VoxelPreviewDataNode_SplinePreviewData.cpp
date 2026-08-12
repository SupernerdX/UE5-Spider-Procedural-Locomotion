// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Spline/VoxelPreviewDataNode_SplinePreviewData.h"
#include "Spline/VoxelSplineMetadata.h"
#include "Spline/VoxelHeightSplineGraph.h"
#include "Spline/VoxelSplineGraphParameters.h"

namespace FVoxelGraphParameters
{
	struct FSplineDataScope : FUniformParameter
	{
		FSplineCurves SplineCurves;
		FVoxelSplineMetadataRuntime MetadataRuntime;
	};
}

void FVoxelPreviewDataNode_SplinePreviewData::Query_WithPosition(
	const FVoxelGraphQueryImpl& Query,
	FVoxelGraphQueryImpl& OutQuery) const
{
	const TSharedRef<const FVoxelVectorBuffer> Positions = PositionsPin.GetSynchronous(Query);
	const TSharedRef<const FVoxelVectorBuffer> ArriveTangents = ArriveTangentsPin.GetSynchronous(Query);
	const TSharedRef<const FVoxelVectorBuffer> LeaveTangents = LeaveTangentsPin.GetSynchronous(Query);
	const TSharedRef<const FVoxelQuaternionBuffer> Rotations = RotationsPin.GetSynchronous(Query);
	const TSharedRef<const FVoxelVectorBuffer> Scales = ScalesPin.GetSynchronous(Query);
	const TSharedRef<const FVoxelByteBuffer> Types = TypesPin.GetSynchronous(Query);
	
	if (Positions->Num() < 2)
	{
		return;
	}

	const int32 NumPositions = Positions->Num();
	const auto Same = [&](const int32 Num) -> bool
	{
		return
			Num == 0 ||
			Num == 1 ||
			Num == NumPositions;
	};
	if (!Same(ArriveTangents->Num()) ||
		!Same(LeaveTangents->Num()) ||
		!Same(Rotations->Num()) ||
		!Same(Scales->Num()) ||
		!Same(Types->Num()))
	{
		RaiseBufferError();
		return;
	}

	const auto GetValue = [](int32 Index, const auto& Buffer, const auto& Default)
	{
		if (Buffer->Num() == 0)
		{
			return Default;
		}

		return (*Buffer)[Index];
	};

	FVoxelGraphParameters::FSplineDataScope& ParameterDataScope = OutQuery.AddParameter<FVoxelGraphParameters::FSplineDataScope>();
	for (int32 Index = 0; Index < Positions->Num(); Index++)
	{
		const FVector3f& Position = (*Positions)[Index];
		const FVector3f ArriveTangent = GetValue(Index, ArriveTangents, Position);
		const FVector3f LeaveTangent = GetValue(Index, LeaveTangents, Position);
		const FQuat4f Rotation = GetValue(Index, Rotations, FQuat4f::Identity);
		const FVector3f Scale = GetValue(Index, Scales, FVector3f::OneVector);
		const EInterpCurveMode Type = INLINE_LAMBDA -> EInterpCurveMode
		{
			if (Types->Num() == 0)
			{
				return CIM_CurveAuto;
			}

			return EInterpCurveMode((*Types)[Index]);
		};
		ParameterDataScope.SplineCurves.Position.Points.Add(FInterpCurvePoint<FVector>(Index, FVector(Position), FVector(ArriveTangent), FVector(LeaveTangent), Type));
		ParameterDataScope.SplineCurves.Rotation.Points.Add(FInterpCurvePoint<FQuat>(Index, FQuat(Rotation)));
		ParameterDataScope.SplineCurves.Scale.Points.Add(FInterpCurvePoint<FVector>(Index, FVector(Scale)));
	}

	FVoxelGraphParameters::FSplineStamp& Parameter = OutQuery.AddParameter<FVoxelGraphParameters::FSplineStamp>();
	Parameter.bIs2D = OutQuery.Context.Environment.Owner.CanBeCastedTo<UVoxelHeightSplineGraph>();
	Parameter.bClosedLoop = ClosedLoopPin.GetSynchronous(Query);
	Parameter.ReparamStepsPerSegment = ReparamStepsPerSegmentPin.GetSynchronous(Query);
	Parameter.SplineLength = ParameterDataScope.SplineCurves.GetSplineLength();
	Parameter.SplineCurves = &ParameterDataScope.SplineCurves;
	Parameter.MetadataRuntime = &ParameterDataScope.MetadataRuntime;
	Parameter.Segments = FVoxelSplineSegment::Create(ParameterDataScope.SplineCurves, ParameterDataScope.MetadataRuntime);
}