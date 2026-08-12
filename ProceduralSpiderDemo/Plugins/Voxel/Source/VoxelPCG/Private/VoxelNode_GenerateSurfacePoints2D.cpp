// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNode_GenerateSurfacePoints2D.h"
#include "VoxelGraphPositionParameter.h"
#include "Utilities/VoxelBufferConversionUtilities.h"
#include "VoxelNode_GenerateSurfacePoints2DImpl.ispc.generated.h"

void FVoxelNode_Generate2DPoints::Compute(const FVoxelGraphQuery Query) const
{
	const TValue<FVoxelBox> Bounds = BoundsPin.Get(Query);
	const TValue<float> CellSize = CellSizePin.Get(Query);
	const TValue<float> Jitter = JitterPin.Get(Query);
	const TValue<FVoxelSeed> Seed = SeedPin.Get(Query);

	VOXEL_GRAPH_WAIT(Bounds, CellSize, Jitter, Seed)
	{
		if (!Bounds.IsValidAndNotEmpty())
		{
			VOXEL_MESSAGE(Error, "{0}: Invalid bounds", this);
			return;
		}
		if (Bounds.IsInfinite())
		{
			VOXEL_MESSAGE(Error, "{0}: Infinite bounds", this);
			return;
		}
		if (CellSize == 0.f)
		{
			VOXEL_MESSAGE(Error, "{0}: Cell Size cannot be 0", this);
			return;
		}

		const int32 NumCellsX = (Bounds.Max.X - Bounds.Min.X) / CellSize;
		const int32 NumCellsY = (Bounds.Max.Y - Bounds.Min.Y) / CellSize;

		FVoxelNodeStatScope StatScope(*this, NumCellsX * NumCellsY);

		if (NumCellsX * NumCellsY == 0)
		{
			return;
		}

		const TSharedRef<FVoxelDoubleVector2DBuffer> Positions = MakeShared<FVoxelDoubleVector2DBuffer>();
		const TSharedRef<FVoxelPointIdBuffer> Ids = MakeShared<FVoxelPointIdBuffer>();
		GeneratePositions(
			Bounds,
			NumCellsX,
			NumCellsY,
			CellSize,
			Jitter,
			Seed,
			*Positions,
			*Ids);

		FVoxelGraphQueryImpl& NewQuery = Query->CloneParameters();
		NewQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetLocalPosition(*Positions);
		const TValue<FVoxelFloatBuffer> Heights = HeightPin.Get(FVoxelGraphQuery(NewQuery, Query.GetCallstack()));

		VOXEL_GRAPH_WAIT(CellSize, Ids, Positions, Heights, Bounds)
		{
			TSharedPtr<FVoxelPointIdBuffer> FilteredIds;
			const TSharedRef<FVoxelDoubleVectorBuffer> FilteredPositions = MakeShared<FVoxelDoubleVectorBuffer>();
			if (Heights->IsConstant())
			{
				if (Heights->GetConstant() < Bounds.Min.Z ||
					Heights->GetConstant() > Bounds.Max.Z)
				{
					return;
				}

				FilteredIds = Ids;

				FilteredPositions->X = Positions->X;
				FilteredPositions->Y = Positions->Y;
				FilteredPositions->Z = FVoxelBufferConversionUtilities::FloatToDouble(*Heights);
			}
			else
			{
				FilteredIds = MakeShared<FVoxelPointIdBuffer>();
				FilteredIds->Allocate(Heights->Num());
				FilteredPositions->Allocate(Heights->Num());

				int32 WriteIndex = 0;
				for (int32 Index = 0; Index < Heights->Num(); Index++)
				{
					const float Height = (*Heights)[Index];
					if (Height < Bounds.Min.Z ||
						Height > Bounds.Max.Z)
					{
						continue;
					}

					FilteredIds->Set(WriteIndex, (*Ids)[Index]);
					FilteredPositions->X.Set(WriteIndex, Positions->X[Index]);
					FilteredPositions->Y.Set(WriteIndex, Positions->Y[Index]);
					FilteredPositions->Z.Set(WriteIndex, Height);
					WriteIndex++;
				}

				FilteredIds->ShrinkTo(WriteIndex);
				FilteredPositions->ShrinkTo(WriteIndex);
			}

			if (FilteredPositions->Num() == 0)
			{
				return;
			}

			const TSharedRef<FVoxelDoubleVector2DBuffer> GradientPositions = MakeShared<FVoxelDoubleVector2DBuffer>();
			GenerateGradientPositions(
				*FilteredPositions,
				CellSize,
				*GradientPositions);

			FVoxelGraphQueryImpl& GradientQuery = Query->CloneParameters();
			GradientQuery.AddParameter<FVoxelGraphParameters::FPosition2D>().SetLocalPosition(*GradientPositions);
			TValue<FVoxelFloatBuffer> GradientHeights = HeightPin.Get(FVoxelGraphQuery(GradientQuery, Query.GetCallstack()));

			VOXEL_GRAPH_WAIT(CellSize, FilteredIds, FilteredPositions, GradientPositions, GradientHeights)
			{
				const TSharedRef<FVoxelPointSet> PointSet = MakeShared<FVoxelPointSet>();
				PointSet->SetNum(FilteredPositions->Num());
				PointSet->Add(FVoxelPointAttributes::Id, FilteredIds.ToSharedRef());
				PointSet->Add(FVoxelPointAttributes::Position, FilteredPositions);
				PointSet->Add(FVoxelPointAttributes::Rotation, ComputeGradient(*GradientHeights, CellSize));
				OutPin.Set(Query, PointSet);
			};
		};
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelNode_Generate2DPoints::GeneratePositions(
	const FVoxelBox& Bounds,
	const int32 NumCellsX,
	const int32 NumCellsY,
	const float CellSize,
	const float Jitter,
	const FVoxelSeed& Seed,
	FVoxelDoubleVector2DBuffer& OutPositions,
	FVoxelPointIdBuffer& OutIds)
{
	VOXEL_FUNCTION_COUNTER_NUM(NumCellsX * NumCellsY, 32);

	OutIds.Allocate(NumCellsX * NumCellsY);
	OutPositions.Allocate(NumCellsX * NumCellsY);

	{
		VOXEL_SCOPE_COUNTER("Compute positions");

		const float MinX = Bounds.Min.X;
		const float MinY = Bounds.Min.Y;
		const float RelativeJitter = CellSize * Jitter;
		const FVoxelPointRandom RandomX(Seed, STATIC_HASH("GenerateSurfacePoints2D_JitterX"));
		const FVoxelPointRandom RandomY(Seed, STATIC_HASH("GenerateSurfacePoints2D_JitterY"));

		const uint64 BaseHash = FVoxelUtilities::MurmurHashMulti(
			FVoxelUtilities::RoundToInt32(Bounds.Min.X),
			FVoxelUtilities::RoundToInt32(Bounds.Min.Y),
			FVoxelUtilities::RoundToInt32(Bounds.Min.Z));

		for (int32 X = 0; X < NumCellsX; X++)
		{
			for (int32 Y = 0; Y < NumCellsY; Y++)
			{
				const float PositionX = MinX + X * CellSize;
				const float PositionY = MinY + Y * CellSize;

				const FVoxelPointId Id = FVoxelUtilities::MurmurHashMulti(PositionX, PositionY) ^ BaseHash;

				const int32 Index = FVoxelUtilities::Get2DIndex<int32>(NumCellsX, NumCellsY, X, Y);

				OutIds.Set(Index, Id);

				OutPositions.X.Set(Index, PositionX + RelativeJitter * (2.f * RandomX.GetFraction(Id) - 1.f));
				OutPositions.Y.Set(Index, PositionY + RelativeJitter * (2.f * RandomY.GetFraction(Id) - 1.f));
			}
		}
	}
}

void FVoxelNode_Generate2DPoints::GenerateGradientPositions(
	const FVoxelDoubleVectorBuffer& Positions,
	const float CellSize,
	FVoxelDoubleVector2DBuffer& OutPositions)
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num() * 4, 32);

	const int32 GradientNum = Positions.Num() * 4;

	OutPositions.Allocate(GradientNum);

	ispc::VoxelNode_GenerateSurfacePoints2D_SplitPositions(
		Positions.X.GetData(),
		Positions.Y.GetData(),
		GradientNum,
		CellSize / 2.f,
		OutPositions.X.GetData(),
		OutPositions.Y.GetData());
}

TSharedRef<FVoxelQuaternionBuffer> FVoxelNode_Generate2DPoints::ComputeGradient(
	const FVoxelFloatBuffer& Heights,
	const float CellSize)
{
	VOXEL_FUNCTION_COUNTER_NUM(Heights.Num(), 32);

	if (Heights.IsConstant())
	{
		return MakeShared<FVoxelQuaternionBuffer>(FQuat4f::Identity);
	}

	check(Heights.Num() % 4 == 0);
	const int32 InputNum = Heights.Num() / 4;

	TSharedRef<FVoxelQuaternionBuffer> Rotation = MakeShared<FVoxelQuaternionBuffer>();
	Rotation->Allocate(InputNum);

	ispc::VoxelNode_GenerateSurfacePoints2D_Collapse(
		Heights.GetData(),
		InputNum,
		CellSize,
		Rotation->X.GetData(),
		Rotation->Y.GetData(),
		Rotation->Z.GetData(),
		Rotation->W.GetData());

	return Rotation;
}