// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelNode_RaymarchLayerDistanceField.h"
#include "Utilities/VoxelBufferConversionUtilities.h"
#include "VoxelQuery.h"
#include "VoxelLayers.h"
#include "Graphs/VoxelStampGraphParameters.h"

void FVoxelNode_RaymarchLayerDistanceField::Compute(const FVoxelGraphQuery Query) const
{
	if (Query.IsPreview())
	{
		return;
	}

	const TValue<FVoxelPointSet> Points = InPin.Get(Query);
	const TValue<FVoxelWeakStackLayer> WeakLayer = LayerPin.Get(Query);
	const TValue<bool> UpdateRotation = UpdateRotationPin.Get(Query);
	const TValue<float> GradientStep = GradientStepPin.Get(Query);

	VOXEL_GRAPH_WAIT(Points, WeakLayer, UpdateRotation, GradientStep)
	{
		if (Points->Num() == 0)
		{
			return;
		}

		if (WeakLayer.Type == EVoxelLayerType::Height)
		{
			const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
			if (!Parameter)
			{
				VOXEL_MESSAGE(Error, "{0}: Cannot call here", this);
				return;
			}
			const FVoxelQuery& VoxelQuery = Parameter->Query;

			if (!VoxelQuery.CheckNoRecursion(WeakLayer) ||
				!VoxelQuery.Layers.HasLayer(WeakLayer, Query->Context.DependencyCollector))
			{
				return;
			}

			const FVoxelDoubleVectorBuffer* PositionBuffer = Points->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
			if (!PositionBuffer)
			{
				VOXEL_MESSAGE(Error, "{0}: Missing attribute Position", this);
				return;
			}

			const FVoxelQuaternionBuffer* RotationBuffer = Points->Find<FVoxelQuaternionBuffer>(FVoxelPointAttributes::Rotation);
			if (!RotationBuffer)
			{
				VOXEL_MESSAGE(Error, "{0}: Missing attribute Rotation", this);
				return;
			}

			if (UpdateRotation)
			{
				OutPin.Set(Query, Project2DWithRotation(
					*Points,
					*PositionBuffer,
					*RotationBuffer,
					VoxelQuery,
					WeakLayer,
					GradientStep));
			}
			else
			{
				OutPin.Set(Query, Project2D(
					*Points,
					*PositionBuffer,
					VoxelQuery,
					WeakLayer));
			}

			return;
		}
		check(WeakLayer.Type == EVoxelLayerType::Volume);

		const TValue<bool> ForceDirection = ForceDirectionPin.Get(Query);
		const TValue<float> KillDistance = KillDistancePin.Get(Query);
		const TValue<float> Tolerance = TolerancePin.Get(Query);
		const TValue<int32> MaxSteps = MaxStepsPin.Get(Query);
		const TValue<float> Speed = SpeedPin.Get(Query);

		VOXEL_GRAPH_WAIT(Points, WeakLayer, UpdateRotation, ForceDirection, KillDistance, Tolerance, MaxSteps, Speed, GradientStep)
		{
			const FVoxelGraphParameters::FQuery* Parameter = Query->FindParameter<FVoxelGraphParameters::FQuery>();
			if (!Parameter)
			{
				VOXEL_MESSAGE(Error, "{0}: Cannot call here", this);
				return;
			}
			const FVoxelQuery& VoxelQuery = Parameter->Query;

			if (!VoxelQuery.CheckNoRecursion(WeakLayer) ||
				!VoxelQuery.Layers.HasLayer(WeakLayer, Query->Context.DependencyCollector))
			{
				return;
			}

			const FVoxelDoubleVectorBuffer* PositionBuffer = Points->Find<FVoxelDoubleVectorBuffer>(FVoxelPointAttributes::Position);
			if (!PositionBuffer)
			{
				VOXEL_MESSAGE(Error, "{0}: Missing attribute Position", this);
				return;
			}

			const FVoxelQuaternionBuffer* RotationBuffer = Points->Find<FVoxelQuaternionBuffer>(FVoxelPointAttributes::Rotation);
			if (!RotationBuffer)
			{
				VOXEL_MESSAGE(Error, "{0}: Missing attribute Rotation", this);
				return;
			}

			OutPin.Set(Query, Project3D(
				*Points,
				*PositionBuffer,
				*RotationBuffer,
				VoxelQuery,
				WeakLayer,
				UpdateRotation,
				ForceDirection,
				KillDistance,
				Tolerance,
				MaxSteps,
				Speed,
				GradientStep));
		};
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelPointSet> FVoxelNode_RaymarchLayerDistanceField::Project2D(
	const FVoxelPointSet& Points,
	FVoxelDoubleVectorBuffer Positions,
	const FVoxelQuery& Query,
	const FVoxelWeakStackLayer& WeakLayer)
{
	VOXEL_FUNCTION_COUNTER();
	check(Positions.IsConstant() || Positions.Num() == Points.Num());

	Positions.ExpandConstants(Points.Num());

	FVoxelDoubleVector2DBuffer Positions2D;
	Positions2D.X = Positions.X;
	Positions2D.Y = Positions.Y;

	const FVoxelFloatBuffer Heights = Query.SampleHeightLayer(WeakLayer, Positions2D);

	FVoxelDoubleVectorBuffer NewPositions;
	NewPositions.X = Positions.X;
	NewPositions.Y = Positions.Y;
	NewPositions.Z = FVoxelBufferConversionUtilities::FloatToDouble(Heights);

	const TSharedRef<FVoxelPointSet> NewPoints = Points.MakeSharedCopy();
	NewPoints->Add(FVoxelPointAttributes::Position, MoveTemp(NewPositions));
	return NewPoints;
}

TSharedRef<FVoxelPointSet> FVoxelNode_RaymarchLayerDistanceField::Project2DWithRotation(
	const FVoxelPointSet& Points,
	const FVoxelDoubleVectorBuffer& PositionBuffer,
	const FVoxelQuaternionBuffer& RotationBuffer,
	const FVoxelQuery& Query,
	const FVoxelWeakStackLayer& WeakLayer,
	const float GradientStep)
{
	VOXEL_FUNCTION_COUNTER();
	check(PositionBuffer.IsConstant() || PositionBuffer.Num() == Points.Num());

	FVoxelQuaternionBuffer Rotations;
	Rotations.Allocate(Points.Num());

	FVoxelDoubleVector2DBuffer Positions;

	{
		VOXEL_SCOPE_COUNTER("Set positions");

		Positions.Allocate(5 * Points.Num());

		const float HalfGradientStep = GradientStep / 2.f;

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			const FVector Position = PositionBuffer[Index];

			Positions.Set(5 * Index + 0, FVector2D(Position.X - HalfGradientStep, Position.Y));
			Positions.Set(5 * Index + 1, FVector2D(Position.X + HalfGradientStep, Position.Y));
			Positions.Set(5 * Index + 2, FVector2D(Position.X, Position.Y - HalfGradientStep));
			Positions.Set(5 * Index + 3, FVector2D(Position.X, Position.Y + HalfGradientStep));
			Positions.Set(5 * Index + 4, FVector2D(Position.X, Position.Y));
		}
	}

	const FVoxelFloatBuffer Heights = Query.SampleHeightLayer(WeakLayer, Positions);

	FVoxelFloatBuffer HeightBuffer;
	HeightBuffer.Allocate(PositionBuffer.Num());

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		const float HeightMinX = Heights[5 * Index + 0];
		const float HeightMaxX = Heights[5 * Index + 1];
		const float HeightMinY = Heights[5 * Index + 2];
		const float HeightMaxY = Heights[5 * Index + 3];
		const float Height = Heights[5 * Index + 4];

		const FVector3f NewUpVector = FVector3f(
			(HeightMaxX - HeightMinX) / -GradientStep,
			(HeightMaxY - HeightMinY) / -GradientStep,
			1.f).GetSafeNormal();

		const FQuat4f NewRotation = FRotationMatrix44f::MakeFromZX(
			NewUpVector,
			RotationBuffer[Index].GetAxisX()).ToQuat();

		Rotations.Set(Index, NewRotation);
		HeightBuffer.Set(Index, Height);
	}

	FVoxelDoubleVectorBuffer NewPositions;
	NewPositions.X = PositionBuffer.X;
	NewPositions.Y = PositionBuffer.Y;
	NewPositions.Z = FVoxelBufferConversionUtilities::FloatToDouble(HeightBuffer);

	const TSharedRef<FVoxelPointSet> NewPoints = Points.MakeSharedCopy();
	NewPoints->Add(FVoxelPointAttributes::Position, MoveTemp(NewPositions));
	NewPoints->Add(FVoxelPointAttributes::Rotation, MoveTemp(Rotations));
	return NewPoints;
}

