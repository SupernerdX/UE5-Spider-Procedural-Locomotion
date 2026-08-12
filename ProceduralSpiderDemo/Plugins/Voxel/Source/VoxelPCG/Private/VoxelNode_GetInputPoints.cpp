// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNode_GetInputPoints.h"

void FVoxelNode_GetInputPoints::Compute(const FVoxelGraphQuery Query) const
{
	const FVoxelGraphParameters::FPointSet* Parameter = Query->FindParameter<FVoxelGraphParameters::FPointSet>();
	if (!Parameter ||
		!ensure(Parameter->Value))
	{
		VOXEL_MESSAGE(Error, "{0}: No points passed in input", this);
		return;
	}

	PointsPin.Set(Query, Parameter->Value.ToSharedRef());
}