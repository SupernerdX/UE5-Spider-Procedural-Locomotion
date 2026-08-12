// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNode_MergePoints.h"

void FVoxelNode_MergePoints::Compute(const FVoxelGraphQuery Query) const
{
	const TVoxelArray<TValue<FVoxelPointSet>> Inputs = InputPins.Get(Query);

	VOXEL_GRAPH_WAIT(Inputs)
	{
		int32 Num = 0;
		for (const TSharedRef<const FVoxelPointSet>& Input : Inputs)
		{
			Num += Input->Num();
		}
		FVoxelNodeStatScope StatScope(*this, Num);

		OutPin.Set(Query, FVoxelPointSet::Merge(Inputs));
	};
}