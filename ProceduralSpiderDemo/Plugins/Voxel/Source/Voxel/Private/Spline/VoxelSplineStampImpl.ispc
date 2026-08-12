// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

// ReSharper disable CppCStyleCast

enum EInterpMode
{
	InterpMode_Constant,
	InterpMode_Linear,
	InterpMode_Cubic
};

struct FSegment
{
	float Key;
	float NextKey;
	float3 Value;
	float3 NextValue;
	float3 LeaveTangent; // LeaveTangent of this point
	float3 ArriveTangent; // ArriveTangent of the next point
	EInterpMode InterpMode; // InterpMode of this point
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE varying float FindNearestOnSegment2D(
	const varying float2 Position,
	const uniform FSegment Segment,
	varying float& OutSquaredDistance)
{
	const uniform float2 Point = MakeFloat2(Segment.Value.x, Segment.Value.y);
	const uniform float2 NextPoint = MakeFloat2(Segment.NextValue.x, Segment.NextValue.y);
	const uniform float2 LeaveTangent = MakeFloat2(Segment.LeaveTangent.x, Segment.LeaveTangent.y);
	const uniform float2 ArriveTangent = MakeFloat2(Segment.ArriveTangent.x, Segment.ArriveTangent.y);
	const uniform uint32 InterpMode = Segment.InterpMode;

	if (InterpMode == InterpMode_Constant)
	{
		const varying float Distance = distance_squared(Position, Point);
		const varying float NextDistance = distance_squared(Position, NextPoint);

		if (Distance < NextDistance)
		{
			OutSquaredDistance = Distance;
			return Segment.Key;
		}
		else
		{
			OutSquaredDistance = NextDistance;
			return Segment.NextKey;
		}
	}

	if (InterpMode == InterpMode_Linear)
	{
		const varying float A = dot(Position - Point, NextPoint - Point);
		const varying float B = distance_squared(Point, NextPoint);
		const varying float Alpha = clamp(A / B, 0.f, 1.f);

		OutSquaredDistance = distance_squared(lerp(Point, NextPoint, Alpha), Position);
		return Segment.Key + Alpha;
	}

	check(InterpMode == InterpMode_Cubic);

	varying float Moves[4];
	varying float Values[4];
	varying float SquaredDistances[4];

	// A first step starting from the 4 start coordinates with a large tolerance to find a good approximation in few iterations
	UNROLL
	for (uniform int32 Index = 0; Index < 4; Index++)
	{
		const uniform float StartValue = Index / 3.f;
		const uniform float MinValue = clamp(StartValue - 1.f / 3.f, 0.f, 1.f);
		const uniform float MaxValue = clamp(StartValue + 1.f / 3.f, 0.f, 1.f);

		varying float2 FoundPoint = CubicInterp(Point, LeaveTangent, NextPoint, ArriveTangent, StartValue);

		varying float Move;
		{
			const varying float2 Tangent = CubicInterpDerivative(Point, LeaveTangent, NextPoint, ArriveTangent, StartValue);
			Move = dot(Position - FoundPoint, Tangent) / length_squared(Tangent);
		}

		varying float Value = StartValue;

		for (uniform int32 Iteration = 1; Iteration < 20; Iteration++)
		{
			const varying float NewValue = clamp(Value + Move, MinValue, MaxValue);

			coherent_if (IsNearlyEqual(Value, NewValue, 1.e-2f))
			{
				// Has converged or is stuck in the clamp
				break;
			}

			Value = NewValue;

			FoundPoint = CubicInterp(Point, LeaveTangent, NextPoint, ArriveTangent, Value);

			const varying float2 Tangent = CubicInterpDerivative(Point, LeaveTangent, NextPoint, ArriveTangent, Value);
			Move = dot(Position - FoundPoint, Tangent) / length_squared(Tangent);
		}

		Moves[Index] = Move;
		Values[Index] = Value;
		SquaredDistances[Index] = distance_squared(FoundPoint, Position);
	}

	varying float Move = Moves[0];
	varying float Value = Values[0];
	varying float SquaredDistance = SquaredDistances[0];
	{
		UNROLL
		for (uniform int32 Index = 1; Index < 4; Index++)
		{
			if (SquaredDistances[Index] < SquaredDistance)
			{
				Move = Moves[Index];
				Value = Values[Index];
				SquaredDistance = SquaredDistances[Index];
			}
		}
	}

	// if needed, a last step starting from the best approximation with a small tolerance to be accurate is performed
	coherent_if (!IsNearlyZero(Move, 1.e-6f))
	{
		Value = clamp(Value + Move, 0.0f, 1.0f);

		varying float2 FoundPoint = CubicInterp(Point, LeaveTangent, NextPoint, ArriveTangent, Value);

		{
			const varying float2 Tangent = CubicInterpDerivative(Point, LeaveTangent, NextPoint, ArriveTangent, Value);
			Move = dot(Position - FoundPoint, Tangent) / length_squared(Tangent);
		}

		for (uniform int32 Iteration = 1; Iteration < 20; Iteration++)
		{
			const varying float NewValue = clamp(Value + Move, 0.0f, 1.0f);

			coherent_if (IsNearlyEqual(Value, NewValue, 1.e-5f))
			{
				// Has converged or is stuck in the clamp
				break;
			}

			FoundPoint = CubicInterp(Point, LeaveTangent, NextPoint, ArriveTangent, Value);

			const varying float2 Tangent = CubicInterpDerivative(Point, LeaveTangent, NextPoint, ArriveTangent, Value);
			Move = dot(Position - FoundPoint, Tangent) / length_squared(Tangent);
		}

		SquaredDistance = length_squared(FoundPoint - Position);
	}

	OutSquaredDistance = SquaredDistance;
	return Segment.Key + Value;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE varying float FindNearestOnSegment3D(
	const varying float3 Position,
	const uniform FSegment Segment,
	varying float& OutSquaredDistance)
{
	const uniform float3 Point = Segment.Value;
	const uniform float3 NextPoint = Segment.NextValue;
	const uniform float3 LeaveTangent = Segment.LeaveTangent;
	const uniform float3 ArriveTangent = Segment.ArriveTangent;
	const uniform uint32 InterpMode = Segment.InterpMode;

	if (InterpMode == InterpMode_Constant)
	{
		const varying float Distance = distance_squared(Position, Point);
		const varying float NextDistance = distance_squared(Position, NextPoint);

		if (Distance < NextDistance)
		{
			OutSquaredDistance = Distance;
			return Segment.Key;
		}
		else
		{
			OutSquaredDistance = NextDistance;
			return Segment.NextKey;
		}
	}

	if (InterpMode == InterpMode_Linear)
	{
		const varying float A = dot(Position - Point, NextPoint - Point);
		const varying float B = distance_squared(Point, NextPoint);
		const varying float Alpha = clamp(A / B, 0.f, 1.f);

		OutSquaredDistance = distance_squared(lerp(Point, NextPoint, Alpha), Position);
		return Segment.Key + Alpha;
	}

	check(InterpMode == InterpMode_Cubic);

	varying float Moves[4];
	varying float Values[4];
	varying float SquaredDistances[4];

	// A first step starting from the 4 start coordinates with a large tolerance to find a good approximation in few iterations
	UNROLL
	for (uniform int32 Index = 0; Index < 4; Index++)
	{
		const uniform float StartValue = Index / 3.f;
		const uniform float MinValue = clamp(StartValue - 1.f / 3.f, 0.f, 1.f);
		const uniform float MaxValue = clamp(StartValue + 1.f / 3.f, 0.f, 1.f);

		varying float3 FoundPoint = CubicInterp(Point, LeaveTangent, NextPoint, ArriveTangent, StartValue);

		varying float Move;
		{
			const varying float3 Tangent = CubicInterpDerivative(Point, LeaveTangent, NextPoint, ArriveTangent, StartValue);
			Move = dot(Position - FoundPoint, Tangent) / length_squared(Tangent);
		}

		varying float Value = StartValue;

		for (uniform int32 Iteration = 1; Iteration < 20; Iteration++)
		{
			const varying float NewValue = clamp(Value + Move, MinValue, MaxValue);

			coherent_if (IsNearlyEqual(Value, NewValue, 1.e-2f))
			{
				// Has converged or is stuck in the clamp
				break;
			}

			Value = NewValue;

			FoundPoint = CubicInterp(Point, LeaveTangent, NextPoint, ArriveTangent, Value);

			const varying float3 Tangent = CubicInterpDerivative(Point, LeaveTangent, NextPoint, ArriveTangent, Value);
			Move = dot(Position - FoundPoint, Tangent) / length_squared(Tangent);
		}

		Moves[Index] = Move;
		Values[Index] = Value;
		SquaredDistances[Index] = distance_squared(FoundPoint, Position);
	}

	varying float Move = Moves[0];
	varying float Value = Values[0];
	varying float SquaredDistance = SquaredDistances[0];
	{
		UNROLL
		for (uniform int32 Index = 1; Index < 4; Index++)
		{
			if (SquaredDistances[Index] < SquaredDistance)
			{
				Move = Moves[Index];
				Value = Values[Index];
				SquaredDistance = SquaredDistances[Index];
			}
		}
	}

	// if needed, a last step starting from the best approximation with a small tolerance to be accurate is performed
	coherent_if (!IsNearlyZero(Move, 1.e-6f))
	{
		Value = clamp(Value + Move, 0.0f, 1.0f);

		varying float3 FoundPoint = CubicInterp(Point, LeaveTangent, NextPoint, ArriveTangent, Value);

		{
			const varying float3 Tangent = CubicInterpDerivative(Point, LeaveTangent, NextPoint, ArriveTangent, Value);
			Move = dot(Position - FoundPoint, Tangent) / length_squared(Tangent);
		}

		for (uniform int32 Iteration = 1; Iteration < 20; Iteration++)
		{
			const varying float NewValue = clamp(Value + Move, 0.0f, 1.0f);

			coherent_if (IsNearlyEqual(Value, NewValue, 1.e-5f))
			{
				// Has converged or is stuck in the clamp
				break;
			}

			FoundPoint = CubicInterp(Point, LeaveTangent, NextPoint, ArriveTangent, Value);

			const varying float3 Tangent = CubicInterpDerivative(Point, LeaveTangent, NextPoint, ArriveTangent, Value);
			Move = dot(Position - FoundPoint, Tangent) / length_squared(Tangent);
		}

		SquaredDistance = length_squared(FoundPoint - Position);
	}

	OutSquaredDistance = SquaredDistance;
	return Segment.Key + Value;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE varying float3 EvalPosition(
	const uniform FSegment Segment,
	const varying float Alpha)
{
	if (Segment.InterpMode == InterpMode_Constant)
	{
		return Segment.Value;
	}

	if (Segment.InterpMode == InterpMode_Linear)
	{
		return lerp(Segment.Value, Segment.NextValue, Alpha);
	}

	check(Segment.InterpMode == InterpMode_Cubic);

	return CubicInterp(
		Segment.Value,
		Segment.LeaveTangent,
		Segment.NextValue,
		Segment.ArriveTangent,
		Alpha);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelSplineStamp_FindNearest2D(
	uniform float OutKeys[],
	const uniform float PositionsX[],
	const uniform float PositionsY[],
	const uniform int32 Num,
	const uniform FSegment Segments[],
	const uniform int32 NumSegments)
{
	check(NumSegments > 0);

	FOREACH(Index, 0, Num)
	{
		const varying float2 Position = MakeFloat2(
			PositionsX[Index],
			PositionsY[Index]);

		varying float BestSquaredDistance;
		varying float Key = FindNearestOnSegment2D(
			Position,
			Segments[0],
			BestSquaredDistance);

		for (uniform int32 SegmentIndex = 1; SegmentIndex < NumSegments; SegmentIndex++)
		{
			varying float LocalSquaredDistance;
			const varying float LocalKey = FindNearestOnSegment2D(
				Position,
				Segments[SegmentIndex],
				LocalSquaredDistance);

			if (LocalSquaredDistance < BestSquaredDistance)
			{
				BestSquaredDistance = LocalSquaredDistance;
				Key = LocalKey;
			}
		}

		OutKeys[Index] = Key;
	}
}

export void VoxelSplineStamp_FindNearest3D(
	uniform float OutKeys[],
	const uniform float PositionsX[],
	const uniform float PositionsY[],
	const uniform float PositionsZ[],
	const uniform int32 Num,
	const uniform FSegment Segments[],
	const uniform int32 NumSegments)
{
	check(NumSegments > 0);

	FOREACH(Index, 0, Num)
	{
		const varying float3 Position = MakeFloat3(
			PositionsX[Index],
			PositionsY[Index],
			PositionsZ[Index]);

		varying float BestSquaredDistance;
		varying float Key = FindNearestOnSegment3D(
			Position,
			Segments[0],
			BestSquaredDistance);

		for (uniform int32 SegmentIndex = 1; SegmentIndex < NumSegments; SegmentIndex++)
		{
			varying float LocalSquaredDistance;
			const varying float LocalKey = FindNearestOnSegment3D(
				Position,
				Segments[SegmentIndex],
				LocalSquaredDistance);

			if (LocalSquaredDistance < BestSquaredDistance)
			{
				BestSquaredDistance = LocalSquaredDistance;
				Key = LocalKey;
			}
		}

		OutKeys[Index] = Key;
	}
}