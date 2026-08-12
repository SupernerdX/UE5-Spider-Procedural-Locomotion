// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNoiseNodesImpl.isph"

FORCEINLINE bool LoadBoolImpl(const uniform bool Data[], const uniform bool bConstant, const varying int32 Index)
{
	return bConstant ? Data[0] : Data[Index];
}
FORCEINLINE float LoadFloatImpl(const uniform float Data[], const uniform bool bConstant, const varying int32 Index)
{
	return bConstant ? Data[0] : Data[Index];
}
FORCEINLINE int32 LoadInt32Impl(const uniform int32 Data[], const uniform bool bConstant, const varying int32 Index)
{
	return bConstant ? Data[0] : Data[Index];
}

#define Load_float(Name, Index) LoadFloatImpl(Array ## Name, bConst ## Name, Index)
#define Store_float(Name, Index, Value) Array ## Name[Index] = Value;

#define Load_float2(Name, Index) MakeFloat2(Load_float(Name ## _X, Index), Load_float(Name ## _Y, Index))

#define Load_float3(Name, Index) MakeFloat3(Load_float(Name ## _X, Index), Load_float(Name ## _Y, Index), Load_float(Name ## _Z, Index))

#define Input_Type(Name, Type) \
	const uniform Type Array ## Name[], \
	const uniform bool bConst ## Name

#define Output_Type(Name, Type) \
	uniform Type Array ## Name[]

#define Input_float(Name) Input_Type(Name, float)
#define Output_float(Name) Output_Type(Name, float)

#define Input_float2(Name) \
	Input_float(Name ## _X), \
	Input_float(Name ## _Y)

#define Input_float3(Name) \
	Input_float(Name ## _X), \
	Input_float(Name ## _Y), \
	Input_float(Name ## _Z)

#define Output_float2(Name) \
	Output_float(Name ## _X), \
	Output_float(Name ## _Y)

export void VoxelNode_DomainWarp2D(
	Input_float2(Position),
	Input_float(Amplitude),
	Input_float(FeatureScale),
	Input_float(Lacunarity),
	Input_float(Gain),
	Input_float(WeightedStrength),
	const uniform float FractalBound,
	const uniform bool bConstFractalBound,
	const uniform int32 NumOctaves,
	const uniform int32 InSeed,
	uniform float ReturnValueX[],
	uniform float ReturnValueY[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float Gain = Load_float(Gain, Index);

		varying float FractalBounding = FractalBound;
		if (!bConstFractalBound)
		{
			varying float AmpFractal = 1.f;
			varying float Amp = Gain;
			for (uniform int32 OctaveIndex = 1; OctaveIndex < NumOctaves; OctaveIndex++)
			{
				AmpFractal += Amp;
				Amp *= Gain;
			}
			FractalBounding = 1.f / AmpFractal;
		}

		const varying float Lacunarity = Load_float(Lacunarity, Index);
		const varying float WeightedStrength = Load_float(WeightedStrength, Index);

		varying float Sum = 0.f;
		varying float AmplitudeSum = 0.f;

		varying float Amplitude = FractalBounding * Load_float(Amplitude, Index);

		const varying float2 Position = Load_float2(Position, Index);
		varying float2 ScaledPosition = Position / Load_float(FeatureScale, Index);
		varying float2 WarpedPosition = Position;
		uniform int32 Seed = InSeed;

		varying float Strength = DomainWarp2D(Seed, Amplitude, ScaledPosition, &WarpedPosition);

		//UNROLL_N(16)
		for (uniform int32 OctaveIndex = 1; OctaveIndex < NumOctaves; OctaveIndex++)
		{
			ScaledPosition = ScaledPosition * Lacunarity;
			Amplitude *= lerp(1.f, 1.f - Strength, WeightedStrength) * Gain;

			Seed = (Seed * 196314165) + 907633515;

			Strength = DomainWarp2D(Seed, Amplitude, ScaledPosition, &WarpedPosition);
		}

		ReturnValueX[Index] = WarpedPosition.x;
		ReturnValueY[Index] = WarpedPosition.y;
	}
}

export void VoxelNode_DomainWarp3D(
	Input_float3(Position),
	Input_float(Amplitude),
	Input_float(FeatureScale),
	Input_float(Lacunarity),
	Input_float(Gain),
	Input_float(WeightedStrength),
	const uniform float FractalBound,
	const uniform bool bConstFractalBound,
	const uniform int32 NumOctaves,
	const uniform int32 InSeed,
	uniform float ReturnValueX[],
	uniform float ReturnValueY[],
	uniform float ReturnValueZ[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying float Gain = Load_float(Gain, Index);

		varying float FractalBounding = FractalBound;
		if (!bConstFractalBound)
		{
			varying float AmpFractal = 1.f;
			varying float Amp = Gain;
			for (uniform int32 OctaveIndex = 1; OctaveIndex < NumOctaves; OctaveIndex++)
			{
				AmpFractal += Amp;
				Amp *= Gain;
			}
			FractalBounding = 1.f / AmpFractal;
		}

		const varying float Lacunarity = Load_float(Lacunarity, Index);
		const varying float WeightedStrength = Load_float(WeightedStrength, Index);

		varying float Sum = 0.f;
		varying float AmplitudeSum = 0.f;

		varying float Amplitude = FractalBounding * Load_float(Amplitude, Index);

		const varying float3 Position = Load_float3(Position, Index);
		varying float3 ScaledPosition = Position / Load_float(FeatureScale, Index);
		varying float3 WarpedPosition = Position;
		uniform int32 Seed = InSeed;

		varying float Strength = DomainWarp3D(Seed, Amplitude, ScaledPosition, &WarpedPosition);

		//UNROLL_N(16)
		for (uniform int32 OctaveIndex = 1; OctaveIndex < NumOctaves; OctaveIndex++)
		{
			ScaledPosition = ScaledPosition * Lacunarity;
			Amplitude *= lerp(1.f, 1.f - Strength, WeightedStrength) * Gain;

			Seed = (Seed * 196314165) + 907633515;

			Strength = DomainWarp3D(Seed, Amplitude, ScaledPosition, &WarpedPosition);
		}

		ReturnValueX[Index] = WarpedPosition.x;
		ReturnValueY[Index] = WarpedPosition.y;
		ReturnValueZ[Index] = WarpedPosition.z;
	}
}