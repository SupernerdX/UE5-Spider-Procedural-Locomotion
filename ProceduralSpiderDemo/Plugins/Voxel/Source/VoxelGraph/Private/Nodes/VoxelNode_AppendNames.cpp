// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_AppendNames.h"

void FVoxelNode_AppendNames::Compute(const FVoxelGraphQuery Query) const
{
	const TVoxelArray<TValue<FName>> Names = NamesPins.Get(Query);

	VOXEL_GRAPH_WAIT(Names)
	{
		TStringBuilder<NAME_SIZE> String;
		for (const FName Name : Names)
		{
			Name.AppendString(String);
		}
		NamePin.Set(Query, FName(String));
	};
}