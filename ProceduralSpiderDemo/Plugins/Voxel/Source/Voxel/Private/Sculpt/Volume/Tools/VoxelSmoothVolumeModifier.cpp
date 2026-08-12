// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/Tools/VoxelSmoothVolumeModifier.h"
#include "Sculpt/Volume/VoxelVolumeSculptCanvasWriter.h"

TSharedPtr<IVoxelVolumeModifierRuntime> FVoxelSmoothVolumeModifier::GetRuntime() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	return MakeShared<FVoxelSmoothVolumeModifierRuntime>(
		Center,
		Radius,
		Strength,
		Brush.GetRuntimeBrush());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox FVoxelSmoothVolumeModifierRuntime::GetBounds() const
{
	return FVoxelBox(Center - Radius, Center + Radius);
}

void FVoxelSmoothVolumeModifierRuntime::Apply(FVoxelVolumeSculptCanvasWriter& Writer) const
{
	VOXEL_FUNCTION_COUNTER();

	const FIntVector Size = Writer.Size;
	const FMatrix IndexToWorld = Writer.IndexToWorld;
	const FVoxelIntBox Indices = Writer.Indices;
	// Scale has no effect when smoothing
	const TVoxelArrayView<float> Distances = Writer.GetUnscaledDistances();

	float CenterStrength = 1;
	float NeighborsStrength = Strength;
	const float Sum = 26 * NeighborsStrength + CenterStrength;
	CenterStrength /= Sum;
	NeighborsStrength /= Sum;

	const TVoxelArray<float> OldDistances = INLINE_LAMBDA
	{
		VOXEL_SCOPE_COUNTER("Copy distances");
		return Distances.Array();
	};

	Voxel::ParallelFor(Indices.GetZ(), [&](const int32 IndexZ)
	{
		for (int32 IndexY = Indices.Min.Y; IndexY < Indices.Max.Y; IndexY++)
		{
			for (int32 IndexX = Indices.Min.X; IndexX < Indices.Max.X; IndexX++)
			{
				float& Distance = Distances[FVoxelUtilities::Get3DIndex<int32>(Size, IndexX, IndexY, IndexZ)];
				if (FVoxelUtilities::IsNaN(Distance))
				{
					continue;
				}

				const auto GetNeighbor = [&](const int32 DX, const int32 DY, const int32 DZ)
				{
					const int32 NeighborIndexX = IndexX + DX;
					const int32 NeighborIndexY = IndexY + DY;
					const int32 NeighborIndexZ = IndexZ + DZ;

					const int32 NeighborIndex = NeighborIndexX + Size.X * NeighborIndexY + Size.X * Size.Y * NeighborIndexZ;
					if (!ensure(OldDistances.IsValidIndex(NeighborIndex)))
					{
						return 0.f;
					}

					const float OldDistance = OldDistances[NeighborIndex];
					if (FVoxelUtilities::IsNaN(OldDistance))
					{
						return 0.f;
					}

					return OldDistance * NeighborsStrength;
				};

				const float NeighborSum =
					GetNeighbor(-1, -1, -1) +
					GetNeighbor(+0, -1, -1) +
					GetNeighbor(+1, -1, -1) +
					GetNeighbor(-1, +0, -1) +
					GetNeighbor(+0, +0, -1) +
					GetNeighbor(+1, +0, -1) +
					GetNeighbor(-1, +1, -1) +
					GetNeighbor(+0, +1, -1) +
					GetNeighbor(+1, +1, -1) +
					GetNeighbor(-1, -1, +0) +
					GetNeighbor(+0, -1, +0) +
					GetNeighbor(+1, -1, +0) +
					GetNeighbor(-1, +0, +0) +
					GetNeighbor(+1, +0, +0) +
					GetNeighbor(-1, +1, +0) +
					GetNeighbor(+0, +1, +0) +
					GetNeighbor(+1, +1, +0) +
					GetNeighbor(-1, -1, +1) +
					GetNeighbor(+0, -1, +1) +
					GetNeighbor(+1, -1, +1) +
					GetNeighbor(-1, +0, +1) +
					GetNeighbor(+0, +0, +1) +
					GetNeighbor(+1, +0, +1) +
					GetNeighbor(-1, +1, +1) +
					GetNeighbor(+0, +1, +1) +
					GetNeighbor(+1, +1, +1);

				const FVector Position = IndexToWorld.TransformPosition(FVector(IndexX, IndexY, IndexZ));
				const double DistanceToCenter = FVector::Distance(Position, Center);

				const float FalloffMultiplier = Brush->GetFalloff(DistanceToCenter, Radius);
				const float MaskStrength = Brush->GetMaskStrength(Center, Position, Radius);

				Distance = FMath::Lerp(Distance, NeighborSum + Distance * CenterStrength, FalloffMultiplier * MaskStrength);
			}
		}
	});
}