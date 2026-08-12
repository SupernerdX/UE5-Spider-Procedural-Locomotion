// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"
#include "VoxelStampQueryImpl.isph"
#include "VoxelStampTransformImpl.isph"

// ReSharper disable CppCStyleCast

FORCEINLINE varying float GetTransformedHeight(
	const uniform FVoxelHeightTransform& StampToQuery,
	const uniform EVoxelHeightBlendMode BlendMode,
	const uniform bool bApplyOnVoid,
	const uniform float Smoothness,
	const uniform float ScaleZ,
	const uniform float OffsetZ,
	const varying float2 Position,
	const varying float OldHeight,
	varying float NewHeight,
	varying float& OutAlpha)
{
	NewHeight = NewHeight * ScaleZ + OffsetZ;
	NewHeight = TransformHeight(StampToQuery, NewHeight, Position);

	switch (BlendMode)
	{
	default: VOXEL_ASSUME(false);
	case HeightBlendMode_Max:
	{
		if (intbits(OldHeight) == NaNf_uint)
		{
			if (bApplyOnVoid)
			{
				OutAlpha = 1.f;
				return ClampHeight(StampToQuery, NewHeight);
			}
			else
			{
				OutAlpha = 0.f;
				return OldHeight;
			}
		}
		else
		{
			OutAlpha = GetSmoothMaxAlpha(OldHeight, NewHeight, Smoothness);
			NewHeight = SmoothMax(OldHeight, NewHeight, Smoothness);

			// OldHeight might be bigger if we got clamped
			return max(OldHeight, ClampHeight(StampToQuery, NewHeight));
		}
	}
	case HeightBlendMode_Min:
	{
		if (intbits(OldHeight) == NaNf_uint)
		{
			if (bApplyOnVoid)
			{
				OutAlpha = 1.f;
				return ClampHeight(StampToQuery, NewHeight);
			}
			else
			{
				OutAlpha = 0.f;
				return OldHeight;
			}
		}
		else
		{
			OutAlpha = GetSmoothMinAlpha(OldHeight, NewHeight, Smoothness);
			NewHeight = SmoothMin(OldHeight, NewHeight, Smoothness);

			// OldHeight might be smaller if we got clamped
			return min(OldHeight, ClampHeight(StampToQuery, NewHeight));
		}
	}
	case HeightBlendMode_Override:
	{
		OutAlpha = 1.f;
		return ClampHeight(StampToQuery, NewHeight);
	}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define Type uint16
#include "VoxelHeightmapStampImpl.isph"
#undef Type

#define Type float
#include "VoxelHeightmapStampImpl.isph"
#undef Type

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelHeightmapStamp_SampleWeightmap(
	const uniform FVoxelHeightSparseQuery& Query,
	const uniform FVoxelHeightTransform& StampToQuery,
	uniform float OutWeights[],
	const uniform float Alphas[],
	const uniform float Strength,
	const uniform bool bUseBicubic,
	const uniform float ScaleXY,
	const uniform float2& HeightmapToWeightmap,
	const uniform int32 SizeX,
	const uniform int32 SizeY,
	const uniform uint8 Weightmap[])
{
	const uniform float2 SizeMinus2 = MakeFloat2(
		SizeX - 2.f,
		SizeY - 2.f);

	const uniform float InvScaleXY = 1.f / ScaleXY;
	const uniform float2 Offset = MakeFloat2(SizeX, SizeY) / 2.f;

	FOREACH(Index, 0, Query.Num)
	{
		const varying float Alpha = Alphas[Index];
		if (all(Alpha == 0))
		{
			// Fast path
			OutWeights[Index] = 0;
			continue;
		}

		const varying double2 WorldPosition = MakeDouble2(
			Query.PositionsX[Index],
			Query.PositionsY[Index]);

		varying float2 LocalPosition = InverseTransform(StampToQuery, WorldPosition);
		LocalPosition = LocalPosition * InvScaleXY + Offset;
		LocalPosition = LocalPosition * HeightmapToWeightmap;
		LocalPosition = clamp(LocalPosition, float2_one, SizeMinus2);

		const varying double MinXf = floor(LocalPosition.x);
		const varying double MinYf = floor(LocalPosition.y);

		const varying float AlphaX = (float)clamp(LocalPosition.x - MinXf, 0.d, 1.d);
		const varying float AlphaY = (float)clamp(LocalPosition.y - MinYf, 0.d, 1.d);

		const varying int32 MinX = (int32)MinXf;
		const varying int32 MinY = (int32)MinYf;

		varying float Weight;
		if (bUseBicubic)
		{
			 Weight = BicubicInterpolation(
				IGNORE_PERF_WARNING
				Weightmap[(MinX - 1) + SizeX * (MinY - 1)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 0) + SizeX * (MinY - 1)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 1) + SizeX * (MinY - 1)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 2) + SizeX * (MinY - 1)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX - 1) + SizeX * (MinY + 0)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 0) + SizeX * (MinY + 0)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 1) + SizeX * (MinY + 0)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 2) + SizeX * (MinY + 0)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX - 1) + SizeX * (MinY + 1)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 0) + SizeX * (MinY + 1)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 1) + SizeX * (MinY + 1)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 2) + SizeX * (MinY + 1)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX - 1) + SizeX * (MinY + 2)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 0) + SizeX * (MinY + 2)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 1) + SizeX * (MinY + 2)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 2) + SizeX * (MinY + 2)],
				AlphaX,
				AlphaY);
		}
		else
		{
			Weight = BilinearInterpolation(
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 0) + SizeX * (MinY + 0)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 1) + SizeX * (MinY + 0)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 0) + SizeX * (MinY + 1)],
				IGNORE_PERF_WARNING
				Weightmap[(MinX + 1) + SizeX * (MinY + 1)],
				AlphaX,
				AlphaY);
		}

		OutWeights[Index] = clamp(Weight * Strength / MAX_uint8, 0.f, 1.f);
	}
}

export void VoxelHeightmapStamp_BlendMetadata(
	const uniform FVoxelHeightSparseQuery& Query,
	uniform float InOutIndirectValues[],
	const uniform float Weights[],
	const uniform float Alphas[])
{
	FOREACH(Index, 0, Query.Num)
	{
		if (Query.Indirection)
		{
			const varying int32 IndirectIndex = Query.Indirection[Index];

			IGNORE_PERF_WARNING
			InOutIndirectValues[IndirectIndex] = lerp(InOutIndirectValues[IndirectIndex], Weights[Index], Alphas[Index]);
		}
		else
		{
			InOutIndirectValues[Index] = lerp(InOutIndirectValues[Index], Weights[Index], Alphas[Index]);
		}
	}
}