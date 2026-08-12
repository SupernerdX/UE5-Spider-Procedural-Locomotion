// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelSphereVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvasWriter.h"

TSharedPtr<IVoxelVolumeModifierRuntime> FVoxelSphereVolumeModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	return MakeShared<FVoxelSphereVolumeModifierRuntime>(
		Center,
		Radius,
		Smoothness,
		Mode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox FVoxelSphereVolumeModifierRuntime::GetBounds() const
{
	return FVoxelBox(Center - Radius, Center + Radius).Extend(Smoothness * Radius);
}

void FVoxelSphereVolumeModifierRuntime::Apply(FVoxelVolumeSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER();

	Writer.UpdateDistances([&](const float OldDistance, const FVector& Position)
	{
		const double DistanceToCenter = FVector::Distance(Position, Center);

		float NewDistance = DistanceToCenter - Radius;
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
			return FVoxelUtilities::SmoothMax(OldDistance, NewDistance, Smoothness * Radius);
		}
		else
		{
			checkVoxelSlow(Mode == EVoxelSculptMode::Add);
			return FVoxelUtilities::SmoothMin(OldDistance, NewDistance, Smoothness * Radius);
		}
	});
}