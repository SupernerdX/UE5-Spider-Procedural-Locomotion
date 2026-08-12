// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/Tools/VoxelFlattenHeightModifier.h"
#include "Sculpt/Height/VoxelHeightSculptCanvasWriter.h"

TSharedPtr<IVoxelHeightModifierRuntime> FVoxelFlattenHeightModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	return MakeShared<FVoxelFlattenHeightModifierRuntime>(
		Center,
		Radius,
		Falloff,
		Type,
		TargetHeight,
		Brush.GetRuntimeBrush());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox2D FVoxelFlattenHeightModifierRuntime::GetBounds() const
{
	return FVoxelBox2D(Center - Radius, Center + Radius);
}

void FVoxelFlattenHeightModifierRuntime::Apply(FVoxelHeightSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER();

	Writer.UpdateHeights([&](const float OldHeight, const FVector2D& Position)
	{
		if (FVoxelUtilities::IsNaN(OldHeight))
		{
			return OldHeight;
		}

		const double DistanceToCenter = FVector2D::Distance(Position, Center);

		const float FalloffMultiplier = Brush->GetFalloff(DistanceToCenter, Radius);
		const float MaskStrength = Brush->GetMaskStrength(Center, Position, Radius);

		const float NewHeight = FMath::Lerp(OldHeight, TargetHeight, FalloffMultiplier * MaskStrength);

		float Height = OldHeight;

		if (Type == EVoxelLevelToolType::Additive ||
			Type == EVoxelLevelToolType::Both)
		{
			Height = FMath::Max(Height, NewHeight);
		}
		if (Type == EVoxelLevelToolType::Subtractive ||
			Type == EVoxelLevelToolType::Both)
		{
			Height = FMath::Min(Height, NewHeight);
		}

		return Height;
	});
}