TSharedRef<FVoxelPointSet> FVoxelNode_RaymarchLayerDistanceField::Project3D(
	const FVoxelPointSet& Points,
	const FVoxelDoubleVectorBuffer& PositionBuffer,
	const FVoxelQuaternionBuffer& RotationBuffer,
	const FVoxelQuery& Query,
	const FVoxelWeakStackLayer& WeakLayer,
	const bool bUpdateRotation,
	const bool bForceDirection,
	const float KillDistance,
	const float Tolerance,
	const int32 MaxSteps,
	const float Speed,
	const float GradientStep)
{
	VOXEL_FUNCTION_COUNTER();
	check(PositionBuffer.IsConstant() || PositionBuffer.Num() == Points.Num());
	check(RotationBuffer.IsConstant() || RotationBuffer.Num() == Points.Num());

	FVoxelDoubleVectorBuffer NewPositions;
	NewPositions.Allocate(Points.Num());
	NewPositions.CopyFrom(PositionBuffer, 0, 0, Points.Num());

	FVoxelQuaternionBuffer NewRotations;
	if (bUpdateRotation)
	{
		NewRotations.Allocate(Points.Num());
		NewRotations.CopyFrom(RotationBuffer, 0, 0, Points.Num());
	}

	TVoxelArray<int32> PointsAlive;
	{
		VOXEL_SCOPE_COUNTER("Initialize PointsAlive");

		FVoxelUtilities::SetNumFast(PointsAlive, Points.Num());

		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			PointsAlive[Index] = Index;
		}
	}

	TVoxelArray<FVector3f> Normals;
	if (bForceDirection)
	{
		FVoxelUtilities::SetNumFast(Normals, Points.Num());
		for (int32 Index = 0; Index < Points.Num(); Index++)
		{
			Normals[Index] = RotationBuffer[Index].GetUpVector();
		}
	}

	FVoxelDoubleVectorBuffer Positions;

	for (int32 Step = 0; Step < MaxSteps; Step++)
	{
		VOXEL_SCOPE_COUNTER("Iteration");

		const TVoxelArrayView<double> NewPositionsX = NewPositions.X.View();
		const TVoxelArrayView<double> NewPositionsY = NewPositions.Y.View();
		const TVoxelArrayView<double> NewPositionsZ = NewPositions.Z.View();

		const TVoxelArrayView<float> NewRotationsX = NewRotations.X.View();
		const TVoxelArrayView<float> NewRotationsY = NewRotations.Y.View();
		const TVoxelArrayView<float> NewRotationsZ = NewRotations.Z.View();
		const TVoxelArrayView<float> NewRotationsW = NewRotations.W.View();

		{
			VOXEL_SCOPE_COUNTER("Set positions");

			Positions.Allocate(7 * PointsAlive.Num());

			const float HalfGradientStep = GradientStep / 2.f;

			for (int32 Index = 0; Index < PointsAlive.Num(); Index++)
			{
				const int32 NewPositionsIndex = PointsAlive[Index];

				const float NewPositionX = NewPositionsX[NewPositionsIndex];
				const float NewPositionY = NewPositionsY[NewPositionsIndex];
				const float NewPositionZ = NewPositionsZ[NewPositionsIndex];

				Positions.Set(7 * Index + 0, FVector(NewPositionX - HalfGradientStep, NewPositionY, NewPositionZ));
				Positions.Set(7 * Index + 1, FVector(NewPositionX + HalfGradientStep, NewPositionY, NewPositionZ));
				Positions.Set(7 * Index + 2, FVector(NewPositionX, NewPositionY - HalfGradientStep, NewPositionZ));
				Positions.Set(7 * Index + 3, FVector(NewPositionX, NewPositionY + HalfGradientStep, NewPositionZ));
				Positions.Set(7 * Index + 4, FVector(NewPositionX, NewPositionY, NewPositionZ - HalfGradientStep));
				Positions.Set(7 * Index + 5, FVector(NewPositionX, NewPositionY, NewPositionZ + HalfGradientStep));
				Positions.Set(7 * Index + 6, FVector(NewPositionX, NewPositionY, NewPositionZ));
			}
		}

		const FVoxelFloatBuffer Distances = Query.SampleVolumeLayer(WeakLayer, Positions);

		for (int32 Index = 0; Index < PointsAlive.Num(); Index++)
		{
			const float DistanceMinX = Distances[7 * Index + 0];
			const float DistanceMaxX = Distances[7 * Index + 1];
			const float DistanceMinY = Distances[7 * Index + 2];
			const float DistanceMaxY = Distances[7 * Index + 3];
			const float DistanceMinZ = Distances[7 * Index + 4];
			const float DistanceMaxZ = Distances[7 * Index + 5];
			const float Distance = Distances[7 * Index + 6];
			const float AbsDistance = FMath::Abs(Distance);

			int32& PointIndex = PointsAlive[Index];

			if (FVoxelUtilities::IsNaN(Distance))
			{
				PointIndex = -1;
				continue;
			}

			if (Step == MaxSteps - 1 &&
				AbsDistance > KillDistance)
			{
				PointIndex = -1;
				continue;
			}

			if (AbsDistance < Tolerance ||
				Step == MaxSteps - 1)
			{
				if (bUpdateRotation)
				{
					const FVector NewUpVector = FVector(
						DistanceMaxX - DistanceMinX,
						DistanceMaxY - DistanceMinY,
						DistanceMaxZ - DistanceMinZ).GetSafeNormal();

					const FQuat CurrentRotation = FQuat(
						NewRotationsX[PointIndex],
						NewRotationsY[PointIndex],
						NewRotationsZ[PointIndex],
						NewRotationsW[PointIndex]);

					const FQuat NewRotation = FRotationMatrix::MakeFromZX(
						NewUpVector,
						CurrentRotation.GetAxisX()).ToQuat();

					NewRotationsX[PointIndex] = NewRotation.X;
					NewRotationsY[PointIndex] = NewRotation.Y;
					NewRotationsZ[PointIndex] = NewRotation.Z;
					NewRotationsW[PointIndex] = NewRotation.W;
				}

				PointIndex = -1;
				continue;
			}

			const FVector3f Gradient =
				FVector3f(
					DistanceMaxX - DistanceMinX,
					DistanceMaxY - DistanceMinY,
					DistanceMaxZ - DistanceMinZ) / GradientStep;

			FVector3f Delta;

			if (bForceDirection)
			{
				const FVector3f Normal = Normals[PointIndex];
				const float Value = FVector3f::DotProduct(Gradient.GetSafeNormal(), Normal.GetSafeNormal()) * Distance * Speed;

				Delta = Normal * Value;
			}
			else
			{
				Delta = Gradient.GetSafeNormal() * Distance * Speed;
			}

			NewPositionsX[PointIndex] -= Delta.X;
			NewPositionsY[PointIndex] -= Delta.Y;
			NewPositionsZ[PointIndex] -= Delta.Z;
		}

		if (Step == MaxSteps - 1)
		{
			break;
		}

		PointsAlive.RemoveAllSwap([](const int32 Index)
		{
			return Index == -1;
		});

		if (PointsAlive.Num() == 0)
		{
			break;
		}
	}

	const TSharedRef<FVoxelPointSet> NewPoints = Points.MakeSharedCopy();
	NewPoints->Add(FVoxelPointAttributes::Position, MoveTemp(NewPositions));

	if (bUpdateRotation)
	{
		NewPoints->Add(FVoxelPointAttributes::Rotation, MoveTemp(NewRotations));
	}

	return NewPoints;
}