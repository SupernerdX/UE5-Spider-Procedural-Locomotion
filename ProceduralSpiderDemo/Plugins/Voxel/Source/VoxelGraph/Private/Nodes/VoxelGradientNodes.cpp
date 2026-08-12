// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Nodes/VoxelGradientNodes.h"
#include "VoxelGraphPositionParameter.h"
#include "Utilities/VoxelBufferMathUtilities.h"
#include "Utilities/VoxelBufferGradientUtilities.h"

void FVoxelNode_GetGradient2D::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<float> Granularity = GranularityPin.Get(Query);

	VOXEL_GRAPH_WAIT(Granularity)
	{
		const FVoxelGraphParameters::FPosition2D* PositionParameter = FVoxelGraphParameters::FPosition2D::Find(Query);
		if (!PositionParameter)
		{
			VOXEL_MESSAGE(Error, "{0}: Cannot query 2D positions here", this);
			return;
		}

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		{
			FVoxelVector2DBuffer QueryPositions = PositionParameter->GetLocalPosition_Float(Query);
			QueryPositions.ExpandConstants();

			const FVoxelVector2DBuffer NewPositions = FVoxelBufferGradientUtilities::SplitPositions2D(QueryPositions, Granularity);

			// Ensure Position always resolves to our new Position2D downstream
			NewQuery.RemoveParameter<FVoxelGraphParameters::FPosition3D>();
			NewQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetLocalPosition(NewPositions);
		}

		const TValue<FVoxelFloatBuffer> Value = ValuePin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));
		const TValue<bool> Normalize = NormalizePin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));

		VOXEL_GRAPH_WAIT(Granularity, PositionParameter, Value, Normalize)
		{
			const FVoxelVector2DBuffer QueryPositions = PositionParameter->GetLocalPosition_Float(Query);

			if (Value->IsConstant())
			{
				GradientPin.Set(Query, FVoxelVector2DBuffer(FVector2f::ZeroVector));
				return;
			}

			if (!ensureVoxelSlow(Value->Num() == 4 * QueryPositions.Num()))
			{
				VOXEL_MESSAGE(Error, "Input size mismatch");

				GradientPin.Set(Query, FVoxelVector2DBuffer(FVector2f::ZeroVector));
				return;
			}

			FVoxelVector2DBuffer Result = FVoxelBufferGradientUtilities::CollapseGradient2D(*Value, QueryPositions.Num(), Granularity);

			if (Normalize)
			{
				Result = FVoxelBufferMathUtilities::Normalize(Result);
			}

			GradientPin.Set(Query, MoveTemp(Result));
		};
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_GetGradient3D::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<float> Granularity = GranularityPin.Get(Query);

	VOXEL_GRAPH_WAIT(Granularity)
	{
		const FVoxelGraphParameters::FPosition3D* PositionParameter = Query->FindParameter<FVoxelGraphParameters::FPosition3D>();
		if (!PositionParameter)
		{
			VOXEL_MESSAGE(Error, "{0}: Cannot query 3D positions here", this);
			return;
		}

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		{
			FVoxelVectorBuffer QueryPositions = PositionParameter->GetLocalPosition_Float(Query);
			QueryPositions.ExpandConstants();

			const FVoxelVectorBuffer NewPositions = FVoxelBufferGradientUtilities::SplitPositions3D(QueryPositions, Granularity);

			// Ensure Position2D resolves to our new Position3D downstream
			NewQuery.RemoveParameter<FVoxelGraphParameters::FPosition2D>();
			NewQuery.AddParameter<FVoxelGraphParameters::FPosition3D>().SetLocalPosition(NewPositions);
		}

		const TValue<FVoxelFloatBuffer> Value = ValuePin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));
		const TValue<bool> Normalize = NormalizePin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));

		VOXEL_GRAPH_WAIT(Granularity, PositionParameter, Value, Normalize)
		{
			const FVoxelVectorBuffer QueryPositions = PositionParameter->GetLocalPosition_Float(Query);

			if (Value->IsConstant())
			{
				GradientPin.Set(Query, FVoxelVectorBuffer(FVector3f::ZeroVector));
				return;
			}

			if (!ensureVoxelSlow(Value->Num() == 6 * QueryPositions.Num()))
			{
				VOXEL_MESSAGE(Error, "Input size mismatch");

				GradientPin.Set(Query, FVoxelVectorBuffer(FVector3f::ZeroVector));
				return;
			}

			FVoxelVectorBuffer Result = FVoxelBufferGradientUtilities::CollapseGradient3D(*Value, QueryPositions.Num(), Granularity);

			if (Normalize)
			{
				Result = FVoxelBufferMathUtilities::Normalize(Result);
			}

			GradientPin.Set(Query, MoveTemp(Result));
		};
	};
}