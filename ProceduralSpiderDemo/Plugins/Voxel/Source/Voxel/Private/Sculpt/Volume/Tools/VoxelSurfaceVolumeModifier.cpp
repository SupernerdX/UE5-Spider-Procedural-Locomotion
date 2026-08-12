// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelSurfaceVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvasWriter.h"

TSharedPtr<IVoxelVolumeModifierRuntime> FVoxelSurfaceVolumeModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	return MakeShared<FVoxelSurfaceVolumeModifierRuntime>(
		Center,
		Radius,
		Strength,
		Mode,
		Brush.GetRuntimeBrush());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox FVoxelSurfaceVolumeModifierRuntime::GetBounds() const
{
	return FVoxelBox(Center - Radius, Center + Radius);
}

void FVoxelSurfaceVolumeModifierRuntime::Apply(FVoxelVolumeSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER();

	Writer.UpdateDistances([&](const float OldDistance, const FVector& Position)
	{
		if (FVoxelUtilities::IsNaN(OldDistance))
		{
			return OldDistance;
		}

		const double DistanceToCenter = FVector::Distance(Position, Center);

		const float FalloffMultiplier = Brush->GetFalloff(DistanceToCenter, Radius);
		const float MaskStrength = Brush->GetMaskStrength(Center, Position, Radius);
		const float Delta = Radius / 10 * FalloffMultiplier * Strength * MaskStrength;

		if (Mode == EVoxelSculptMode::Add)
		{
			return OldDistance - Delta;
		}
		else
		{
			checkVoxelSlow(Mode == EVoxelSculptMode::Remove);
			return OldDistance + Delta;
		}
	});
}