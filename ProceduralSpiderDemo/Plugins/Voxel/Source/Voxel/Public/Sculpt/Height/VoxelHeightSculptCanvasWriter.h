// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelBuffer;
struct FVoxelMetadataRef;
struct FVoxelSurfaceTypeBlend;
class FVoxelHeightSculptCanvas;

class VOXEL_API FVoxelHeightSculptCanvasWriter
{
public:
	const FIntPoint Size;
	const FTransform2d IndexToWorld;
	const float ScaleZ;
	const float OffsetZ;
	const FVoxelIntBox2D Indices;

	FVoxelHeightSculptCanvasWriter(
		FVoxelHeightSculptCanvas& Canvas,
		const FTransform2d& SculptToWorld,
		float ScaleZ,
		float OffsetZ);

private:
	FVoxelHeightSculptCanvas& Canvas;

public:
	TVoxelArrayView<float> GetUnscaledHeights();

	struct FSurfaceTypes
	{
		TVoxelArrayView<float> Alphas;
		TVoxelArrayView<FVoxelSurfaceTypeBlend> Blends;
	};
	FSurfaceTypes GetSurfaceTypes();

	struct FMetadata
	{
		TVoxelArrayView<float> Alphas;
		FVoxelBuffer* Buffer = nullptr;
	};
	FMetadata GetMetadata(const FVoxelMetadataRef& MetadataRef);

public:
	template<typename LambdaType>
	requires LambdaHasSignature_V<LambdaType, float(float, const FVector2D&)>
	void UpdateHeights(LambdaType Lambda)
	{
		VOXEL_FUNCTION_COUNTER_NUM(Indices.Count_int32());

		const TVoxelArrayView<float> Heights = GetUnscaledHeights();

		Voxel::ParallelFor(Indices.GetY(), [&](const int32 IndexY)
		{
			for (int32 IndexX = Indices.Min.X; IndexX < Indices.Max.X; IndexX++)
			{
				float& Height = Heights[FVoxelUtilities::Get2DIndex<int32>(Size, IndexX, IndexY)];
				const FVector2D Position = IndexToWorld.TransformPoint(FVector2D(IndexX, IndexY));

				Height = Lambda(Height, Position);
			}
		});
	}
};
