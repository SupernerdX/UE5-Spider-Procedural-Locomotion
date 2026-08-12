// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Spline/VoxelSplineStampFunctionLibrary.h"
#include "Spline/VoxelSplineMetadata.h"
#include "Spline/VoxelSplineGraphParameters.h"
#include "Components/SplineComponent.h"
#include "VoxelSplineStampImpl.ispc.generated.h"

FVoxelFloatBuffer UVoxelSplineStampFunctionLibrary::GetClosestSplineKeyGeneric(const FVoxelVectorBuffer& Position) const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	if (Parameter->bIs2D)
	{
		FVoxelVector2DBuffer Position2D;
		Position2D.X = Position.X;
		Position2D.Y = Position.Y;
		return GetClosestSplineKey2D(Position2D);
	}
	else
	{
		return GetClosestSplineKey3D(Position);
	}
}

FVoxelFloatBuffer UVoxelSplineStampFunctionLibrary::GetClosestSplineKey2D(const FVoxelVector2DBuffer& InPosition) const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	FVoxelVector2DBuffer Position = InPosition;
	Position.ExpandConstants(Position.Num());

	const FVoxelBox2D Bounds = FVoxelBox2D::FromPositions(
		Position.X.View(),
		Position.Y.View());

	TVoxelInlineArray<ispc::FSegment, 32> LocalSegments;
	{
		VOXEL_SCOPE_COUNTER("Segments");

		for (const FVoxelSplineSegment& Segment : Parameter->Segments)
		{
			if (!FVoxelBox2D(Segment.Bounds).Intersects(Bounds))
			{
				continue;
			}

			LocalSegments.Add(Segment.Segment);
		}
	}

	if (LocalSegments.Num() == 0)
	{
		// Try to return a sane value

		for (const FVoxelSplineSegment& Segment : Parameter->Segments)
		{
			LocalSegments.Add(Segment.Segment);
		}
	}
	ensure(LocalSegments.Num() > 0);

	FVoxelFloatBuffer Result;
	Result.Allocate(Position.Num());

	VOXEL_SCOPE_COUNTER_FORMAT("VoxelSplineStamp_FindNearest2D Num=%d NumSegments=%d", Position.Num(), LocalSegments.Num());

	ispc::VoxelSplineStamp_FindNearest2D(
		Result.GetData(),
		Position.X.GetData(),
		Position.Y.GetData(),
		Position.Num(),
		LocalSegments.GetData(),
		LocalSegments.Num());

	return Result;
}

FVoxelFloatBuffer UVoxelSplineStampFunctionLibrary::GetClosestSplineKey3D(const FVoxelVectorBuffer& InPosition) const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	FVoxelVectorBuffer Position = InPosition;
	Position.ExpandConstants(Position.Num());

	const FVoxelBox Bounds = FVoxelBox::FromPositions(
		Position.X.View(),
		Position.Y.View(),
		Position.Z.View());

	TVoxelInlineArray<ispc::FSegment, 32> LocalSegments;
	{
		VOXEL_SCOPE_COUNTER("Segments");

		for (const FVoxelSplineSegment& Segment : Parameter->Segments)
		{
			if (!Segment.Bounds.Intersects(Bounds))
			{
				continue;
			}

			LocalSegments.Add(Segment.Segment);
		}
	}

	if (LocalSegments.Num() == 0)
	{
		// Try to return a sane value

		for (const FVoxelSplineSegment& Segment : Parameter->Segments)
		{
			LocalSegments.Add(Segment.Segment);
		}
	}
	ensure(LocalSegments.Num() > 0);

	FVoxelFloatBuffer Result;
	Result.Allocate(Position.Num());

	VOXEL_SCOPE_COUNTER_FORMAT("VoxelSplineStamp_FindNearest3D Num=%d NumSegments=%d", Position.Num(), LocalSegments.Num());

	ispc::VoxelSplineStamp_FindNearest3D(
		Result.GetData(),
		Position.X.GetData(),
		Position.Y.GetData(),
		Position.Z.GetData(),
		Position.Num(),
		LocalSegments.GetData(),
		LocalSegments.Num());

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float UVoxelSplineStampFunctionLibrary::GetSplineLength() const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	return Parameter->SplineLength;
}

float UVoxelSplineStampFunctionLibrary::GetLastSplineKey() const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return 0.f;
	}

	const int32 NumPoints = Parameter->SplineCurves->Position.Points.Num();
	return float(NumPoints - 1);
}

