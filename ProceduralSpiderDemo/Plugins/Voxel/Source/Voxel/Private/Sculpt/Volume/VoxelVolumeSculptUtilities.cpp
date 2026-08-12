// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeSculptUtilities.h"

void FVoxelVolumeSculptUtilities::DiffDistances(
	const TConstVoxelArrayView<float> Distances,
	const TConstVoxelArrayView<float> PreviousDistances,
	const FIntVector& Size,
	TVoxelArray<float>& AdditiveDistances,
	TVoxelArray<float>& SubtractiveDistances)
{
	VOXEL_FUNCTION_COUNTER_NUM(Distances.Num());
	check(Distances.Num() == Size.X * Size.Y * Size.Z);
	check(PreviousDistances.Num() == Size.X * Size.Y * Size.Z);

	FVoxelUtilities::SetNumFast(AdditiveDistances, Distances.Num());
	FVoxelUtilities::SetNumFast(SubtractiveDistances, Distances.Num());

	Voxel::ParallelFor(Distances.Num(), [&](const int32 Index)
	{
		const float NewDistance = Distances[Index];
		const float PreviousDistance = PreviousDistances[Index];

		if (FVoxelUtilities::IsNaN(PreviousDistance))
		{
			AdditiveDistances[Index] = NewDistance;
			SubtractiveDistances[Index] = -NewDistance;
			return;
		}

		// FVoxelUtilities::JumpFlood considers 0 to be positive
		// To avoid having a fully positive distance field when making small surface edits,
		// force set values to be negative where the distance changed (where it decreased for additive, increased for subtractive)
		// This will be fixed up by maxing with NewDistance again
		// If we don't do this, JumpFlood will return NaNs everywhere
		// We don't want to do -1 everywhere though, as otherwise any naturally-occuring 0 in the original
		// surface will be added to the additive or subtractive distance fields
		// An easy test for that is sculpting a sphere on a plane at Z = 0: once the plane is removed, the sphere
		// distance field should be contained within the sphere and not "leak" at Z = 0
		// We invert SubtractiveDistance to ensure the behavior is the same sign-wise as additive distances,
		// as otherwise all the 0s would create unwanted surfaces

		AdditiveDistances[Index] = FMath::Max(NewDistance, -PreviousDistance) - (NewDistance < PreviousDistance);
		SubtractiveDistances[Index] = FMath::Max(-NewDistance, PreviousDistance) - (NewDistance > PreviousDistance);
	});

	FVoxelUtilities::JumpFlood(Size, AdditiveDistances);
	FVoxelUtilities::JumpFlood(Size, SubtractiveDistances);

	// Grow the distance fields to ensure we don't get glitches when interpolating between cells
	// eg, Voxel World with a Voxel Size of 10cm but sculpt volume with a Voxel Size of 100cm

	Voxel::ParallelFor(AdditiveDistances, [&](float& Value, const int32 Index)
	{
		if (FVoxelUtilities::IsNaN(Value))
		{
			return;
		}

		ensureVoxelSlow(!FVoxelUtilities::IsNaN(Distances[Index]));
		Value = FMath::Max(Value - 1, Distances[Index]);
	});

	Voxel::ParallelFor(SubtractiveDistances, [&](float& Value, const int32 Index)
	{
		if (FVoxelUtilities::IsNaN(Value))
		{
			return;
		}

		ensureVoxelSlow(!FVoxelUtilities::IsNaN(Distances[Index]));
		Value = FMath::Max(Value - 1, -Distances[Index]);
	});
}