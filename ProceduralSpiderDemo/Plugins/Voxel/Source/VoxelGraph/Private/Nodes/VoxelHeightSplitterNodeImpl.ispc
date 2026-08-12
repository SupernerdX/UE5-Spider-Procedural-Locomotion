// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"

FORCEINLINE float LoadFloatImpl(const uniform float Data[], const uniform bool bConstant, const varying int32 Index)
{
	return bConstant ? Data[0] : Data[Index];
}

#define Load_float(Name, Index) LoadFloatImpl(Array ## Name, bConst ## Name, Index)

#define Input_float(Name) \
	const uniform float Array ## Name[], \
	const uniform bool bConst ## Name

export void VoxelNode_HeightSplitter_FirstLayer(
	Input_float(Values),
	Input_float(CurrentHeights),
	Input_float(CurrentFalloffs),
	const uniform int32 Num,
	uniform float OutValues[])
{
	FOREACH(Index, 0, Num)
	{
		const varying float Value = Load_float(Values, Index);
		const varying float CurrentHeight = Load_float(CurrentHeights, Index);
		const varying float CurrentFalloff = Load_float(CurrentFalloffs, Index);

		OutValues[Index] = min(1.f, 1.f - SmoothStep(CurrentHeight - CurrentFalloff, CurrentHeight + CurrentFalloff, Value));
	}
}

export void VoxelNode_HeightSplitter_MiddleLayer(
	Input_float(Values),
	Input_float(PreviousHeights),
	Input_float(PreviousFalloffs),
	Input_float(CurrentHeights),
	Input_float(CurrentFalloffs),
	const uniform int32 Num,
	uniform float OutValues[])
{
	FOREACH(Index, 0, Num)
	{
		const varying float Value = Load_float(Values, Index);
		const varying float PreviousHeight = Load_float(PreviousHeights, Index);
		const varying float PreviousFalloff = Load_float(PreviousFalloffs, Index);
		const varying float CurrentHeight = Load_float(CurrentHeights, Index);
		const varying float CurrentFalloff = Load_float(CurrentFalloffs, Index);

		varying float Strength = 1.f;
		Strength = min(Strength, SmoothStep(PreviousHeight - PreviousFalloff, PreviousHeight + PreviousFalloff, Value));
		Strength = min(Strength, 1.f - SmoothStep(CurrentHeight - CurrentFalloff, CurrentHeight + CurrentFalloff, Value));

		OutValues[Index] = Strength;
	}
}

export void VoxelNode_HeightSplitter_LastLayer(
	Input_float(Values),
	Input_float(PreviousHeights),
	Input_float(PreviousFalloffs),
	const uniform int32 Num,
	uniform float OutValues[])
{
	FOREACH(Index, 0, Num)
	{
		const varying float Value = Load_float(Values, Index);
		const varying float PreviousHeight = Load_float(PreviousHeights, Index);
		const varying float PreviousFalloff = Load_float(PreviousFalloffs, Index);

		OutValues[Index] = min(1.f, SmoothStep(PreviousHeight - PreviousFalloff, PreviousHeight + PreviousFalloff, Value));
	}
}