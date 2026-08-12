// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

export void VoxelBufferMathUtilities_Min(
	const uniform float Buffer0[], const uniform bool bIsConstBuffer0,
	const uniform float Buffer1[], const uniform bool bIsConstBuffer1,
	uniform float Result[],
	const uniform int32 Num)
{
	if (!bIsConstBuffer0 &&
		!bIsConstBuffer1)
	{
		FOREACH(Index, 0, Num)
		{
			Result[Index] = min(
				Buffer0[Index],
				Buffer1[Index]);
		}
		return;
	}

	FOREACH(Index, 0, Num)
	{
		Result[Index] = min(
			bIsConstBuffer0 ? Buffer0[0] : Buffer0[Index],
			bIsConstBuffer1 ? Buffer1[0] : Buffer1[Index]);
	}
}

export void VoxelBufferMathUtilities_Max(
	const uniform float Buffer0[], const uniform bool bIsConstBuffer0,
	const uniform float Buffer1[], const uniform bool bIsConstBuffer1,
	uniform float Result[],
	const uniform int32 Num)
{
	if (!bIsConstBuffer0 &&
		!bIsConstBuffer1)
	{
		FOREACH(Index, 0, Num)
		{
			Result[Index] = max(
				Buffer0[Index],
				Buffer1[Index]);
		}
		return;
	}

	FOREACH(Index, 0, Num)
	{
		Result[Index] = max(
			bIsConstBuffer0 ? Buffer0[0] : Buffer0[Index],
			bIsConstBuffer1 ? Buffer1[0] : Buffer1[Index]);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferMathUtilities_Min8(
	const uniform float Buffer0[], const uniform bool bIsConstBuffer0,
	const uniform float Buffer1[], const uniform bool bIsConstBuffer1,
	const uniform float Buffer2[], const uniform bool bIsConstBuffer2,
	const uniform float Buffer3[], const uniform bool bIsConstBuffer3,
	const uniform float Buffer4[], const uniform bool bIsConstBuffer4,
	const uniform float Buffer5[], const uniform bool bIsConstBuffer5,
	const uniform float Buffer6[], const uniform bool bIsConstBuffer6,
	const uniform float Buffer7[], const uniform bool bIsConstBuffer7,
	uniform float Result[],
	const uniform int32 Num)
{
	if (!bIsConstBuffer0 &&
		!bIsConstBuffer1 &&
		!bIsConstBuffer2 &&
		!bIsConstBuffer3 &&
		!bIsConstBuffer4 &&
		!bIsConstBuffer5 &&
		!bIsConstBuffer6 &&
		!bIsConstBuffer7)
	{
		FOREACH(Index, 0, Num)
		{
			Result[Index] =
				min(Buffer0[Index],
				min(Buffer1[Index],
				min(Buffer2[Index],
				min(Buffer3[Index],
				min(Buffer4[Index],
				min(Buffer5[Index],
				min(Buffer6[Index], Buffer7[Index])))))));
		}
		return;
	}

	FOREACH(Index, 0, Num)
	{
		Result[Index] =
			min(bIsConstBuffer0 ? Buffer0[0] : Buffer0[Index],
			min(bIsConstBuffer1 ? Buffer1[0] : Buffer1[Index],
			min(bIsConstBuffer2 ? Buffer2[0] : Buffer2[Index],
			min(bIsConstBuffer3 ? Buffer3[0] : Buffer3[Index],
			min(bIsConstBuffer4 ? Buffer4[0] : Buffer4[Index],
			min(bIsConstBuffer5 ? Buffer5[0] : Buffer5[Index],
			min(bIsConstBuffer6 ? Buffer6[0] : Buffer6[Index], bIsConstBuffer7 ? Buffer7[0] : Buffer7[Index])))))));
	}
}

export void VoxelBufferMathUtilities_Max8(
	const uniform float Buffer0[], const uniform bool bIsConstBuffer0,
	const uniform float Buffer1[], const uniform bool bIsConstBuffer1,
	const uniform float Buffer2[], const uniform bool bIsConstBuffer2,
	const uniform float Buffer3[], const uniform bool bIsConstBuffer3,
	const uniform float Buffer4[], const uniform bool bIsConstBuffer4,
	const uniform float Buffer5[], const uniform bool bIsConstBuffer5,
	const uniform float Buffer6[], const uniform bool bIsConstBuffer6,
	const uniform float Buffer7[], const uniform bool bIsConstBuffer7,
	uniform float Result[],
	const uniform int32 Num)
{
	if (!bIsConstBuffer0 &&
		!bIsConstBuffer1 &&
		!bIsConstBuffer2 &&
		!bIsConstBuffer3 &&
		!bIsConstBuffer4 &&
		!bIsConstBuffer5 &&
		!bIsConstBuffer6 &&
		!bIsConstBuffer7)
	{
		FOREACH(Index, 0, Num)
		{
			Result[Index] =
				max(Buffer0[Index],
				max(Buffer1[Index],
				max(Buffer2[Index],
				max(Buffer3[Index],
				max(Buffer4[Index],
				max(Buffer5[Index],
				max(Buffer6[Index], Buffer7[Index])))))));
		}
		return;
	}

	FOREACH(Index, 0, Num)
	{
		Result[Index] =
			max(bIsConstBuffer0 ? Buffer0[0] : Buffer0[Index],
			max(bIsConstBuffer1 ? Buffer1[0] : Buffer1[Index],
			max(bIsConstBuffer2 ? Buffer2[0] : Buffer2[Index],
			max(bIsConstBuffer3 ? Buffer3[0] : Buffer3[Index],
			max(bIsConstBuffer4 ? Buffer4[0] : Buffer4[Index],
			max(bIsConstBuffer5 ? Buffer5[0] : Buffer5[Index],
			max(bIsConstBuffer6 ? Buffer6[0] : Buffer6[Index], bIsConstBuffer7 ? Buffer7[0] : Buffer7[Index])))))));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferMathUtilities_Add_Float(
	const uniform float A[],
	const uniform bool bConstantA,
	const uniform float B[],
	const uniform bool bConstantB,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Result[Index] = (bConstantA ? A[0] : A[Index]) + (bConstantB ? B[0] : B[Index]);
	}
}

export void VoxelBufferMathUtilities_Add_Double(
	const uniform double A[],
	const uniform bool bConstantA,
	const uniform double B[],
	const uniform bool bConstantB,
	uniform double Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Result[Index] = (bConstantA ? A[0] : A[Index]) + (bConstantB ? B[0] : B[Index]);
	}
}

export void VoxelBufferMathUtilities_Multiply(
	const uniform float A[],
	const uniform bool bConstantA,
	const uniform float B[],
	const uniform bool bConstantB,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Result[Index] = (bConstantA ? A[0] : A[Index]) * (bConstantB ? B[0] : B[Index]);
	}
}

export void VoxelBufferMathUtilities_Less(
	const uniform float A[],
	const uniform bool bConstantA,
	const uniform float B[],
	const uniform bool bConstantB,
	uniform uint8 Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Result[Index] = (bConstantA ? A[0] : A[Index]) < (bConstantB ? B[0] : B[Index]) ? 1 : 0;
	}
}

