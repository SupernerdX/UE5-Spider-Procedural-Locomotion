// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_Parameter.h"

void FVoxelNode_Parameter::Compute(const FVoxelGraphQuery Query) const
{
	const FVoxelRuntimePinValue* ParameterValuePtr = Query->Context.Environment.ParameterGuidToValue.Find(ParameterGuid);
	if (!ensureVoxelSlow(ParameterValuePtr))
	{
		VOXEL_MESSAGE(Error, " {0}: INTERNAL ERROR: Missing parameter", this);
		return;
	}

	if (!ensureVoxelSlow(ParameterValuePtr->GetType() == ValuePin.GetType_RuntimeOnly()))
	{
		VOXEL_MESSAGE(Error, " {0}: INTERNAL ERROR: Invalid parameter type", this);
		return;
	}

	ValuePin.Set(Query, *ParameterValuePtr);
}