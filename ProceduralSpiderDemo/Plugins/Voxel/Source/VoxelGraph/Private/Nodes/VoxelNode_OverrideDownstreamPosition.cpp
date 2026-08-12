// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelNode_OverrideDownstreamPosition.h"
#include "VoxelGraphPositionParameter.h"

void FVoxelNode_OverrideDownstreamPosition::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelVectorBuffer> Positions = PositionPin.Get(Query);

	VOXEL_GRAPH_WAIT(Positions)
	{
		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		NewQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(*Positions);

		const FValue Data = DataPin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));

		VOXEL_GRAPH_WAIT(Data)
		{
			OutDataPin.Set(Query, Data);
		};
	};
}

#if WITH_EDITOR
FVoxelPinTypeSet FVoxelNode_OverrideDownstreamPosition::GetPromotionTypes(const FVoxelPin& Pin) const
{
	return FVoxelPinTypeSet::All();
}

void FVoxelNode_OverrideDownstreamPosition::PromotePin(FVoxelPin& Pin, const FVoxelPinType& NewType)
{
	GetPin(DataPin).SetType(NewType);
	GetPin(OutDataPin).SetType(NewType);
}
#endif