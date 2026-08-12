// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Shape/VoxelPlaneShape.h"
#include "Engine/StaticMesh.h"

FVoxelBox FVoxelPlaneShape::GetLocalBounds() const
{
	const FVector Size3D = FVector(Size, Size.GetAbsMax() * Height);

	return FVoxelBox(-Size3D / 2, Size3D / 2);
}

void FVoxelPlaneShape::Sample(
	const TVoxelArrayView<float> OutDistances,
	const FVoxelDoubleVectorBuffer& Positions) const
{
	VOXEL_FUNCTION_COUNTER_NUM(OutDistances.Num());
	checkVoxelSlow(OutDistances.Num() == Positions.Num());

	const double HalfSizeX = Size.X / 2.f;
	const double HalfSizeY = Size.Y / 2.f;
	const double HalfHeight = Size.GetAbsMax() * Height / 2.f;

	for (int32 Index = 0; Index < OutDistances.Num(); Index++)
	{
		const double X = Positions.X[Index];
		const double Y = Positions.Y[Index];
		const double Z = Positions.Z[Index];

		if (FMath::Abs(X) > HalfSizeX ||
			FMath::Abs(Y) > HalfSizeY ||
			FMath::Abs(Z) > HalfHeight)
		{
			OutDistances[Index] = FVoxelUtilities::NaNf();
			continue;
		}

		OutDistances[Index] = Z;
	}
}

#if WITH_EDITOR
void FVoxelPlaneShape::GetPreviewInfo(
	UStaticMesh*& OutMesh,
	FTransform& OutTransform) const
{
	UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	ensure(StaticMesh);
	OutMesh = StaticMesh;

	OutTransform = FTransform(
		FQuat::Identity,
		FVector::ZeroVector,
		FVector(Size / 100.f, 1.f));
}
#endif