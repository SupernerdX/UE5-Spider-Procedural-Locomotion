// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNode_ScatterPoints.h"
#include "VoxelPointId.h"
#include "Buffer/VoxelFloatBuffers.h"
#include "Buffer/VoxelDoubleBuffers.h"

void FVoxelNode_ScatterPoints::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelPointSet> Points = InPin.Get(Query);
	const TValue<FVoxelFloatRange> Radius = RadiusPin.Get(Query);
	const TValue<float> RadialOffset = RadialOffsetPin.Get(Query);
	const TValue<FVoxelInt32Range> NumPoints = NumPointsPin.Get(Query);
	const TValue<FVoxelSeed> Seed = SeedPin.Get(Query);

	VOXEL_GRAPH_WAIT(Points, Radius, RadialOffset, NumPoints, Seed)
	{
		if (Points->Num() == 0)
		{
			return;
		}

		const float HalfRadialOffset = FMath::DegreesToRadians(RadialOffset) / 2;
		const FVoxelFloatRange RadialOffsetRange = FVoxelFloatRange(-HalfRadialOffset, HalfRadialOffset);

		FVoxelNodeStatScope StatScope(*this, Points->Num());

		const FVoxelPointIdBuffer* ParentIds = Points->Find<FVoxelPointIdBuffer>(FVoxelPointAttributes::Id);
		if (!ParentIds)
		{
			VOXEL_MESSAGE(Error, "{0}: Missing attribute Id", this);
			return;
		}

		const FVoxelDoubleVectorBuffer* ParentPositions = Points->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
		if (!ParentPositions)
		{
			VOXEL_MESSAGE(Error, "{0}: Missing attribute Position", this);
			return;
		}

		const FVoxelQuaternionBuffer* ParentRotations = Points->Find<FVoxelQuaternionBuffer>(FVoxelPointAttributes::Rotation);

		const FVoxelPointRandom ChildIdRandom(Seed, STATIC_HASH("RadialPointSpawner_ChildId"));
		const FVoxelPointRandom RadiusRandom(Seed, STATIC_HASH("RadialPointSpawner_Radius"));
		const FVoxelPointRandom RadialOffsetRandom(Seed, STATIC_HASH("RadialPointSpawner_RadialOffset"));
		const FVoxelPointRandom NumPointsRandom(Seed, STATIC_HASH("RadialPointSpawner_NumPoints"));

		TVoxelArray<int32> AllNumChildren;
		FVoxelUtilities::SetNumFast(AllNumChildren, Points->Num());

		int32 TotalNum = 0;
		{
			VOXEL_SCOPE_COUNTER("AllNumChildren");

			for (int32 ParentIndex = 0; ParentIndex < Points->Num(); ParentIndex++)
			{
				int32 NumChildren = NumPoints.Interpolate(NumPointsRandom.GetFraction((*ParentIds)[ParentIndex]));
				if (NumChildren > 1000)
				{
					VOXEL_MESSAGE(Error, "{0}: More than 1000 child points generated, skipping", this);
					NumChildren = 0;
				}
				NumChildren = FMath::Max(NumChildren, 0);

				AllNumChildren[ParentIndex] = NumChildren;
				TotalNum += NumChildren;
			}
		}

		if (TotalNum == 0)
		{
			return;
		}

		struct FParentInfo
		{
			int32 ParentIndex = 0;
			int32 ChildIndex = 0;
			int32 NumChildren = 0;
			FVector X = FVector(ForceInit);
			FVector Y = FVector(ForceInit);
		};
		TVoxelArray<FParentInfo> ParentInfos;
		FVoxelUtilities::SetNumFast(ParentInfos, TotalNum);
		{
			VOXEL_SCOPE_COUNTER("ParentInfos");

			int32 Index = 0;
			for (int32 ParentIndex = 0; ParentIndex < Points->Num(); ParentIndex++)
			{
				const int32 NumChildren = AllNumChildren[ParentIndex];
				if (NumChildren == 0)
				{
					continue;
				}

				const FVector Normal =
					ParentRotations
					? FVector((*ParentRotations)[ParentIndex].GetUpVector())
					: FVector::UpVector;

				FVector X = FMath::Abs(Normal.X) > 0.9f
					? FVector(0, 1, 0)
					: FVector(1, 0, 0);

				const FVector Y = FVector::CrossProduct(Normal, X).GetSafeNormal();
				X = FVector::CrossProduct(Y, Normal).GetSafeNormal();

				for (int32 ChildIndex = 0; ChildIndex < NumChildren; ChildIndex++)
				{
					ParentInfos[Index + ChildIndex] = FParentInfo
					{
						ParentIndex,
						ChildIndex,
						NumChildren,
						X,
						Y
					};
				}

				Index += NumChildren;
			}
			ensure(Index == TotalNum);
		}

		FVoxelPointIdBuffer Id;
		FVoxelDoubleVectorBuffer Position;

		Id.Allocate(TotalNum);
		Position.Allocate(TotalNum);

		for (int32 ChildIndex = 0; ChildIndex < TotalNum; ChildIndex++)
		{
			const FParentInfo ParentInfo = ParentInfos[ChildIndex];

			const FVoxelPointId ParentId = (*ParentIds)[ParentInfo.ParentIndex];
			const FVector ParentPosition = (*ParentPositions)[ParentInfo.ParentIndex];

			const FVoxelPointId ChildId = ChildIdRandom.MakeId(ParentId, ParentInfo.ChildIndex);

			float AngleInRadians = UE_TWO_PI * ParentInfo.ChildIndex / float(ParentInfo.NumChildren);
			AngleInRadians += RadialOffsetRange.Interpolate(RadialOffsetRandom.GetFraction(ChildId));

			const float ChildRadius = Radius.Interpolate(RadiusRandom.GetFraction(ChildId));
			const FVector2D RelativePosition = FVector2D(FMath::Cos(AngleInRadians), FMath::Sin(AngleInRadians)) * ChildRadius;

			const FVector NewPosition =
				ParentPosition +
				ParentInfo.X * RelativePosition.X +
				ParentInfo.Y * RelativePosition.Y;

			Id.Set(ChildIndex, ChildId);
			Position.Set(ChildIndex, NewPosition);
		}

		const TSharedRef<FVoxelPointSet> Result = MakeShared<FVoxelPointSet>();
		Result->SetNum(TotalNum);

		TVoxelMap<FName, TSharedPtr<const FVoxelBuffer>> NewBuffers;
		NewBuffers.Reserve(Points->GetAttributes().Num());

		for (const auto& It : Points->GetAttributes())
		{
			const TSharedRef<const FVoxelBuffer> Buffer = It.Value->Replicate(AllNumChildren, TotalNum);
			NewBuffers.Add_EnsureNew(It.Key, Buffer);
		}

		// First add all existing buffers
		for (const auto& It : NewBuffers)
		{
			Result->Add(It.Key, It.Value.ToSharedRef());
		}

		// Then add parents, potentially overriding existing buffers
		// eg, we might have a new Parent Position replacing the previous one
		for (const auto& It : NewBuffers)
		{
			Result->Add(FVoxelPointAttributes::MakeParent(It.Key), It.Value.ToSharedRef());
		}

		Result->Add(FVoxelPointAttributes::Id, MoveTemp(Id));
		Result->Add(FVoxelPointAttributes::Position, MoveTemp(Position));

		OutPin.Set(Query, Result);
	};
}