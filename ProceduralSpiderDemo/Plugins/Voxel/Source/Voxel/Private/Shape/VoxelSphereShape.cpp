// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Shape/VoxelSphereShape.h"
#include "Engine/StaticMesh.h"
#include "VoxelShapeFunctionLibraryImpl.ispc.generated.h"

FVoxelBox FVoxelSphereShape::GetLocalBounds() const
{
	return FVoxelBox(-Radius, Radius);
}

void FVoxelSphereShape::Sample(
	const TVoxelArrayView<float> OutDistances,
	const FVoxelDoubleVectorBuffer& Positions) const
{
	VOXEL_FUNCTION_COUNTER_NUM(OutDistances.Num());
	checkVoxelSlow(OutDistances.Num() == Positions.Num());

	ispc::VoxelShapeFunctionLibrary_MakeSphere(
		Positions.ISPC(),
		FVoxelDoubleBuffer(Radius).ISPC(),
		OutDistances.GetData(),
		OutDistances.Num());
}

#if WITH_EDITOR
void FVoxelSphereShape::GetPreviewInfo(
	UStaticMesh*& OutMesh,
	FTransform& OutTransform) const
{
	UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	ensure(StaticMesh);
	OutMesh = StaticMesh;

	OutTransform = FTransform(
		FQuat::Identity,
		FVector::ZeroVector,
		FVector(Radius / 50.f));
}
#endif