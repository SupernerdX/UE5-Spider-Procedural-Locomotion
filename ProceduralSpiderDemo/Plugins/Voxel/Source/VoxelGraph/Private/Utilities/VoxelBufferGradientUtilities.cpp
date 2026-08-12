// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Utilities/VoxelBufferGradientUtilities.h"
#include "VoxelBufferAccessor.h"

FVoxelVector2DBuffer FVoxelBufferGradientUtilities::SplitPositions2D(
	const FVoxelVector2DBuffer& Positions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num(), 1024);

	FVoxelVector2DBuffer Result;
	Result.Allocate(4 * Positions.Num());

	const float HalfStep = Step / 2.f;

	const int32 NumPositions = Positions.Num();
	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		Result.X.Set(4 * Index + 0, Positions.X[Index] - HalfStep);
		Result.X.Set(4 * Index + 1, Positions.X[Index] + HalfStep);
		Result.X.Set(4 * Index + 2, Positions.X[Index]);
		Result.X.Set(4 * Index + 3, Positions.X[Index]);

		Result.Y.Set(4 * Index + 0, Positions.Y[Index]);
		Result.Y.Set(4 * Index + 1, Positions.Y[Index]);
		Result.Y.Set(4 * Index + 2, Positions.Y[Index] - HalfStep);
		Result.Y.Set(4 * Index + 3, Positions.Y[Index] + HalfStep); 
	}

	return Result;
}

FVoxelDoubleVector2DBuffer FVoxelBufferGradientUtilities::SplitPositions2D(
	const FVoxelDoubleVector2DBuffer& Positions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num(), 1024);

	FVoxelDoubleVector2DBuffer Result;
	Result.Allocate(4 * Positions.Num());

	const float HalfStep = Step / 2.f;

	const int32 NumPositions = Positions.Num();
	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		Result.X.Set(4 * Index + 0, Positions.X[Index] - HalfStep);
		Result.X.Set(4 * Index + 1, Positions.X[Index] + HalfStep);
		Result.X.Set(4 * Index + 2, Positions.X[Index]);
		Result.X.Set(4 * Index + 3, Positions.X[Index]);

		Result.Y.Set(4 * Index + 0, Positions.Y[Index]);
		Result.Y.Set(4 * Index + 1, Positions.Y[Index]);
		Result.Y.Set(4 * Index + 2, Positions.Y[Index] - HalfStep);
		Result.Y.Set(4 * Index + 3, Positions.Y[Index] + HalfStep); 
	}

	return Result;
}

FVoxelVector2DBuffer FVoxelBufferGradientUtilities::SplitPositions2D(
	const FVoxelVectorBuffer& Positions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num(), 1024);

	FVoxelVector2DBuffer Result;
	Result.Allocate(4 * Positions.Num());

	const float HalfStep = Step / 2.f;

	const int32 NumPositions = Positions.Num();
	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		Result.X.Set(4 * Index + 0, Positions.X[Index] - HalfStep);
		Result.X.Set(4 * Index + 1, Positions.X[Index] + HalfStep);
		Result.X.Set(4 * Index + 2, Positions.X[Index]);
		Result.X.Set(4 * Index + 3, Positions.X[Index]);

		Result.Y.Set(4 * Index + 0, Positions.Y[Index]);
		Result.Y.Set(4 * Index + 1, Positions.Y[Index]);
		Result.Y.Set(4 * Index + 2, Positions.Y[Index] - HalfStep);
		Result.Y.Set(4 * Index + 3, Positions.Y[Index] + HalfStep); 
	}

	return Result;
}

