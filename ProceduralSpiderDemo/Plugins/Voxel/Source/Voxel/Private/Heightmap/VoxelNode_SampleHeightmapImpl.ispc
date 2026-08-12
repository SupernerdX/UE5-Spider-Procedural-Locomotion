// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

#define Type uint16
#include "VoxelNode_SampleHeightmapImpl.isph"
#undef Type

#define Type float
#include "VoxelNode_SampleHeightmapImpl.isph"
#undef Type

export void VoxelNode_SampleHeightmap_SampleWeightmap(
	const uniform float PositionX[],
	const uniform float PositionY[],
	uniform float OutWeights[],
	const uniform int32 Num,
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

	FOREACH(Index, 0, Num)
	{
		varying float2 Position = MakeFloat2(PositionX[Index], PositionY[Index]) * InvScaleXY + Offset;
		Position = Position * HeightmapToWeightmap;
		Position = clamp(Position, float2_one, SizeMinus2);
		
		coherent_if (
			Position.x < 1.f ||
			Position.y < 1.f ||
			Position.x > SizeMinus2.x ||
			Position.y > SizeMinus2.y)
		{
			OutWeights[Index] = NaNf;
			continue;
		}

		const varying double MinXf = floor(Position.x);
		const varying double MinYf = floor(Position.y);

		const varying float AlphaX = (float)clamp(Position.x - MinXf, 0.d, 1.d);
		const varying float AlphaY = (float)clamp(Position.y - MinYf, 0.d, 1.d);

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