export void VoxelBufferMathUtilities_IsFinite(
	const uniform float Values[],
	uniform uint8 Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Result[Index] = IsFinite(Values[Index]) ? 1 : 0;
	}
}

export void VoxelBufferMathUtilities_Alpha(
	const uniform float A[],
	const uniform bool bConstantA,
	const uniform float B[],
	const uniform bool bConstantB,
	const uniform float Alpha[],
	const uniform bool bConstantAlpha,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Result[Index] = lerp(
			bConstantA ? A[0] : A[Index],
			bConstantB ? B[0] : B[Index],
			bConstantAlpha ? Alpha[0] : Alpha[Index]);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferMathUtilities_LerpRotation(
	const uniform float AX[],
	const uniform bool bConstantAX,
	const uniform float AY[],
	const uniform bool bConstantAY,
	const uniform float AZ[],
	const uniform bool bConstantAZ,
	const uniform float AW[],
	const uniform bool bConstantAW,
	const uniform float BX[],
	const uniform bool bConstantBX,
	const uniform float BY[],
	const uniform bool bConstantBY,
	const uniform float BZ[],
	const uniform bool bConstantBZ,
	const uniform float BW[],
	const uniform bool bConstantBW,
	const uniform float Alpha[],
	const uniform bool bConstantAlpha,
	uniform float X[],
	uniform float Y[],
	uniform float Z[],
	uniform float W[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float A0 = bConstantAX ? AX[0] : AX[Index];
		const varying float A1 = bConstantAY ? AY[0] : AY[Index];
		const varying float A2 = bConstantAZ ? AZ[0] : AZ[Index];
		const varying float A3 = bConstantAW ? AW[0] : AW[Index];

		const varying float B0 = bConstantBX ? BX[0] : BX[Index];
		const varying float B1 = bConstantBY ? BY[0] : BY[Index];
		const varying float B2 = bConstantBZ ? BZ[0] : BZ[Index];
		const varying float B3 = bConstantBW ? BW[0] : BW[Index];

		const varying float A = bConstantAlpha ? Alpha[0] : Alpha[Index];

		// Get cosine of angle between quats.
		varying float RawCos = 
			A0 * B0 +
			A1 * B1 +
			A2 * B2 +
			A3 * B3;

		// Unaligned quats - compensate, results in taking shorter route.
		const varying float Sign = RawCos >= 0.f ? 1.f : -1.f;
		RawCos *= Sign;

		varying float Scale0 = 1.f - A;
		varying float Scale1 = A * Sign;

		if (RawCos < 0.9999f)
		{
			const varying float Omega = acos(RawCos);
			const varying float InvSin = 1.f / sin(Omega);
			Scale0 = sin(Scale0 * Omega) * InvSin;
			Scale1 = sin(Scale1 * Omega) * InvSin;
		}

		X[Index] = Scale0 * A0 + Scale1 * B0;
		Y[Index] = Scale0 * A1 + Scale1 * B1;
		Z[Index] = Scale0 * A2 + Scale1 * B2;
		W[Index] = Scale0 * A3 + Scale1 * B3;

		// Normalize Quaternion
		const varying float SquareSum =
			X[Index] * X[Index] +
			Y[Index] * Y[Index] +
			Z[Index] * Z[Index] +
			W[Index] * W[Index];
	
		if (SquareSum >= SMALL_NUMBER)
		{
			const varying float Scale = InvSqrt(SquareSum);
	
			X[Index] *= Scale; 
			Y[Index] *= Scale; 
			Z[Index] *= Scale;
			W[Index] *= Scale;
		}
		else
		{
			X[Index] = 0.f;
			Y[Index] = 0.f;
			Z[Index] = 0.f;
			W[Index] = 1.f;
		}
	}
}

export void VoxelBufferMathUtilities_Combine(
	const uniform float AX[],
	const uniform bool bConstantAX,
	const uniform float AY[],
	const uniform bool bConstantAY,
	const uniform float AZ[],
	const uniform bool bConstantAZ,
	const uniform float AW[],
	const uniform bool bConstantAW,
	const uniform float BX[],
	const uniform bool bConstantBX,
	const uniform float BY[],
	const uniform bool bConstantBY,
	const uniform float BZ[],
	const uniform bool bConstantBZ,
	const uniform float BW[],
	const uniform bool bConstantBW,
	uniform float X[],
	uniform float Y[],
	uniform float Z[],
	uniform float W[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float A0 = bConstantAX ? AX[0] : AX[Index];
		const varying float A1 = bConstantAY ? AY[0] : AY[Index];
		const varying float A2 = bConstantAZ ? AZ[0] : AZ[Index];
		const varying float A3 = bConstantAW ? AW[0] : AW[Index];

		const varying float B0 = bConstantBX ? BX[0] : BX[Index];
		const varying float B1 = bConstantBY ? BY[0] : BY[Index];
		const varying float B2 = bConstantBZ ? BZ[0] : BZ[Index];
		const varying float B3 = bConstantBW ? BW[0] : BW[Index];

		X[Index] = A3 * B0 + A0 * B3 + A1 * B2 - A2 * B1;
		Y[Index] = A3 * B1 - A0 * B2 + A1 * B3 + A2 * B0;
		Z[Index] = A3 * B2 + A0 * B1 - A1 * B0 + A2 * B3;
		W[Index] = A3 * B3 - A0 * B0 - A1 * B1 - A2 * B2;
	}
}

export void VoxelBufferMathUtilities_RotateVector(
	const uniform float QX[],
	const uniform bool bConstantQX,
	const uniform float QY[],
	const uniform bool bConstantQY,
	const uniform float QZ[],
	const uniform bool bConstantQZ,
	const uniform float QW[],
	const uniform bool bConstantQW,
	const uniform float VX[],
	const uniform bool bConstantVX,
	const uniform float VY[],
	const uniform bool bConstantVY,
	const uniform float VZ[],
	const uniform bool bConstantVZ,
	uniform float X[],
	uniform float Y[],
	uniform float Z[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float Q0 = bConstantQX ? QX[0] : QX[Index];
		const varying float Q1 = bConstantQY ? QY[0] : QY[Index];
		const varying float Q2 = bConstantQZ ? QZ[0] : QZ[Index];
		const varying float Q3 = bConstantQW ? QW[0] : QW[Index];
		const float4 Quat = MakeFloat4(Q0, Q1, Q2, Q3);

		const varying float V0 = bConstantVX ? VX[0] : VX[Index];
		const varying float V1 = bConstantVY ? VY[0] : VY[Index];
		const varying float V2 = bConstantVZ ? VZ[0] : VZ[Index];
		const float3 Vector = MakeFloat3(V0, V1, V2);

		float3 Result = RotateVector(Vector, Quat);

		X[Index] = Result.x;
		Y[Index] = Result.y;
		Z[Index] = Result.z;
	}
}

export void VoxelBufferMathUtilities_UnrotateVector(
	const uniform float QX[],
	const uniform bool bConstantQX,
	const uniform float QY[],
	const uniform bool bConstantQY,
	const uniform float QZ[],
	const uniform bool bConstantQZ,
	const uniform float QW[],
	const uniform bool bConstantQW,
	const uniform float VX[],
	const uniform bool bConstantVX,
	const uniform float VY[],
	const uniform bool bConstantVY,
	const uniform float VZ[],
	const uniform bool bConstantVZ,
	uniform float X[],
	uniform float Y[],
	uniform float Z[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float Q0 = bConstantQX ? QX[0] : QX[Index];
		const varying float Q1 = bConstantQY ? QY[0] : QY[Index];
		const varying float Q2 = bConstantQZ ? QZ[0] : QZ[Index];
		const varying float Q3 = bConstantQW ? QW[0] : QW[Index];
		const float4 Quat = MakeFloat4(Q0, Q1, Q2, Q3);

		const varying float V0 = bConstantVX ? VX[0] : VX[Index];
		const varying float V1 = bConstantVY ? VY[0] : VY[Index];
		const varying float V2 = bConstantVZ ? VZ[0] : VZ[Index];
		const float3 Vector = MakeFloat3(V0, V1, V2);

		float3 Result = UnrotateVector(Vector, Quat);

		X[Index] = Result.x;
		Y[Index] = Result.y;
		Z[Index] = Result.z;
	}
}

export void VoxelBufferMathUtilities_RotateVector2D(
	const uniform float VX[],
	const uniform bool bConstantVX,
	const uniform float VY[],
	const uniform bool bConstantVY,
	const uniform float AngleDeg[],
	const uniform bool bConstantAngleDeg,
	uniform float X[],
	uniform float Y[],
	const uniform int32 Num)
{
	const uniform float DEG_TO_RAD = (PI / 180.f);

	FOREACH(Index, 0, Num)
	{
		const varying float AngleDeg0 = bConstantAngleDeg ? AngleDeg[0] : AngleDeg[Index];

		varying float Sin;
		varying float Cos;
		sincos(AngleDeg0 * DEG_TO_RAD, &Sin, &Cos);

		const varying float V0 = bConstantVX ? VX[0] : VX[Index];
		const varying float V1 = bConstantVY ? VY[0] : VY[Index];
		const float2 Vector = MakeFloat2(V0, V1);

		X[Index] = Cos * Vector.x - Sin * Vector.y;
		Y[Index] = Sin * Vector.x + Cos * Vector.y;
	}
}

export void VoxelBufferMathUtilities_MakeEulerFromQuaternion(
	const uniform float AX[],
	const uniform bool bConstantAX,
	const uniform float AY[],
	const uniform bool bConstantAY,
	const uniform float AZ[],
	const uniform bool bConstantAZ,
	const uniform float AW[],
	const uniform bool bConstantAW,
	uniform float X[],
	uniform float Y[],
	uniform float Z[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float A0 = bConstantAX ? AX[0] : AX[Index];
		const varying float A1 = bConstantAY ? AY[0] : AY[Index];
		const varying float A2 = bConstantAZ ? AZ[0] : AZ[Index];
		const varying float A3 = bConstantAW ? AW[0] : AW[Index];
		const float4 Quat = MakeFloat4(A0, A1, A2, A3);

		float Pitch;
		float Yaw;
		float Roll;
		MakeEulerFromQuaternion(Quat, Pitch, Yaw, Roll);

		X[Index] = Pitch;
		Y[Index] = Yaw;
		Z[Index] = Roll;
	}
}

export void VoxelBufferMathUtilities_MakeQuaternionFromEuler(
	const uniform float AX[],
	const uniform bool bConstantAX,
	const uniform float AY[],
	const uniform bool bConstantAY,
	const uniform float AZ[],
	const uniform bool bConstantAZ,
	uniform float X[],
	uniform float Y[],
	uniform float Z[],
	uniform float W[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float A0 = bConstantAX ? AX[0] : AX[Index];
		const varying float A1 = bConstantAY ? AY[0] : AY[Index];
		const varying float A2 = bConstantAZ ? AZ[0] : AZ[Index];

		float4 ReturnValue = MakeQuaternionFromEuler(A0, A1, A2);

		X[Index] = ReturnValue.x;
		Y[Index] = ReturnValue.y;
		Z[Index] = ReturnValue.z;
		W[Index] = ReturnValue.w;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferMathUtilities_Length_float2(
	const uniform float VectorX[],
	const uniform bool bConstantVectorX,
	const uniform float VectorY[],
	const uniform bool bConstantVectorY,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];

		Result[Index] = length(MakeFloat2(X, Y));
	}
}

export void VoxelBufferMathUtilities_Length_float3(
	const uniform float VectorX[],
	const uniform bool bConstantVectorX,
	const uniform float VectorY[],
	const uniform bool bConstantVectorY,
	const uniform float VectorZ[],
	const uniform bool bConstantVectorZ,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];
		const varying float Z = bConstantVectorZ ? VectorZ[0] : VectorZ[Index];

		Result[Index] = length(MakeFloat3(X, Y, Z));
	}
}

