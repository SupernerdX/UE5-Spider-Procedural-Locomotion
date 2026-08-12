// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

export void VoxelMathFunctionLibrary_MakeNormal(
	const uniform float NormalX[],
	const uniform bool bConstNormalX,
	const uniform float NormalY[],
	const uniform bool bConstNormalY,
	const uniform float NormalZ[],
	const uniform bool bConstNormalZ,
	uniform FVoxelOctahedron OutNormal[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float X = bConstNormalX ? NormalX[0] : NormalX[Index];
		const varying float Y = bConstNormalY ? NormalY[0] : NormalY[Index];
		const varying float Z = bConstNormalZ ? NormalZ[0] : NormalZ[Index];

		IGNORE_PERF_WARNING
		OutNormal[Index] = MakeOctahedron(UnitVectorToOctahedron(normalize(MakeFloat3(X, Y, Z))));
	}
}

export void VoxelMathFunctionLibrary_BreakNormal(
	const uniform FVoxelOctahedron Normal[],
	uniform float OutNormalX[],
	uniform float OutNormalY[],
	uniform float OutNormalZ[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		IGNORE_PERF_WARNING
		const varying float3 Result = OctahedronToUnitVector(Normal[Index]);

		OutNormalX[Index] = Result.x;
		OutNormalY[Index] = Result.y;
		OutNormalZ[Index] = Result.z;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelMathFunctionLibrary_SmoothMin(
	const uniform float DistancesA[],
	const uniform bool bConstDistanceA,
	const uniform float DistancesB[],
	const uniform bool bConstDistanceB,
	const uniform float Smoothnesses[],
	const uniform bool bConstSmoothness,
	const uniform int32 Num,
	uniform float OutAlpha[],
	uniform float OutDistance[])
{
	FOREACH(Index, 0, Num)
	{
		const varying float DistanceA = bConstDistanceA ? DistancesA[0] : DistancesA[Index];
		const varying float DistanceB = bConstDistanceB ? DistancesB[0] : DistancesB[Index];
		const varying float Smoothness = bConstSmoothness ? Smoothnesses[0] : Smoothnesses[Index];

		if (isnan(DistanceA))
		{
			OutAlpha[Index] = 1.f;
			OutDistance[Index] = DistanceB;
			continue;
		}

		if (isnan(DistanceB))
		{
			OutAlpha[Index] = 0.f;
			OutDistance[Index] = DistanceA;
			continue;
		}

		const varying float OneMinusAlpha = clamp(0.5f + 0.5f * SmoothHelper(DistanceB - DistanceA, Smoothness), 0.f, 1.f);
		const varying float Alpha = 1.f - OneMinusAlpha;

		OutAlpha[Index] = Alpha;
		OutDistance[Index] = lerp(DistanceA, DistanceB, Alpha) - Smoothness * Alpha * OneMinusAlpha;
	}
}

export void VoxelMathFunctionLibrary_SmoothMax(
	const uniform float DistancesA[],
	const uniform bool bConstDistanceA,
	const uniform float DistancesB[],
	const uniform bool bConstDistanceB,
	const uniform float Smoothnesses[],
	const uniform bool bConstSmoothness,
	const uniform int32 Num,
	uniform float OutAlpha[],
	uniform float OutDistance[])
{
	FOREACH(Index, 0, Num)
	{
		const varying float DistanceA = bConstDistanceA ? DistancesA[0] : DistancesA[Index];
		const varying float DistanceB = bConstDistanceB ? DistancesB[0] : DistancesB[Index];
		const varying float Smoothness = bConstSmoothness ? Smoothnesses[0] : Smoothnesses[Index];

		if (isnan(DistanceA))
		{
			OutAlpha[Index] = 1.f;
			OutDistance[Index] = DistanceB;
			continue;
		}

		if (isnan(DistanceB))
		{
			OutAlpha[Index] = 0.f;
			OutDistance[Index] = DistanceA;
			continue;
		}

		const varying float OneMinusAlpha = clamp(0.5f - 0.5f * SmoothHelper(DistanceB - DistanceA, Smoothness), 0.f, 1.f);
		const varying float Alpha = 1.f - OneMinusAlpha;

		OutAlpha[Index] = Alpha;
		OutDistance[Index] = lerp(DistanceA, DistanceB, Alpha) + Smoothness * Alpha * OneMinusAlpha;
	}
}

export void VoxelMathFunctionLibrary_SmoothSubtraction(
	const uniform float DistancesA[],
	const uniform bool bConstDistanceA,
	const uniform float DistancesB[],
	const uniform bool bConstDistanceB,
	const uniform float Smoothnesses[],
	const uniform bool bConstSmoothness,
	const uniform int32 Num,
	uniform float OutAlpha[],
	uniform float OutDistance[])
{
	FOREACH(Index, 0, Num)
	{
		const varying float DistanceA = bConstDistanceA ? DistancesA[0] : DistancesA[Index];
		const varying float DistanceB = -(bConstDistanceB ? DistancesB[0] : DistancesB[Index]);
		const varying float Smoothness = bConstSmoothness ? Smoothnesses[0] : Smoothnesses[Index];
		
		if (isnan(DistanceA))
		{
			OutAlpha[Index] = 1.f;
			OutDistance[Index] = DistanceB;
			continue;
		}

		if (isnan(DistanceB))
		{
			OutAlpha[Index] = 0.f;
			OutDistance[Index] = DistanceA;
			continue;
		}

		const varying float OneMinusAlpha = clamp(0.5f - 0.5f * SmoothHelper(DistanceB - DistanceA, Smoothness), 0.f, 1.f);
		const varying float Alpha = 1.f - OneMinusAlpha;

		OutAlpha[Index] = Alpha;
		OutDistance[Index] = lerp(DistanceA, DistanceB, Alpha) + Smoothness * Alpha * OneMinusAlpha;
	}
}