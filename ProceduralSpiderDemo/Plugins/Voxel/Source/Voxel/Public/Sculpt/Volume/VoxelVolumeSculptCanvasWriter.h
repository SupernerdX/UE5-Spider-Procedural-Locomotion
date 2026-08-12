// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelBuffer;
struct FVoxelMetadataRef;
struct FVoxelSurfaceTypeBlend;
class FVoxelVolumeSculptCanvas;

class VOXEL_API FVoxelVolumeSculptCanvasWriter
{
public:
	const FIntVector Size;
	const float DistanceScale;
	const FMatrix IndexToWorld;
	const FVoxelIntBox Indices;

	FVoxelVolumeSculptCanvasWriter(
		FVoxelVolumeSculptCanvas& Canvas,
		const FMatrix& SculptToWorld);

private:
	FVoxelVolumeSculptCanvas& Canvas;

public:
	TVoxelArrayView<float> GetUnscaledDistances();

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
	requires LambdaHasSignature_V<LambdaType, float(float, const FVector&)>
	void UpdateDistances(LambdaType Lambda)
	{
		VOXEL_FUNCTION_COUNTER_NUM(Indices.Count_int32());

		const TVoxelArrayView<float> UnscaledDistances = GetUnscaledDistances();

		Voxel::ParallelFor(Indices.GetZ(), [&](const int32 IndexZ)
		{
			for (int32 IndexY = Indices.Min.Y; IndexY < Indices.Max.Y; IndexY++)
			{
				for (int32 IndexX = Indices.Min.X; IndexX < Indices.Max.X; IndexX++)
				{
					float& UnscaledDistance = UnscaledDistances[FVoxelUtilities::Get3DIndex<int32>(Size, IndexX, IndexY, IndexZ)];
					const FVector Position = IndexToWorld.TransformPosition(FVector(IndexX, IndexY, IndexZ));

					UnscaledDistance = Lambda(UnscaledDistance * DistanceScale, Position) / DistanceScale;
				}
			}
		});
	}
};