// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/Tools/VoxelSmoothHeightModifier.h"
#include "Sculpt/Height/VoxelHeightSculptCanvasWriter.h"

TSharedPtr<IVoxelHeightModifierRuntime> FVoxelSmoothHeightModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	return MakeShared<FVoxelSmoothHeightModifierRuntime>(
		Center,
		Radius,
		Strength,
		Brush.GetRuntimeBrush());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox2D FVoxelSmoothHeightModifierRuntime::GetBounds() const
{
	return FVoxelBox2D(Center - Radius, Center + Radius);
}

void FVoxelSmoothHeightModifierRuntime::Apply(FVoxelHeightSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER_NUM(Writer.Indices.Count_int32());

	const FIntPoint Size = Writer.Size;
	const FTransform2d IndexToWorld = Writer.IndexToWorld;
	const FVoxelIntBox2D Indices = Writer.Indices;
	const TVoxelArrayView<float> Heights = Writer.GetUnscaledHeights();

	float CenterStrength = 1;
	float NeighborsStrength = Strength;
	const float Sum = 8 * NeighborsStrength + CenterStrength;
	CenterStrength /= Sum;
	NeighborsStrength /= Sum;

	const TVoxelArray<float> OldHeights = INLINE_LAMBDA
	{
		VOXEL_SCOPE_COUNTER("Copy heights");
		return Heights.Array();
	};

	for (int32 IndexY = Indices.Min.Y; IndexY < Indices.Max.Y; IndexY++)
	{
		for (int32 IndexX = Indices.Min.X; IndexX < Indices.Max.X; IndexX++)
		{
			float& Height = Heights[FVoxelUtilities::Get2DIndex<int32>(Size, IndexX, IndexY)];
			if (FVoxelUtilities::IsNaN(Height))
			{
				continue;
			}

			const auto GetNeighbor = [&](const int32 DX, const int32 DY)
			{
				const int32 NeighborIndexX = IndexX + DX;
				const int32 NeighborIndexY = IndexY + DY;

				const int32 NeighborIndex = FVoxelUtilities::Get2DIndex<int32>(Size, NeighborIndexX, NeighborIndexY);
				if (!ensure(OldHeights.IsValidIndex(NeighborIndex)))
				{
					return 0.f;
				}

				const float OldHeight = OldHeights[NeighborIndex];
				if (FVoxelUtilities::IsNaN(OldHeight))
				{
					return 0.f;
				}

				return OldHeight * NeighborsStrength;
			};

			const float NeighborSum =
				GetNeighbor(-1, -1) +
				GetNeighbor(+0, -1) +
				GetNeighbor(+1, -1) +
				GetNeighbor(-1, +0) +
				GetNeighbor(+1, +0) +
				GetNeighbor(-1, +1) +
				GetNeighbor(+0, +1) +
				GetNeighbor(+1, +1);

			const FVector2D Position = IndexToWorld.TransformPoint(FVector2D(IndexX, IndexY));
			const double DistanceToCenter = FVector2D::Distance(Position, Center);

			const float FalloffMultiplier = Brush->GetFalloff(DistanceToCenter, Radius);
			const float MaskStrength = Brush->GetMaskStrength(Center, Position, Radius);

			Height = FMath::Lerp(Height, NeighborSum + Height * CenterStrength, FalloffMultiplier * MaskStrength);
		}
	}
}