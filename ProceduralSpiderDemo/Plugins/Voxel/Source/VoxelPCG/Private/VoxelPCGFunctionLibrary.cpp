// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPCGFunctionLibrary.h"
#include "Scatter/VoxelScatterFunctionLibrary.h"

FVoxelBox UVoxelPCGFunctionLibrary::GetPCGBounds() const
{
	const FVoxelGraphParameters::FPointSetChunkBounds* Parameter = Query->FindParameter<FVoxelGraphParameters::FPointSetChunkBounds>();
	if (!Parameter)
	{
		VOXEL_MESSAGE(Error, "{0}: No bounds passed in input", this);
		return {};
	}

	if (!Parameter->GetInitialBounds().IsValidAndNotEmpty())
	{
		VOXEL_MESSAGE(Error, "{0}: No or invalid bounds passed in input", this);
		return {};
	}

	return Parameter->GetInitialBounds();
}