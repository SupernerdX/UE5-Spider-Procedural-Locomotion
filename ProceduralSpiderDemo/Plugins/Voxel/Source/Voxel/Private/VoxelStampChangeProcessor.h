// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampChange.h"
#include "VoxelVolumeBlendMode.h"

class FVoxelAABBTree;
struct FVoxelStampRuntime;
struct FVoxelHeightStampRuntime;
struct FVoxelVolumeStampRuntime;

template<typename>
struct TVoxelStampChange;

struct FVoxelStampChangeProcessor
{
public:
	const bool bIs2D;
	const float StackMaxDistance;
	const FTransform QueryToWorld;
	const FTransform WorldToQuery;
	const TConstVoxelArrayView<TSharedRef<FVoxelDependency3D>> LODToDependency;

	FVoxelStampChangeProcessor(
		const bool bIs2D,
		const float StackMaxDistance,
		const FTransform& QueryToWorld,
		const FTransform& WorldToQuery,
		const TConstVoxelArrayView<TSharedRef<FVoxelDependency3D>>& LODToDependency)
		: bIs2D(bIs2D)
		, StackMaxDistance(StackMaxDistance)
		, QueryToWorld(QueryToWorld)
		, WorldToQuery(WorldToQuery)
		, LODToDependency(LODToDependency)
	{
	}

public:
	void Process(const TVoxelChunkedArray<FVoxelStampChange>& StampChanges);

private:
	struct FChange
	{
		FVoxelBox Bounds;
		FInt32Interval LODRange;
	};
	TVoxelChunkedArray<FChange> Changes;

	template<typename RuntimeType>
	void ProcessImpl(const TVoxelChunkedArray<TVoxelStampChange<RuntimeType>>& StampChanges);

	void Finalize();
	TSharedRef<FVoxelAABBTree> BuildTree(int32 LOD) const;

private:
	void Invalidate(
		const FVoxelHeightStampRuntime& Stamp,
		TConstVoxelArrayView<FVoxelBox> LocalBoundsToInvalidate);

	void Invalidate(
		const FVoxelVolumeStampRuntime& Stamp,
		TConstVoxelArrayView<FVoxelBox> LocalBoundsToInvalidate);

	template<typename>
	friend struct TVoxelStampChange;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename RuntimeType>
struct TVoxelStampChange
{
	TSharedPtr<const RuntimeType> OldStamp;
	TSharedPtr<const RuntimeType> NewStamp;

	bool ShouldInvalidateEverywhere() const
	{
		checkStatic(
			std::is_same_v<RuntimeType, FVoxelHeightStampRuntime> ||
			std::is_same_v<RuntimeType, FVoxelVolumeStampRuntime>);

		if constexpr (std::is_same_v<RuntimeType, FVoxelVolumeStampRuntime>)
		{
			// Force refresh intersect stamps

			if (OldStamp &&
				OldStamp->GetBlendMode() == EVoxelVolumeBlendMode::Intersect)
			{
				return true;
			}

			if (NewStamp &&
				NewStamp->GetBlendMode() == EVoxelVolumeBlendMode::Intersect)
			{
				return true;
			}
		}

		const bool bOldStampHasCollectDependencies = OldStamp && OldStamp->HasCollectDependencies();
		const bool bNewStampHasCollectDependencies = NewStamp && NewStamp->HasCollectDependencies();

		if (bOldStampHasCollectDependencies != bNewStampHasCollectDependencies)
		{
			// Force refresh if HasCollectDependencies changed
			return true;
		}

		return false;
	}

	bool TryToPartiallyInvalidate(FVoxelStampChangeProcessor& Processor) const
	{
		if (!OldStamp ||
			!NewStamp ||
			!NewStamp->CanPartiallyInvalidate())
		{
			return false;
		}

		TVoxelArray<FVoxelBox> LocalBoundsToInvalidate;
		if (!NewStamp->TryToPartiallyInvalidate(
			*OldStamp,
			LocalBoundsToInvalidate))
		{
			return false;
		}

		const auto& OldStampRef = OldStamp->GetStamp();
		const auto& NewStampRef = NewStamp->GetStamp();

		if constexpr (std::is_same_v<RuntimeType, FVoxelVolumeStampRuntime>)
		{
			if (OldStampRef.BoundsExtensionMultiplier != NewStampRef.BoundsExtensionMultiplier ||
				OldStampRef.MaximumBoundsExtension != NewStampRef.MaximumBoundsExtension)
			{
				return false;
			}
		}
		else
		{
			if (OldStampRef.HeightPaddingMultiplier != NewStampRef.HeightPaddingMultiplier)
			{
				return false;
			}
		}

		if (OldStampRef.Behavior != NewStampRef.Behavior ||
			OldStampRef.Priority != NewStampRef.Priority ||
			OldStampRef.Smoothness != NewStampRef.Smoothness ||
			OldStamp->GetBlendMode() != NewStamp->GetBlendMode() ||
			OldStamp->GetLODRange() != NewStamp->GetLODRange() ||
			!OldStamp->GetLocalToWorld().Equals(NewStamp->GetLocalToWorld(), 0))
		{
			return false;
		}

		Processor.Invalidate(
			*OldStamp,
			LocalBoundsToInvalidate);

		Processor.Invalidate(
			*NewStamp,
			LocalBoundsToInvalidate);

		return true;
	}

	void Invalidate(FVoxelStampChangeProcessor& Processor) const
	{
		if (OldStamp)
		{
			Processor.Invalidate(
				*OldStamp,
				MakeVoxelArrayView(OldStamp->GetLocalBounds()));
		}

		if (NewStamp)
		{
			Processor.Invalidate(
				*NewStamp,
				MakeVoxelArrayView(NewStamp->GetLocalBounds()));
		}
	}
};