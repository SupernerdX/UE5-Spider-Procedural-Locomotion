// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Spline/VoxelSplineStamp.h"
#include "Spline/VoxelSplineMetadata.h"

TVoxelArray<FVoxelSplineSegment> FVoxelSplineSegment::Create(
	const FSplineCurves& SplineCurves,
	const FVoxelSplineMetadataRuntime& MetadataRuntime)
{
	VOXEL_FUNCTION_COUNTER();

	const TConstVoxelArrayView<FInterpCurvePoint<FVector>> Points = SplineCurves.Position.Points;
	ensure(Points.Num() >= 2);

	TVoxelArray<FVoxelSplineSegment> Segments;

	if (SplineCurves.Position.bIsLooped)
	{
		FVoxelUtilities::SetNum(Segments, Points.Num());
	}
	else
	{
		FVoxelUtilities::SetNum(Segments, Points.Num() - 1);
	}

	for (int32 Index = 0; Index < Segments.Num(); Index++)
	{
		checkVoxelSlow(Points[Index].InVal == Index);

		const int32 NextIndex = Index + 1 < Points.Num() ? Index + 1 : 0;

		FVector Min = FVector(MAX_dbl);
		FVector Max = FVector(-MAX_dbl);
		CurveVectorFindIntervalBounds(
			SplineCurves.Position.Points[Index],
			SplineCurves.Position.Points[NextIndex],
			Min,
			Max);

		FVoxelSplineSegment& Segment = Segments[Index];
		Segment.Bounds = FVoxelBox
		{
			Min,
			Max
		};

		Segment.Segment = ispc::FSegment
		{
			float(Index),
			float(NextIndex),
			GetISPCValue(FVector3f(Points[Index].OutVal)),
			GetISPCValue(FVector3f(Points[NextIndex].OutVal)),
			GetISPCValue(FVector3f(Points[Index].LeaveTangent)),
			GetISPCValue(FVector3f(Points[NextIndex].ArriveTangent)),
			INLINE_LAMBDA
			{
				switch (Points[Index].InterpMode)
				{
				case CIM_Constant: return ispc::InterpMode_Constant;
				case CIM_Linear: return ispc::InterpMode_Linear;
				default: return ispc::InterpMode_Cubic;
				}
			}
		};

		Segment.RotationA = SplineCurves.Rotation.Points[Index];
		Segment.RotationB = SplineCurves.Rotation.Points[NextIndex];
		Segment.PositionA = SplineCurves.Position.Points[Index];
		Segment.PositionB = SplineCurves.Position.Points[NextIndex];
		Segment.ScaleA = SplineCurves.Scale.Points[Index];
		Segment.ScaleB = SplineCurves.Scale.Points[NextIndex];

		Segment.GuidToFloatValue.Reserve(MetadataRuntime.GuidToFloatValues.Num());
		Segment.GuidToVector2DValue.Reserve(MetadataRuntime.GuidToVector2DValues.Num());
		Segment.GuidToVectorValue.Reserve(MetadataRuntime.GuidToVectorValues.Num());

		for (const auto& It : MetadataRuntime.GuidToFloatValues)
		{
			const TVoxelArray<float>& Values = It.Value;
			if (!ensure(Values.IsValidIndex(Index)) ||
				!ensure(Values.IsValidIndex(NextIndex)))
			{
				continue;
			}

			Segment.GuidToFloatValue.Add_EnsureNew(It.Key) = { Values[Index], Values[NextIndex] };
		}

		for (const auto& It : MetadataRuntime.GuidToVector2DValues)
		{
			const TVoxelArray<FVector2f>& Values = It.Value;
			if (!ensure(Values.IsValidIndex(Index)) ||
				!ensure(Values.IsValidIndex(NextIndex)))
			{
				continue;
			}

			Segment.GuidToVector2DValue.Add_EnsureNew(It.Key) = { Values[Index], Values[NextIndex] };
		}

		for (const auto& It : MetadataRuntime.GuidToVectorValues)
		{
			const TVoxelArray<FVector3f>& Values = It.Value;
			if (!ensure(Values.IsValidIndex(Index)) ||
				!ensure(Values.IsValidIndex(NextIndex)))
			{
				continue;
			}

			Segment.GuidToVectorValue.Add_EnsureNew(It.Key) = { Values[Index], Values[NextIndex] };
		}
	}

	return Segments;
}