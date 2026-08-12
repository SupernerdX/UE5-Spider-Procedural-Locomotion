// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNode_FindNearestNeighbors.h"
#include "VoxelPointNodesStats.h"
#include "Buffer/VoxelDoubleBuffers.h"
#include "Scatter/VoxelScatterFunctionLibrary.h"

void FVoxelNode_FindNearestNeighbors::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<float> MaxDistance = MaxDistancePin.Get(Query);
	const TValue<float> BoundsExtension = BoundsExtensionPin.Get(Query);

	VOXEL_GRAPH_WAIT(MaxDistance, BoundsExtension)
	{
		// Get reference points with extended bounds to find neighbors across chunk boundaries
		const TValue<FVoxelPointSet> ReferencePoints = INLINE_LAMBDA
		{
			const FVoxelGraphParameters::FPointSetChunkBounds* BoundsParameter = Query->FindParameter<FVoxelGraphParameters::FPointSetChunkBounds>();
			if (!BoundsParameter)
			{
				return ReferencePointsPin.Get(Query);
			}

			FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
			NewQuery.RemoveParameter<FVoxelGraphParameters::FPointSetChunkBounds>();

			FVoxelGraphParameters::FPointSetChunkBounds& Parameter = NewQuery.AddParameter<FVoxelGraphParameters::FPointSetChunkBounds>(*BoundsParameter);
			Parameter.ExtendBounds(
				BoundsParameter->GetInitialBounds()
				.Extend(MaxDistance * BoundsExtension));

			return ReferencePointsPin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));
		};

		const TValue<FVoxelPointSet> QueryPoints = QueryPointsPin.Get(Query);

		VOXEL_GRAPH_WAIT(QueryPoints, ReferencePoints, MaxDistance)
		{
			const int32 NumQueryPoints = QueryPoints->Num();

			// Early exit if no query points
			if (NumQueryPoints == 0)
			{
				return;
			}

			const FVector NaNVector(
				std::numeric_limits<double>::quiet_NaN(),
				std::numeric_limits<double>::quiet_NaN(),
				std::numeric_limits<double>::quiet_NaN());

			// Handle edge case: no reference points
			if (ReferencePoints->Num() == 0)
			{
				FirstNeighborPositionPin.Set(Query, FVoxelDoubleVectorBuffer(NaNVector));
				SecondNeighborPositionPin.Set(Query, FVoxelDoubleVectorBuffer(NaNVector));
				ThirdNeighborPositionPin.Set(Query, FVoxelDoubleVectorBuffer(NaNVector));
				return;
			}

			if (MaxDistance < KINDA_SMALL_NUMBER)
			{
				VOXEL_MESSAGE(Error, "{0}: MaxDistance must be greater than 0", this);
				return;
			}

			VOXEL_SCOPE_COUNTER_FORMAT("FindNearestNeighbors QueryNum=%d RefNum=%d",
				NumQueryPoints, ReferencePoints->Num());
			FVoxelNodeStatScope StatScope(*this, NumQueryPoints);

			// Get position buffers
			const FVoxelDoubleVectorBuffer* QueryPositions =
				QueryPoints->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
			const FVoxelDoubleVectorBuffer* RefPositions =
				ReferencePoints->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);

			if (!QueryPositions)
			{
				VOXEL_MESSAGE(Error, "{0}: QueryPoints missing Position attribute", this);
				return;
			}
			if (!RefPositions)
			{
				VOXEL_MESSAGE(Error, "{0}: ReferencePoints missing Position attribute", this);
				return;
			}

			// Build spatial hash map from reference points
			const float BucketSize = MaxDistance;
			const float InvBucketSize = 1.0 / BucketSize;
			const float MaxDistanceSquared = FMath::Square(MaxDistance);

			struct FRefNode
			{
				int32 Next = -1;
				FVector Position = FVector(ForceInit);
			};
			TVoxelChunkedArray<FRefNode> RefNodes;

			struct FBucket
			{
				int32 NodeIndex = -1;
			};

			TVoxelMap<FIntVector, FBucket> SpatialMap;
			SpatialMap.Reserve(ReferencePoints->Num());

			// Populate spatial hash with reference points
			for (int32 RefIndex = 0; RefIndex < ReferencePoints->Num(); RefIndex++)
			{
				const FVector Position = (*RefPositions)[RefIndex];
				const FIntVector Key = FVoxelUtilities::FloorToInt(Position * InvBucketSize);

				FBucket& Bucket = SpatialMap.FindOrAdd(Key);
				FRefNode NewNode;
				NewNode.Next = Bucket.NodeIndex;
				NewNode.Position = Position;
				Bucket.NodeIndex = RefNodes.Add(NewNode);
			}

			// Allocate output buffers
			FVoxelDoubleVectorBuffer Neighbor0;
			FVoxelDoubleVectorBuffer Neighbor1;
			FVoxelDoubleVectorBuffer Neighbor2;
			Neighbor0.Allocate(NumQueryPoints);
			Neighbor1.Allocate(NumQueryPoints);
			Neighbor2.Allocate(NumQueryPoints);

			// For each query point, find nearest 3 neighbors
			for (int32 QueryIndex = 0; QueryIndex < NumQueryPoints; QueryIndex++)
			{
				const FVector QueryPosition = (*QueryPositions)[QueryIndex];

				// Track top 3 nearest
				struct FCandidate
				{
					float DistanceSquared = MAX_FLT;
					FVector Position = FVector(ForceInit);
				};
				FCandidate Candidates[3];

				// Get all bucket keys to check (neighborhood around query point)
				const FVector MinCheck = QueryPosition - FVector(MaxDistance);
				const FVector MaxCheck = QueryPosition + FVector(MaxDistance);
				const FIntVector MinKey = FVoxelUtilities::FloorToInt(MinCheck * InvBucketSize);
				const FIntVector MaxKey = FVoxelUtilities::FloorToInt(MaxCheck * InvBucketSize);

				for (int32 Z = MinKey.Z; Z <= MaxKey.Z; Z++)
				{
					for (int32 Y = MinKey.Y; Y <= MaxKey.Y; Y++)
					{
						for (int32 X = MinKey.X; X <= MaxKey.X; X++)
						{
							const FBucket* Bucket = SpatialMap.Find(FIntVector(X, Y, Z));
							if (!Bucket)
							{
								continue;
							}

							int32 NodeIndex = Bucket->NodeIndex;
							while (NodeIndex != -1)
							{
								const FRefNode& Node = RefNodes[NodeIndex];
								const float DistSq = FVector::DistSquared(QueryPosition, Node.Position);

								if (DistSq < MaxDistanceSquared &&
									DistSq < Candidates[2].DistanceSquared)
								{
									// Insert into sorted candidates (inline insertion sort)
									if (DistSq < Candidates[1].DistanceSquared)
									{
										if (DistSq < Candidates[0].DistanceSquared)
										{
											Candidates[2] = Candidates[1];
											Candidates[1] = Candidates[0];
											Candidates[0] = { DistSq, Node.Position };
										}
										else
										{
											Candidates[2] = Candidates[1];
											Candidates[1] = { DistSq, Node.Position };
										}
									}
									else
									{
										Candidates[2] = { DistSq, Node.Position };
									}
								}
								NodeIndex = Node.Next;
							}
						}
					}
				}

				// Set output values (NaN if not found)
				Neighbor0.Set(QueryIndex, Candidates[0].DistanceSquared < MAX_FLT ? Candidates[0].Position : NaNVector);
				Neighbor1.Set(QueryIndex, Candidates[1].DistanceSquared < MAX_FLT ? Candidates[1].Position : NaNVector);
				Neighbor2.Set(QueryIndex, Candidates[2].DistanceSquared < MAX_FLT ? Candidates[2].Position : NaNVector);
			}

			FirstNeighborPositionPin.Set(Query, MoveTemp(Neighbor0));
			SecondNeighborPositionPin.Set(Query, MoveTemp(Neighbor1));
			ThirdNeighborPositionPin.Set(Query, MoveTemp(Neighbor2));
		};
	};
}
