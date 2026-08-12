// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/Tools/VoxelSculptHeightModifier.h"
#include "Sculpt/Height/VoxelHeightSculptCanvasWriter.h"

TSharedPtr<IVoxelHeightModifierRuntime> FVoxelSculptHeightModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	return MakeShared<FVoxelSculptHeightModifierRuntime>(
		Center,
		Radius,
		Strength,
		Mode,
		Brush.GetRuntimeBrush());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox2D FVoxelSculptHeightModifierRuntime::GetBounds() const
{
	return FVoxelBox2D(Center - Radius, Center + Radius);
}

void FVoxelSculptHeightModifierRuntime::Apply(FVoxelHeightSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER();

	Writer.UpdateHeights([&](const float OldHeight, const FVector2D& Position)
	{
		const double Distance = FVector2D::Distance(Position, Center);
		const float FalloffMultiplier = Brush->GetFalloff(Distance, Radius);
		const float MaskStrength = Brush->GetMaskStrength(Center, Position, Radius);
		const float Delta = Radius / 10 * FalloffMultiplier * Strength * MaskStrength;

		if (Mode == EVoxelSculptMode::Add)
		{
			return OldHeight + Delta;
		}
		else
		{
			checkVoxelSlow(Mode == EVoxelSculptMode::Remove);
			return OldHeight - Delta;
		}
	});
}