// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelAngleVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvasWriter.h"

TSharedPtr<IVoxelVolumeModifierRuntime> FVoxelAngleVolumeModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	return MakeShared<FVoxelAngleVolumeModifierRuntime>(
		Center,
		Radius,
		Strength,
		Plane,
		MergeMode,
		Brush.GetRuntimeBrush());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox FVoxelAngleVolumeModifierRuntime::GetBounds() const
{
	return FVoxelBox(Center - Radius, Center + Radius);
}

void FVoxelAngleVolumeModifierRuntime::Apply(FVoxelVolumeSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER();

	Writer.UpdateDistances([&](const float OldDistance, const FVector& Position)
	{
		if (FVoxelUtilities::IsNaN(OldDistance))
		{
			return OldDistance;
		}

		const double DistanceToCenter = FVector::Distance(Position, Center);

		if (DistanceToCenter > Radius)
		{
			return OldDistance;
		}

		float Alpha = Brush->GetFalloff(DistanceToCenter, Radius);
		Alpha *= Strength;
		Alpha *= Brush->GetMaskStrength(Center, Position, Radius);

		const float DistanceToPlane = Plane.PlaneDot(Position);

		const float NewDistance = INLINE_LAMBDA
		{
			switch (MergeMode)
			{
			default: ensure(false);
			case EVoxelSDFMergeMode::Union: return FMath::Min(OldDistance, DistanceToPlane);
			case EVoxelSDFMergeMode::Intersection: return FMath::Max(DistanceToPlane, OldDistance);
			case EVoxelSDFMergeMode::Override: return DistanceToPlane;
			}
		};

		return FMath::Lerp(OldDistance, NewDistance, FMath::Clamp(Alpha, 0.f, 1.f));
	});
}