export void VoxelBufferMathUtilities_Length_int2(
	const uniform int32 VectorX[],
	const uniform bool bConstantVectorX,
	const uniform int32 VectorY[],
	const uniform bool bConstantVectorY,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];

		Result[Index] = length(MakeFloat2(X, Y));
	}
}

export void VoxelBufferMathUtilities_Length_int3(
	const uniform int32 VectorX[],
	const uniform bool bConstantVectorX,
	const uniform int32 VectorY[],
	const uniform bool bConstantVectorY,
	const uniform int32 VectorZ[],
	const uniform bool bConstantVectorZ,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];
		const varying float Z = bConstantVectorZ ? VectorZ[0] : VectorZ[Index];

		Result[Index] = length(MakeFloat3(X, Y, Z));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferMathUtilities_SquaredLength_float2(
	const uniform float VectorX[],
	const uniform bool bConstantVectorX,
	const uniform float VectorY[],
	const uniform bool bConstantVectorY,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];

		Result[Index] = X * X + Y * Y;
	}
}

export void VoxelBufferMathUtilities_SquaredLength_float3(
	const uniform float VectorX[],
	const uniform bool bConstantVectorX,
	const uniform float VectorY[],
	const uniform bool bConstantVectorY,
	const uniform float VectorZ[],
	const uniform bool bConstantVectorZ,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];
		const varying float Z = bConstantVectorZ ? VectorZ[0] : VectorZ[Index];

		Result[Index] = X * X + Y * Y + Z * Z;
	}
}