FVoxelVectorBuffer UVoxelSplineStampFunctionLibrary::GetPositionAlongSpline(const FVoxelFloatBuffer& SplineKey) const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	const FSplineCurves& SplineCurves = *Parameter->SplineCurves;

	VOXEL_FUNCTION_COUNTER_NUM(SplineKey.Num());

	FVoxelVectorBuffer Result;
	Result.Allocate(SplineKey.Num());

	for (int32 Index = 0; Index < SplineKey.Num(); Index++)
	{
		const float Key = SplineKey[Index];
		Result.Set(Index, FVector3f(SplineCurves.Position.Eval(Key)));
	}

	return Result;
}

FVoxelFloatBuffer UVoxelSplineStampFunctionLibrary::GetHeightAlongSpline(const FVoxelFloatBuffer& SplineKey) const
{
	return GetPositionAlongSpline(SplineKey).Z;
}

FVoxelFloatBuffer UVoxelSplineStampFunctionLibrary::GetDistanceAlongSpline(const FVoxelFloatBuffer& SplineKey) const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	const FSplineCurves& SplineCurves = *Parameter->SplineCurves;

	const int32 NumPoints = SplineCurves.Position.Points.Num();
	const int32 NumSegments = Parameter->bClosedLoop ? NumPoints : NumPoints - 1;

	FVoxelFloatBuffer Result;
	Result.Allocate(SplineKey.Num());

	for (int32 Index = 0; Index < SplineKey.Num(); Index++)
	{
		const float Key = SplineKey[Index];
		if (Key < 0)
		{
			Result.Set(Index, 0.f);
			continue;
		}

		if (Key >= NumSegments)
		{
			Result.Set(Index, SplineCurves.GetSplineLength());
			continue;
		}

		const int32 PointIndex = FMath::FloorToInt(Key);
		const float Fraction = Key - PointIndex;
		const int32 ReparamPointIndex = PointIndex * Parameter->ReparamStepsPerSegment;
		const float Distance = SplineCurves.ReparamTable.Points[ReparamPointIndex].InVal;

		Result.Set(Index, Distance + SplineCurves.GetSegmentLength(PointIndex, Fraction, Parameter->bClosedLoop, FVector::OneVector));
	}

	return Result;
}

FVoxelTransformBuffer UVoxelSplineStampFunctionLibrary::GetTransformAlongSpline(const FVoxelFloatBuffer& SplineKey) const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	const FSplineCurves& SplineCurves = *Parameter->SplineCurves;

	FVoxelTransformBuffer Result;
	Result.Allocate(SplineKey.Num());

	for (int32 Index = 0; Index < SplineKey.Num(); Index++)
	{
		const float Key = SplineKey[Index];

		const FVector Position = SplineCurves.Position.Eval(Key);

		FQuat Rotation = SplineCurves.Rotation.Eval(Key);
		{
			Rotation.Normalize();

			const FVector Direction = SplineCurves.Position.EvalDerivative(Key, FVector::ZeroVector).GetSafeNormal();
			const FVector UpVector = Rotation.RotateVector(FVector::UpVector);

			Rotation = FRotationMatrix::MakeFromXZ(Direction, UpVector).ToQuat();
		}

		const FVector Scale = SplineCurves.Scale.Eval(Key, FVector(1.f));

		Result.Set(Index, FTransform3f(
			FQuat4f(Rotation),
			FVector3f(Position),
			FVector3f(Scale)));
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer UVoxelSplineStampFunctionLibrary::GetFloatSplineParameterValue(
	const FVoxelFloatSplineParameter& InParameter,
	const FVoxelFloatBuffer& SplineKey) const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	const TVoxelArray<float>* ValuesPtr = Parameter->MetadataRuntime->GuidToFloatValues.Find(InParameter.ParameterGuid);
	if (!ensureVoxelSlow(ValuesPtr))
	{
		return DefaultBuffer;
	}

	const TConstVoxelArrayView<float> Values = *ValuesPtr;

	FVoxelFloatBuffer Result;
	Result.Allocate(SplineKey.Num());

	for (int32 Index = 0; Index < SplineKey.Num(); Index++)
	{
		const float Key = SplineKey[Index];
		int32 KeyMin = FMath::FloorToInt(Key);
		int32 KeyMax = FMath::CeilToInt(Key);
		ensureVoxelSlow(Values.IsValidIndex(KeyMin));
		ensureVoxelSlow(Values.IsValidIndex(KeyMax));
		KeyMin = FMath::Clamp(KeyMin, 0, Values.Num() - 1);
		KeyMax = FMath::Clamp(KeyMax, 0, Values.Num() - 1);

		const float ValueMin = Values[KeyMin];
		const float ValueMax = Values[KeyMax];

		Result.Set(Index, FMath::Lerp(ValueMin, ValueMax, FMath::Clamp(Key - KeyMin, 0.f, 1.f)));
	}

	return Result;
}

