// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelCubeVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvasWriter.h"

TSharedPtr<IVoxelVolumeModifierRuntime> FVoxelCubeVolumeModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	return MakeShared<FVoxelCubeVolumeModifierRuntime>(
		Center,
		Size,
		Rotation,
		Roundness,
		Smoothness,
		Mode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox FVoxelCubeVolumeModifierRuntime::GetBounds() const
{
	return FVoxelBox(Center - Size * 0.5f, Center + Size * 0.5f).Extend(Smoothness * Size * 0.5f);
}

void FVoxelCubeVolumeModifierRuntime::Apply(FVoxelVolumeSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER();

	const FVector HalfSize = Size * 0.5f;
	const float HalfLength = HalfSize.Size();

	Writer.UpdateDistances([&](const float OldDistance, const FVector& Position)
	{
		const FVector PositionFromCenter = Rotation.UnrotateVector(Position - Center);
		const double Radius = FMath::Min3(HalfSize.X, HalfSize.Y, HalfSize.Z) * Roundness;

		const FVector Q = PositionFromCenter.GetAbs() - HalfSize + Radius;
		const double Length = FVector::Max(Q, FVector::ZeroVector).Size();

		float NewDistance = Length + FMath::Min(0.f, FMath::Max3(Q.X, Q.Y, Q.Z)) - Radius;

		if (Mode == EVoxelSculptMode::Remove)
		{
			NewDistance = -NewDistance;
		}

		if (FVoxelUtilities::IsNaN(OldDistance))
		{
			return NewDistance;
		}

		if (Mode == EVoxelSculptMode::Remove)
		{
			return FVoxelUtilities::SmoothMax(OldDistance, NewDistance, Smoothness * HalfLength);
		}
		else
		{
			checkVoxelSlow(Mode == EVoxelSculptMode::Add);
			return FVoxelUtilities::SmoothMin(OldDistance, NewDistance, Smoothness * HalfLength);
		}
	});
}