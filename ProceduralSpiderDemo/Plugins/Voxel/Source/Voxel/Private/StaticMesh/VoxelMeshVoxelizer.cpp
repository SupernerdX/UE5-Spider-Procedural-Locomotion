// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "StaticMesh/VoxelMeshVoxelizer.h"

void FVoxelMeshVoxelizer::Voxelize(
	const FVoxelStaticMeshSettings& Settings,
	const TVoxelArray<FVector3f>& Vertices,
	const TVoxelArray<int32>& Indices,
	const TVoxelArray<FTriangleID>& TriangleIds,
	const FVector3f& Origin,
	const FIntVector& Size,
	TVoxelArray<float>& Distances,
	TVoxelArray<float>& ClosestX,
	TVoxelArray<float>& ClosestY,
	TVoxelArray<float>& ClosestZ,
	TVoxelMap<FVector3f, FVoxelStaticMeshPoint>& ClosestToPoint,
	int32& NumLeaks)
{
	VOXEL_FUNCTION_COUNTER();

	const int64 NumVoxels = int64(Size.X) * int64(Size.Y) * int64(Size.Z);
	if (!ensure(NumVoxels < MAX_int32))
	{
		return;
	}

	FVoxelUtilities::SetNumFast(Distances, NumVoxels);
	FVoxelUtilities::SetNumFast(ClosestX, NumVoxels);
	FVoxelUtilities::SetNumFast(ClosestY, NumVoxels);
	FVoxelUtilities::SetNumFast(ClosestZ, NumVoxels);

	ClosestToPoint.Reserve(NumVoxels / 10);

	FVoxelUtilities::SetAll(Distances, MAX_flt);
	FVoxelUtilities::SetAll(ClosestX, FVoxelUtilities::NaNf());
	FVoxelUtilities::SetAll(ClosestY, FVoxelUtilities::NaNf());
	FVoxelUtilities::SetAll(ClosestZ, FVoxelUtilities::NaNf());

	TVoxelArray<int32> IntersectionCount;
	FVoxelUtilities::SetNumZeroed(IntersectionCount, NumVoxels);

	int32 IndexI;
	int32 IndexJ;
	int32 IndexK;
	switch (Settings.SweepDirection)
	{
	default: ensure(false);
	case EVoxelAxis::X: IndexI = 1; IndexJ = 2; IndexK = 0; break;
	case EVoxelAxis::Y: IndexI = 2; IndexJ = 0; IndexK = 1; break;
	case EVoxelAxis::Z: IndexI = 0; IndexJ = 1; IndexK = 2; break;
	}

	{
		VOXEL_SCOPE_COUNTER("Intersections");
		checkVoxelSlow(Indices.Num() / 3 == TriangleIds.Num());

		for (int32 TriangleIndex = 0; TriangleIndex < TriangleIds.Num(); TriangleIndex++)
		{
			VOXEL_SCOPE_COUNTER("Process triangle");

			const int32 IndexA = Indices[3 * TriangleIndex + 0];
			const int32 IndexB = Indices[3 * TriangleIndex + 1];
			const int32 IndexC = Indices[3 * TriangleIndex + 2];

			const FVector3f& VertexA = Vertices[IndexA];
			const FVector3f& VertexB = Vertices[IndexB];
			const FVector3f& VertexC = Vertices[IndexC];

			if (!FVoxelUtilities::IsTriangleValid(
				VertexA,
				VertexB,
				VertexC))
			{
				continue;
			}

			const FVector3f VoxelVertexA = VertexA - Origin;
			const FVector3f VoxelVertexB = VertexB - Origin;
			const FVector3f VoxelVertexC = VertexC - Origin;

			const FVector3f MinVoxelVertex = FVoxelUtilities::ComponentMin3(VoxelVertexA, VoxelVertexB, VoxelVertexC);
			const FVector3f MaxVoxelVertex = FVoxelUtilities::ComponentMax3(VoxelVertexA, VoxelVertexB, VoxelVertexC);

			{
				VOXEL_SCOPE_COUNTER("Compute distance");

				const FIntVector Start = FVoxelUtilities::Clamp(FVoxelUtilities::FloorToInt(MinVoxelVertex), FIntVector(0), Size - 1);
				const FIntVector End = FVoxelUtilities::Clamp(FVoxelUtilities::CeilToInt(MaxVoxelVertex), FIntVector(0), Size - 1);

				for (int32 Z = Start.Z; Z <= End.Z; Z++)
				{
					for (int32 Y = Start.Y; Y <= End.Y; Y++)
					{
						for (int32 X = Start.X; X <= End.X; X++)
						{
							const FVector3f Position = Origin + FVector3f(X, Y, Z);

							FVector3f Barycentrics;
							const float Distance = FMath::Sqrt(FVoxelUtilities::PointTriangleDistanceSquared(
								Position,
								VertexA,
								VertexB,
								VertexC,
								Barycentrics));

							if (!FVoxelUtilities::IsFinite(Distance))
							{
								// Can happen if triangle is too small
								continue;
							}

							const int32 Index = FVoxelUtilities::Get3DIndex<int32>(Size, X, Y, Z);
							if (Distance >= Distances[Index])
							{
								continue;
							}

							const FVector3f Closest =
								Barycentrics.X * VoxelVertexA +
								Barycentrics.Y * VoxelVertexB +
								Barycentrics.Z * VoxelVertexC;

							Distances[Index] = Distance;
							ClosestX[Index] = Closest.X;
							ClosestY[Index] = Closest.Y;
							ClosestZ[Index] = Closest.Z;
							ClosestToPoint.FindOrAdd(Closest) = FVoxelStaticMeshPoint(TriangleIds[TriangleIndex], Barycentrics);
						}
					}
				}
			}

			{
				VOXEL_SCOPE_COUNTER("Compute intersections");

				const FIntVector Start = FVoxelUtilities::Clamp(FVoxelUtilities::CeilToInt(MinVoxelVertex), FIntVector(0), Size - 1);
				const FIntVector End = FVoxelUtilities::Clamp(FVoxelUtilities::FloorToInt(MaxVoxelVertex), FIntVector(0), Size - 1);

				FIntVector Position;
				for (int32 I = Start[IndexI]; I <= End[IndexI]; I++)
				{
					Position[IndexI] = I;

					for (int32 J = Start[IndexJ]; J <= End[IndexJ]; J++)
					{
						Position[IndexJ] = J;

						const auto Get2D = [&](const FVector3f& V)
						{
							return FVector2d(V[IndexI], V[IndexJ]);
						};

						double AlphaA;
						double AlphaB;
						double AlphaC;
						if (!PointInTriangle2D(
							FVector2d(I, J),
							Get2D(VoxelVertexA),
							Get2D(VoxelVertexB),
							Get2D(VoxelVertexC),
							AlphaA,
							AlphaB,
							AlphaC))
						{
							continue;
						}

						const double K =
							AlphaA * VoxelVertexA[IndexK] +
							AlphaB * VoxelVertexB[IndexK] +
							AlphaC * VoxelVertexC[IndexK];

						Position[IndexK] = FMath::Clamp(
							Settings.bReverseSweep
							? FMath::FloorToInt(K)
							: FMath::CeilToInt(K),
							0,
							Size[IndexK] - 1);

						IntersectionCount[FVoxelUtilities::Get3DIndex<int32>(Size, Position)]++;
					}
				}
			}
		}
	}

	NumLeaks = 0;
	{
		VOXEL_SCOPE_COUNTER("Compute Signs");

		// Figure out signs (inside/outside) from intersection counts
		FIntVector Position;
		for (int32 I = 0; I < Size[IndexI]; I++)
		{
			Position[IndexI] = I;

			for (int32 J = 0; J < Size[IndexJ]; J++)
			{
				Position[IndexJ] = J;

				// Compute the number of intersections, and skip it if it's a leak
				if (Settings.bHideLeaks)
				{
					int32 Count = 0;
					for (int32 K = 0; K < Size[IndexK]; K++)
					{
						Position[IndexK] = K;
						Count += IntersectionCount[FVoxelUtilities::Get3DIndex<int32>(Size, Position)];
					}

					if (Settings.bWatertight)
					{
						if (Count % 2 == 1)
						{
							// For watertight meshes, we're expecting to come in and out of the mesh
							NumLeaks++;
							continue;
						}
					}
					else
					{
						if (Count == 0)
						{
							// For other meshes, only skip when there was no hit
							NumLeaks++;
							continue;
						}
					}
				}
				// If we are not watertight, start inside (unless we're reverse)
				int32 Count = (!Settings.bWatertight && !Settings.bReverseSweep) ? 1 : 0;

				for (int32 K = 0; K < Size[IndexK]; K++)
				{
					Position[IndexK] = Settings.bReverseSweep ? Size[IndexK] - 1 - K : K;

					const int32 Index = FVoxelUtilities::Get3DIndex<int32>(Size, Position);

					Count += IntersectionCount[Index];

					if (Count % 2 == 1)
					{
						// If parity of intersections so far is odd, we are inside the mesh
						Distances[Index] *= -1;
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelMeshVoxelizer::Orientation(
	const FVector2d& A,
	const FVector2d& B,
	double& TwiceSignedArea)
{
	TwiceSignedArea = A.Y * B.X - A.X * B.Y;
	if (TwiceSignedArea > 0) return 1;
	else if (TwiceSignedArea < 0) return -1;
	else if (B.Y > A.Y) return 1;
	else if (B.Y < A.Y) return -1;
	else if (A.X > B.X) return 1;
	else if (A.X < B.X) return -1;
	else return 0; // Only true when A.X == B.X and A.Y == B.Y
}

bool FVoxelMeshVoxelizer::PointInTriangle2D(
	const FVector2d& Point,
	FVector2d A,
	FVector2d B,
	FVector2d C,
	double& AlphaA, double& AlphaB, double& AlphaC)
{
	A -= Point;
	B -= Point;
	C -= Point;

	const int32 SignA = Orientation(B, C, AlphaA);
	if (SignA == 0) return false;

	const int32 SignB = Orientation(C, A, AlphaB);
	if (SignB != SignA) return false;

	const int32 SignC = Orientation(A, B, AlphaC);
	if (SignC != SignA) return false;

	const double Sum = AlphaA + AlphaB + AlphaC;
	checkSlow(Sum != 0); // if the SOS signs match and are non-zero, there's no way all of a, b, and c are zero.
	AlphaA /= Sum;
	AlphaB /= Sum;
	AlphaC /= Sum;

	return true;
}