FVoxelVector2DBuffer UVoxelSplineStampFunctionLibrary::GetVector2DSplineParameterValue(
	const FVoxelVector2DSplineParameter& InParameter,
	const FVoxelFloatBuffer& SplineKey) const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	const TVoxelArray<FVector2f>* ValuesPtr = Parameter->MetadataRuntime->GuidToVector2DValues.Find(InParameter.ParameterGuid);
	if (!ensureVoxelSlow(ValuesPtr))
	{
		return DefaultBuffer;
	}

	const TConstVoxelArrayView<FVector2f> Values = *ValuesPtr;

	FVoxelVector2DBuffer Result;
	Result.Allocate(SplineKey.Num());

	for (int32 Index = 0; Index < SplineKey.Num(); Index++)
	{
		const float Key = SplineKey[Index];
		int32 KeyMin = FMath::FloorToInt(Key);
		int32 KeyMax = FMath::CeilToInt(Key);
		ensureVoxelSlow(Values.IsValidIndex(KeyMin));
		ensureVoxelSlow(Values.IsValidIndex(KeyMax));
		KeyMin = FMath::Clamp(KeyMin, 0, Values.Num() - 1);
		KeyMax = FMath::Clamp(KeyMax, 0, Values.Num() - 1);

		const FVector2f ValueMin = Values[KeyMin];
		const FVector2f ValueMax = Values[KeyMax];
		const FVector2f Value = FMath::Lerp(ValueMin, ValueMax, FMath::Clamp(Key - KeyMin, 0.f, 1.f));

		Result.Set(Index, Value);
	}

	return Result;
}

FVoxelVectorBuffer UVoxelSplineStampFunctionLibrary::GetVectorSplineParameterValue(
	const FVoxelVectorSplineParameter& InParameter,
	const FVoxelFloatBuffer& SplineKey) const
{
	const FVoxelGraphParameters::FSplineStamp* Parameter = Query->FindParameter<FVoxelGraphParameters::FSplineStamp>();
	if (!Parameter)
	{
		if (Query.IsPreview())
		{
			VOXEL_MESSAGE(Error, "{0}: No Spline Preview Data node found in Editor Graph", this);
		}
		else
		{
			VOXEL_MESSAGE(Error, "{0}: No spline data", this);
		}
		return DefaultBuffer;
	}

	const TVoxelArray<FVector3f>* ValuesPtr = Parameter->MetadataRuntime->GuidToVectorValues.Find(InParameter.ParameterGuid);
	if (!ensureVoxelSlow(ValuesPtr))
	{
		return DefaultBuffer;
	}

	const TConstVoxelArrayView<FVector3f> Values = *ValuesPtr;

	FVoxelVectorBuffer Result;
	Result.Allocate(SplineKey.Num());

	for (int32 Index = 0; Index < SplineKey.Num(); Index++)
	{
		const float Key = SplineKey[Index];
		int32 KeyMin = FMath::FloorToInt(Key);
		int32 KeyMax = FMath::CeilToInt(Key);
		ensureVoxelSlow(Values.IsValidIndex(KeyMin));
		ensureVoxelSlow(Values.IsValidIndex(KeyMax));
		KeyMin = FMath::Clamp(KeyMin, 0, Values.Num() - 1);
		KeyMax = FMath::Clamp(KeyMax, 0, Values.Num() - 1);

		const FVector3f ValueMin = Values[KeyMin];
		const FVector3f ValueMax = Values[KeyMax];
		const FVector3f Value = FMath::Lerp(ValueMin, ValueMax, FMath::Clamp(Key - KeyMin, 0.f, 1.f));

		Result.Set(Index, Value);
	}

	return Result;
}
