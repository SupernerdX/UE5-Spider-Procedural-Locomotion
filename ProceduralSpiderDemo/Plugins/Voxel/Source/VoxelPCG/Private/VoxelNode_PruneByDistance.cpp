// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNode_PruneByDistance.h"
#include "VoxelPointNodesStats.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Scatter/VoxelScatterFunctionLibrary.h"

void FVoxelNode_PruneByDistance::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<float> Distance = DistancePin.Get(Query);
	const TValue<float> BoundsExtension = BoundsExtensionPin.Get(Query);

	VOXEL_GRAPH_WAIT(Distance, BoundsExtension)
	{
		const TValue<FVoxelPointSet> Points = INLINE_LAMBDA
		{
			const FVoxelGraphParameters::FPointSetChunkBounds* BoundsParameter = Query->FindParameter<FVoxelGraphParameters::FPointSetChunkBounds>();
			if (!BoundsParameter)
			{
				return InPin.Get(Query);
			}

			FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
			NewQuery.RemoveParameter<FVoxelGraphParameters::FPointSetChunkBounds>();

			FVoxelGraphParameters::FPointSetChunkBounds& Parameter = NewQuery.AddParameter<FVoxelGraphParameters::FPointSetChunkBounds>(*BoundsParameter);
			Parameter.ExtendBounds(
				BoundsParameter->GetInitialBounds()
				.Extend(Distance * BoundsExtension));

			return InPin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));
		};

		VOXEL_GRAPH_WAIT(Points, Distance)
		{
			if (Points->Num() == 0)
			{
				return;
			}

			if (Distance < KINDA_SMALL_NUMBER)
			{
				OutPin.Set(Query, Points);
				return;
			}

			VOXEL_SCOPE_COUNTER_FORMAT("PruneByDistance Num=%d", Points->Num());
			FVoxelNodeStatScope StatScope(*this, Points->Num());

			const FVoxelDoubleVectorBuffer* PositionBuffer = Points->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
			if (!PositionBuffer)
			{
				VOXEL_MESSAGE(Error, "{0}: Missing attribute Position", this);
				return;
			}

			const float HalfDistance = Distance / 2;
			const float DistanceSquared = FMath::Square(Distance);
			const float BucketSize = Distance;
			const float InvBucketSize = 1. / BucketSize;

			struct FNode
			{
				int32 Next = -1;
				FVector Position = FVector(ForceInit);
			};
			TVoxelChunkedArray<FNode> Nodes;

			struct FBucket
			{
				int32 NodeIndex = -1;
			};

			TVoxelMap<FIntVector, FBucket> Map;
			Map.Reserve(8 * Points->Num());

			FVoxelInt32Buffer Indices;
			Indices.Allocate(Points->Num());

			int32 WriteIndex = 0;
			for (int32 Index = 0; Index < Points->Num(); Index++)
			{
				const FVector Position = (*PositionBuffer)[Index];

				TVoxelFixedArray<FIntVector, 8> Keys;
				Keys.AddUnique(FVoxelUtilities::FloorToInt((Position + FVector(-HalfDistance, -HalfDistance, -HalfDistance)) * InvBucketSize));
				Keys.AddUnique(FVoxelUtilities::FloorToInt((Position + FVector(+HalfDistance, -HalfDistance, -HalfDistance)) * InvBucketSize));
				Keys.AddUnique(FVoxelUtilities::FloorToInt((Position + FVector(-HalfDistance, +HalfDistance, -HalfDistance)) * InvBucketSize));
				Keys.AddUnique(FVoxelUtilities::FloorToInt((Position + FVector(+HalfDistance, +HalfDistance, -HalfDistance)) * InvBucketSize));
				Keys.AddUnique(FVoxelUtilities::FloorToInt((Position + FVector(-HalfDistance, -HalfDistance, +HalfDistance)) * InvBucketSize));
				Keys.AddUnique(FVoxelUtilities::FloorToInt((Position + FVector(+HalfDistance, -HalfDistance, +HalfDistance)) * InvBucketSize));
				Keys.AddUnique(FVoxelUtilities::FloorToInt((Position + FVector(-HalfDistance, +HalfDistance, +HalfDistance)) * InvBucketSize));
				Keys.AddUnique(FVoxelUtilities::FloorToInt((Position + FVector(+HalfDistance, +HalfDistance, +HalfDistance)) * InvBucketSize));

				TVoxelFixedArray<FBucket*, 8> Buckets;
				for (const FIntVector& Key : Keys)
				{
					FBucket& Bucket = Map.FindOrAdd(Key);
					int32 NodeIndex = Bucket.NodeIndex;
					while (NodeIndex != -1)
					{
						const FNode& Node = Nodes[NodeIndex];
						if (FVector::DistSquared(Node.Position, Position) < DistanceSquared)
						{
							goto Skip;
						}
						NodeIndex = Node.Next;
					}
					Buckets.Add(&Bucket);
				}

				for (FBucket* Bucket : Buckets)
				{
					FNode NewNode;
					NewNode.Next = Bucket->NodeIndex;
					NewNode.Position = Position;
					Bucket->NodeIndex = Nodes.Add(NewNode);
				}

				Indices.Set(WriteIndex++, Index);

				Skip:
					;
			}
			Indices.ShrinkTo(WriteIndex);

			FVoxelPointFilterStats::RecordNodeStats(*this, Points->Num(), Indices.Num());

			OutPin.Set(Query, Points->Gather(Indices.View()));
		};
	};
}