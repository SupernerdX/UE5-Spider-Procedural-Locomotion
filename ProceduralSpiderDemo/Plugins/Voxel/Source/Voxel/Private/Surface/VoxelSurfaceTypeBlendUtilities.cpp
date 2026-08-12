// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Surface/VoxelSurfaceTypeBlendUtilities.h"
#include "VoxelBufferAccessor.h"

FVoxelSurfaceTypeBlendBuffer FVoxelSurfaceTypeBlendUtilities::Lerp(
	const FVoxelSurfaceTypeBlendBuffer& A,
	const FVoxelSurfaceTypeBlendBuffer& B,
	const FVoxelFloatBuffer& Alpha)
{
	const FVoxelBufferAccessor BufferAccessor(A, B, Alpha);
	if (!ensure(BufferAccessor.IsValid()))
	{
		return FVoxelSurfaceTypeBlendBuffer::MakeDefault();
	}
	const int32 Num = BufferAccessor.Num();

	VOXEL_FUNCTION_COUNTER_NUM(Num);

	FVoxelSurfaceTypeBlendBuffer Result;
	Result.Allocate(Num);

	if (A.IsConstant() &&
		B.IsConstant() &&
		A.GetConstant().GetLayers().Num() == 1 &&
		B.GetConstant().GetLayers().Num() == 1)
	{
		VOXEL_SCOPE_COUNTER("Fast path");

		const FVoxelSurfaceType LayerA = A.GetConstant().GetLayers()[0].Type;
		const FVoxelSurfaceType LayerB = B.GetConstant().GetLayers()[0].Type;

		if (LayerA.IsNull())
		{
			return FVoxelSurfaceTypeBlend::FromType(LayerB);
		}
		if (LayerB.IsNull())
		{
			return FVoxelSurfaceTypeBlend::FromType(LayerA);
		}

		if (LayerA == LayerB)
		{
			return FVoxelSurfaceTypeBlend::FromType(LayerA);
		}

		for (int32 Index = 0; Index < Num; Index++)
		{
			FVoxelSurfaceTypeBlend& Blend = Result.View()[Index];

			int32 WeightB = FVoxelUtilities::FloatToUINT16(Alpha[Index]);
			if (WeightB == 0)
			{
				Blend.InitializeFromType(LayerA);
				continue;
			}
			if (WeightB == MAX_uint16)
			{
				Blend.InitializeFromType(LayerB);
				continue;
			}

			int32 WeightA = MAX_uint16 - WeightB;

			FVoxelSurfaceType SortedLayerA = LayerA;
			FVoxelSurfaceType SortedLayerB = LayerB;
			if (SortedLayerA > SortedLayerB)
			{
				Swap(SortedLayerA, SortedLayerB);
				Swap(WeightA, WeightB);
			}

			Blend.NumLayers = 2;
			Blend.Layers[0] = FVoxelSurfaceTypeBlendLayer(SortedLayerA, FVoxelSurfaceTypeBlendWeight::MakeUnsafe(WeightA));
			Blend.Layers[1] = FVoxelSurfaceTypeBlendLayer(SortedLayerB, FVoxelSurfaceTypeBlendWeight::MakeUnsafe(WeightB));
			Blend.Check();
		}

		return Result;
	}

	for (int32 Index = 0; Index < Num; Index++)
	{
		FVoxelSurfaceTypeBlend::Lerp(
			Result.View()[Index],
			A[Index],
			B[Index],
			FMath::Clamp(Alpha[Index], 0.f, 1.f));
	}

	return Result;
}