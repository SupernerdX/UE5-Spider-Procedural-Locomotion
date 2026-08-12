// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelScreenSizeHelper.h"
#include "VoxelSubsystem.h"

FVoxelScreenSizeHelper::FVoxelScreenSizeHelper(const FVoxelConfig& Config)
{
	FFloatInterval Quality =
		Config.bIsEditorWorld && !Config.LODQuality.bAlwaysUseGameQuality
		? Config.LODQuality.EditorQuality
		: Config.LODQuality.GameQuality;

	Quality.Min *= 32 / double(Config.RenderChunkSize);
	Quality.Max *= 32 / double(Config.RenderChunkSize);

	Quality.Min = FMath::Max(Quality.Min, 0);
	Quality.Max = FMath::Max(Quality.Min, Quality.Max);

	MinQuality = Quality.Min;
	MaxQuality = Quality.Max;

	ensure(MinQuality <= MaxQuality);

	ChunkToWorld = Config.RenderChunkSize * Config.VoxelSize;
	QualityExponent = FMath::Clamp(Config.QualityExponent, 0.001f, 100.f);
}

double FVoxelScreenSizeHelper::GetChunkQuality(const TVoxelArray<FVector>& Invokers, const FVoxelChunkKey ChunkKey) const
{
	const FVoxelIntBox Bounds = ChunkKey.GetChunkKeyBounds();

	double ClosestDistance = MAX_flt;

	for (const FVector& InvokerPosition : Invokers)
	{
		ClosestDistance = FMath::Min(ClosestDistance, Bounds.DistanceToPoint(InvokerPosition));
	}

	if (ClosestDistance == 0)
	{
		return 0;
	}

	const int32 ChunkSize = 1 << ChunkKey.LOD;
	return ClosestDistance / FMath::Pow(double(ChunkSize), QualityExponent);
}