FVoxelDoubleVector2DBuffer FVoxelBufferGradientUtilities::SplitPositions2D(
	const FVoxelDoubleVectorBuffer& Positions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num(), 1024);

	FVoxelDoubleVector2DBuffer Result;
	Result.Allocate(4 * Positions.Num());

	const float HalfStep = Step / 2.f;

	const int32 NumPositions = Positions.Num();
	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		Result.X.Set(4 * Index + 0, Positions.X[Index] - HalfStep);
		Result.X.Set(4 * Index + 1, Positions.X[Index] + HalfStep);
		Result.X.Set(4 * Index + 2, Positions.X[Index]);
		Result.X.Set(4 * Index + 3, Positions.X[Index]);

		Result.Y.Set(4 * Index + 0, Positions.Y[Index]);
		Result.Y.Set(4 * Index + 1, Positions.Y[Index]);
		Result.Y.Set(4 * Index + 2, Positions.Y[Index] - HalfStep);
		Result.Y.Set(4 * Index + 3, Positions.Y[Index] + HalfStep); 
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVectorBuffer FVoxelBufferGradientUtilities::SplitPositions3D(
	const FVoxelVectorBuffer& Positions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num(), 1024);

	FVoxelVectorBuffer Result;
	Result.Allocate(6 * Positions.Num());

	const float HalfStep = Step / 2.f;

	const int32 NumPositions = Positions.Num();
	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		Result.X.Set(6 * Index + 0, Positions.X[Index] - HalfStep);
		Result.X.Set(6 * Index + 1, Positions.X[Index] + HalfStep);
		Result.X.Set(6 * Index + 2, Positions.X[Index]);
		Result.X.Set(6 * Index + 3, Positions.X[Index]);
		Result.X.Set(6 * Index + 4, Positions.X[Index]);
		Result.X.Set(6 * Index + 5, Positions.X[Index]);

		Result.Y.Set(6 * Index + 0, Positions.Y[Index]);
		Result.Y.Set(6 * Index + 1, Positions.Y[Index]);
		Result.Y.Set(6 * Index + 2, Positions.Y[Index] - HalfStep);
		Result.Y.Set(6 * Index + 3, Positions.Y[Index] + HalfStep); 
		Result.Y.Set(6 * Index + 4, Positions.Y[Index]);
		Result.Y.Set(6 * Index + 5, Positions.Y[Index]);


		Result.Z.Set(6 * Index + 0, Positions.Z[Index]);
		Result.Z.Set(6 * Index + 1, Positions.Z[Index]);
		Result.Z.Set(6 * Index + 2, Positions.Z[Index]);
		Result.Z.Set(6 * Index + 3, Positions.Z[Index]); 
		Result.Z.Set(6 * Index + 4, Positions.Z[Index] - HalfStep);
		Result.Z.Set(6 * Index + 5, Positions.Z[Index] + HalfStep);
	}

	return Result;
}

FVoxelDoubleVectorBuffer FVoxelBufferGradientUtilities::SplitPositions3D(
	const FVoxelDoubleVectorBuffer& Positions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num(), 1024);

	FVoxelDoubleVectorBuffer Result;
	Result.Allocate(6 * Positions.Num());

	const float HalfStep = Step / 2.f;

	const int32 NumPositions = Positions.Num();
	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		Result.X.Set(6 * Index + 0, Positions.X[Index] - HalfStep);
		Result.X.Set(6 * Index + 1, Positions.X[Index] + HalfStep);
		Result.X.Set(6 * Index + 2, Positions.X[Index]);
		Result.X.Set(6 * Index + 3, Positions.X[Index]);
		Result.X.Set(6 * Index + 4, Positions.X[Index]);
		Result.X.Set(6 * Index + 5, Positions.X[Index]);

		Result.Y.Set(6 * Index + 0, Positions.Y[Index]);
		Result.Y.Set(6 * Index + 1, Positions.Y[Index]);
		Result.Y.Set(6 * Index + 2, Positions.Y[Index] - HalfStep);
		Result.Y.Set(6 * Index + 3, Positions.Y[Index] + HalfStep); 
		Result.Y.Set(6 * Index + 4, Positions.Y[Index]);
		Result.Y.Set(6 * Index + 5, Positions.Y[Index]);


		Result.Z.Set(6 * Index + 0, Positions.Z[Index]);
		Result.Z.Set(6 * Index + 1, Positions.Z[Index]);
		Result.Z.Set(6 * Index + 2, Positions.Z[Index]);
		Result.Z.Set(6 * Index + 3, Positions.Z[Index]); 
		Result.Z.Set(6 * Index + 4, Positions.Z[Index] - HalfStep);
		Result.Z.Set(6 * Index + 5, Positions.Z[Index] + HalfStep);
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVector2DBuffer FVoxelBufferGradientUtilities::CollapseGradient2D(
	const FVoxelFloatBuffer& Heights,
	const int32 NumPositions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Heights.Num(), 1024);

	if (!ensure(Heights.Num() == NumPositions * 4))
	{
		return FVoxelVector2DBuffer::MakeDefault();
	}

	FVoxelVector2DBuffer Result;
	Result.Allocate(NumPositions);

	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		Result.X.Set(Index, (Heights[4 * Index + 1] - Heights[4 * Index + 0]) / Step);
		Result.Y.Set(Index, (Heights[4 * Index + 3] - Heights[4 * Index + 2]) / Step);
	}

	return Result;
}