export void VoxelBufferMathUtilities_SquaredLength_int2(
	const uniform int32 VectorX[],
	const uniform bool bConstantVectorX,
	const uniform int32 VectorY[],
	const uniform bool bConstantVectorY,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];

		Result[Index] = X * X + Y * Y;
	}
}

export void VoxelBufferMathUtilities_SquaredLength_int3(
	const uniform int32 VectorX[],
	const uniform bool bConstantVectorX,
	const uniform int32 VectorY[],
	const uniform bool bConstantVectorY,
	const uniform int32 VectorZ[],
	const uniform bool bConstantVectorZ,
	uniform float Result[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];
		const varying float Z = bConstantVectorZ ? VectorZ[0] : VectorZ[Index];

		Result[Index] = X * X + Y * Y + Z * Z;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferMathUtilities_Normalize_float2(
	const uniform float VectorX[],
	const uniform bool bConstantVectorX,
	const uniform float VectorY[],
	const uniform bool bConstantVectorY,
	uniform float ResultX[],
	uniform float ResultY[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];

		const float2 Normalized = normalize(MakeFloat2(X, Y));

		ResultX[Index] = Normalized.x;
		ResultY[Index] = Normalized.y;
	}
}

export void VoxelBufferMathUtilities_Normalize_float3(
	const uniform float VectorX[],
	const uniform bool bConstantVectorX,
	const uniform float VectorY[],
	const uniform bool bConstantVectorY,
	const uniform float VectorZ[],
	const uniform bool bConstantVectorZ,
	uniform float ResultX[],
	uniform float ResultY[],
	uniform float ResultZ[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];
		const varying float Z = bConstantVectorZ ? VectorZ[0] : VectorZ[Index];

		const float3 Normalized = normalize(MakeFloat3(X, Y, Z));

		ResultX[Index] = Normalized.x;
		ResultY[Index] = Normalized.y;
		ResultZ[Index] = Normalized.z;
	}
}

