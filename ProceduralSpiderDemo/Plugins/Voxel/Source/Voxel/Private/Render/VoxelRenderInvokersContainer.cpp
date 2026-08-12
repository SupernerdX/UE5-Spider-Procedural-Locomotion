// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Render/VoxelRenderInvokersContainer.h"

FVoxelLODVolume FVoxelLODVolume::MakeSphere(
	const FVector& Position,
	const double Radius,
	const int32 MinLOD)
{
	FVoxelLODVolume LODVolume;
	LODVolume.Position = Position;
	LODVolume.Radius = Radius;

	LODVolume.Type = EVoxelLODInvokerType::Sphere;
	LODVolume.MinLOD = MinLOD;
	return MoveTemp(LODVolume);
}

FVoxelLODVolume FVoxelLODVolume::MakeBox(
	const FVector& Position,
	const FQuat& Rotation,
	const FVector& Extents,
	const int32 MinLOD)
{
	FVoxelLODVolume LODVolume;
	LODVolume.Position = Position;
	LODVolume.InvRotation = Rotation.Inverse();
	LODVolume.Extents = Extents;

	const FMatrix RotationMatrix = LODVolume.InvRotation.ToMatrix();
	for (int32 Row = 0; Row < 3; Row++)
	{
		for (int32 Col = 0; Col < 3; Col++)
		{
			LODVolume.AbsRotationMatrix.M[Row][Col] = FMath::Abs(RotationMatrix.M[Row][Col]);
		}
	}

	LODVolume.Type = EVoxelLODInvokerType::Box;
	LODVolume.MinLOD = MinLOD;
	return MoveTemp(LODVolume);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelBox FVoxelLODVolume::GetBounds() const
{
	switch (Type)
	{
	case EVoxelLODInvokerType::Sphere: return FVoxelBox(Position).Extend(Radius);
	case EVoxelLODInvokerType::Box: return FVoxelBox(FVector::ZeroVector).Extend(Extents).TransformBy(InvRotation.Inverse().ToMatrix()).Translate(Position);
	}

	ensure(false);
	return FVoxelBox();
}

bool FVoxelLODVolume::operator==(const FVoxelLODVolume& Other) const
{
	return
		Position == Other.Position &&
		Radius == Other.Radius &&
		Type == Other.Type &&
		InvRotation == Other.InvRotation &&
		Extents == Other.Extents &&
		AbsRotationMatrix == Other.AbsRotationMatrix &&
		MinLOD == Other.MinLOD;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelRenderInvokersContainer::operator==(const FVoxelRenderInvokersContainer& Other) const
{
	return
		CameraInvokers == Other.CameraInvokers &&
		LODVolumes == Other.LODVolumes;
}