FVoxelVectorBuffer FVoxelBufferGradientUtilities::CollapseGradient2DToNormal(
	const FVoxelFloatBuffer& Heights,
	const int32 NumPositions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Heights.Num(), 1024);

	if (!ensure(Heights.Num() == NumPositions * 4))
	{
		return FVoxelVectorBuffer::MakeDefault();
	}

	FVoxelVectorBuffer Result;
	Result.Allocate(NumPositions);

	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		const FVector3f Normal = FVector3f(
			-(Heights[4 * Index + 1] - Heights[4 * Index + 0]) / Step,
			-(Heights[4 * Index + 3] - Heights[4 * Index + 2]) / Step,
			1.f).GetSafeNormal();
		Result.Set(Index, Normal);
	}

	return Result;
}

FVoxelVectorBuffer FVoxelBufferGradientUtilities::CollapseGradient3D(
	const FVoxelFloatBuffer& Distances,
	const int32 NumPositions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Distances.Num(), 1024);

	if (!ensure(Distances.Num() == NumPositions * 6))
	{
		return FVoxelVectorBuffer::MakeDefault();
	}

	FVoxelVectorBuffer Result;
	Result.Allocate(NumPositions);

	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		Result.X.Set(Index, (Distances[6 * Index + 1] - Distances[6 * Index + 0]) / Step);
		Result.Y.Set(Index, (Distances[6 * Index + 3] - Distances[6 * Index + 2]) / Step);
		Result.Z.Set(Index, (Distances[6 * Index + 5] - Distances[6 * Index + 4]) / Step);
	}

	return Result;
}

FVoxelVectorBuffer FVoxelBufferGradientUtilities::CollapseGradient3DToNormal(
	const FVoxelFloatBuffer& Distances,
	const int32 NumPositions,
	const float Step)
{
	VOXEL_FUNCTION_COUNTER_NUM(Distances.Num(), 1024);

	if (!ensure(Distances.Num() == NumPositions * 6))
	{
		return FVoxelVectorBuffer::MakeDefault();
	}

	FVoxelVectorBuffer Result;
	Result.Allocate(NumPositions);

	for (int32 Index = 0; Index < NumPositions; Index++)
	{
		const FVector Normal = FVector(
			(Distances[6 * Index + 1] - Distances[6 * Index + 0]) / Step,
			(Distances[6 * Index + 3] - Distances[6 * Index + 2]) / Step,
			(Distances[6 * Index + 5] - Distances[6 * Index + 4]) / Step).GetSafeNormal();
		Result.X.Set(Index, Normal.X);
		Result.Y.Set(Index, Normal.Y);
		Result.Z.Set(Index, Normal.Z);
	}

	return Result;
}