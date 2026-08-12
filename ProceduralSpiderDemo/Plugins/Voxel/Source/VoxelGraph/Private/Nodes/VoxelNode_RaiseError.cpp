// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_RaiseError.h"

void FVoxelNode_RaiseError::Compute(const FVoxelGraphQuery Query) const
{
	const FValue Value = InPin.Get(Query);
	const TValue<FName> Error = ErrorPin.Get(Query);

	VOXEL_GRAPH_WAIT(Value, Error)
	{
		VOXEL_MESSAGE(Error, "{0}: {1}", this, Error);

		OutPin.Set(Query, Value);
	};
}

#if WITH_EDITOR
void FVoxelNode_RaiseError::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	GetPin(InPin).SetType(NewType);
	GetPin(OutPin).SetType(NewType);
}
#endif