export void VoxelBufferMathUtilities_Normalize_int2(
	const uniform int32 VectorX[],
	const uniform bool bConstantVectorX,
	const uniform int32 VectorY[],
	const uniform bool bConstantVectorY,
	uniform float ResultX[],
	uniform float ResultY[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];

		const float2 Normalized = normalize(MakeFloat2(X, Y));

		ResultX[Index] = Normalized.x;
		ResultY[Index] = Normalized.y;
	}
}

export void VoxelBufferMathUtilities_Normalize_int3(
	const uniform int32 VectorX[],
	const uniform bool bConstantVectorX,
	const uniform int32 VectorY[],
	const uniform bool bConstantVectorY,
	const uniform int32 VectorZ[],
	const uniform bool bConstantVectorZ,
	uniform float ResultX[],
	uniform float ResultY[],
	uniform float ResultZ[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];
		const varying float Z = bConstantVectorZ ? VectorZ[0] : VectorZ[Index];

		const float3 Normalized = normalize(MakeFloat3(X, Y, Z));

		ResultX[Index] = Normalized.x;
		ResultY[Index] = Normalized.y;
		ResultZ[Index] = Normalized.z;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelBufferMathUtilities_MakeQuaternionFromZ(
	const uniform float VectorX[],
	const uniform bool bConstantVectorX,
	const uniform float VectorY[],
	const uniform bool bConstantVectorY,
	const uniform float VectorZ[],
	const uniform bool bConstantVectorZ,
	uniform float ResultX[],
	uniform float ResultY[],
	uniform float ResultZ[],
	uniform float ResultW[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstantVectorX ? VectorX[0] : VectorX[Index];
		const varying float Y = bConstantVectorY ? VectorY[0] : VectorY[Index];
		const varying float Z = bConstantVectorZ ? VectorZ[0] : VectorZ[Index];

		const float4 Result = MakeQuaternionFromZ(MakeFloat3(X, Y, Z));

		ResultX[Index] = Result.x;
		ResultY[Index] = Result.y;
		ResultZ[Index] = Result.z;
		ResultW[Index] = Result.w;
	}
}