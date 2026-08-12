// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelPointNodes.h"
#include "VoxelBufferSplitter.h"
#include "VoxelPointNodesStats.h"
#include "VoxelPointId.h"

void FVoxelNode_SplitPoints::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelPointSet> Points = InPin.Get(Query);

	VOXEL_GRAPH_WAIT(Points)
	{
		if (Points->Num() == 0)
		{
			return;
		}

		const TValue<FVoxelBoolBuffer> Condition = ConditionPin.Get(Points->MakeQuery(Query));

		VOXEL_GRAPH_WAIT(Points, Condition)
		{
			if (!Points->CheckNum(this, Condition->Num()))
			{
				TruePin.Set(Query, Points);
				return;
			}

			FVoxelNodeStatScope StatScope(*this, Points->Num());

			if (Condition->IsConstant())
			{
				if (Condition->GetConstant())
				{
					TruePin.Set(Query, Points);
				}
				else
				{
					FalsePin.Set(Query, Points);
				}
				return;
			}

			TVoxelArray<TSharedPtr<FVoxelPointSet>> PointSets = Points->Split(FVoxelBufferSplitter(*Condition));
			if (const TSharedPtr<FVoxelPointSet> PointSet = PointSets[0])
			{
				FalsePin.Set(Query, PointSet.ToSharedRef());
			}
			if (const TSharedPtr<FVoxelPointSet> PointSet = PointSets[1])
			{
				TruePin.Set(Query, PointSet.ToSharedRef());
			}
		};
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_DensityFilter::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelPointSet> Points = InPin.Get(Query);

	VOXEL_GRAPH_WAIT(Points)
	{
		if (Points->Num() == 0)
		{
			return;
		}

		const TValue<FVoxelFloatBuffer> Densities = DensityPin.Get(Points->MakeQuery(Query));
		const TValue<FVoxelSeed> Seed = SeedPin.Get(Points->MakeQuery(Query));

		VOXEL_GRAPH_WAIT(Points, Densities, Seed)
		{
			if (!Points->CheckNum(this, Densities->Num()))
			{
				APin.Set(Query, Points);
				return;
			}

			const FVoxelPointIdBuffer* IdBuffer = Points->Find<FVoxelPointIdBuffer>(FVoxelPointAttributes::Id);
			if (!IdBuffer)
			{
				VOXEL_MESSAGE(Error, "{0}: Missing attribute Id", this);
				return;
			}

			FVoxelNodeStatScope StatScope(*this, Points->Num());

			const FVoxelPointRandom Random(Seed, STATIC_HASH("DensityFilter"));

			FVoxelBoolBuffer Condition;
			Condition.Allocate(Points->Num());

			for (int32 Index = 0; Index < Points->Num(); Index++)
			{
				const FVoxelPointId Id = (*IdBuffer)[Index];
				const float Fraction = Random.GetFraction(Id);
				Condition.Set(Index, Fraction >= (*Densities)[Index]);
			}

			TVoxelArray<TSharedPtr<FVoxelPointSet>> PointSets = Points->Split(FVoxelBufferSplitter(Condition));
			if (const TSharedPtr<FVoxelPointSet> PointSet = PointSets[0])
			{
				BPin.Set(Query, PointSet.ToSharedRef());
			}
			if (const TSharedPtr<FVoxelPointSet> PointSet = PointSets[1])
			{
				APin.Set(Query, PointSet.ToSharedRef());
			}
		};
	};
}