// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelLayerBase.h"

class VOXEL_API FVoxelHeightLayer : public FVoxelLayerBase
{
public:
	const float MaxDistance;
	const TSharedPtr<const FVoxelHeightLayer> PreviousLayer;

	VOXEL_COUNT_INSTANCES()

	FVoxelHeightLayer(
		const FVoxelWeakStackLayer& WeakLayer,
		const float MaxDistance,
		const TVoxelArray<TSharedRef<FVoxelFutureStampTree>>& LODToTree,
		const TSharedPtr<const FVoxelHeightLayer>& PreviousLayer)
		: FVoxelLayerBase(WeakLayer, LODToTree)
		, MaxDistance(MaxDistance)
		, PreviousLayer(PreviousLayer)
	{
	}

public:
	FVoxelOptionalBox GetBoundsToGenerate(FVoxelDependencyCollector& DependencyCollector) const;

public:
	bool HasStamps(
		const FVoxelQuery& Query,
		const FVoxelBox& Bounds,
		EVoxelStampBehavior BehaviorMask,
		bool bExtendByMaxDistance) const;

	FVoxelOptionalBox2D GetStampBounds(
		const FVoxelQuery& Query,
		const FVoxelBox2D& Bounds,
		EVoxelStampBehavior BehaviorMask) const;

public:
	void Sample(const FVoxelHeightBulkQuery& Query) const;
	void Sample(const FVoxelHeightSparseQuery& Query) const;

private:
	template<typename QueryType>
	void SampleStamps(
		const FVoxelQuery& PreviousQuery,
		const QueryType& Query,
		TConstVoxelArrayView<FVoxelStampTree::FStamp> Stamps) const;

	template<typename QueryType>
	void SampleStampsImpl(
		const QueryType& Query,
		TConstVoxelArrayView<FVoxelStampTree::FStamp> Stamps) const;

public:
	void SampleAsVolume(const FVoxelVolumeBulkQuery& Query) const;
	void SampleAsVolume(const FVoxelVolumeSparseQuery& Query) const;
};