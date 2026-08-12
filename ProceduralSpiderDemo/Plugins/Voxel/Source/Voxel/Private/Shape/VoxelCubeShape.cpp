// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Shape/VoxelCubeShape.h"
#include "Engine/StaticMesh.h"
#include "VoxelShapeFunctionLibraryImpl.ispc.generated.h"

FVoxelBox FVoxelCubeShape::GetLocalBounds() const
{
	return FVoxelBox(-Size / 2, Size / 2);
}

void FVoxelCubeShape::Sample(
	const TVoxelArrayView<float> OutDistances,
	const FVoxelDoubleVectorBuffer& Positions) const
{
	VOXEL_FUNCTION_COUNTER_NUM(OutDistances.Num());
	checkVoxelSlow(OutDistances.Num() == Positions.Num());

	ispc::VoxelShapeFunctionLibrary_MakeCube(
		Positions.ISPC(),
		FVoxelDoubleVectorBuffer(Size).ISPC(),
		FVoxelDoubleBuffer(Roundness).ISPC(),
		OutDistances.GetData(),
		OutDistances.Num());
}

#if WITH_EDITOR
void FVoxelCubeShape::GetPreviewInfo(
	UStaticMesh*& OutMesh,
	FTransform& OutTransform) const
{
	UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	ensure(StaticMesh);
	OutMesh = StaticMesh;

	OutTransform = FTransform(
		FQuat::Identity,
		FVector::ZeroVector,
		Size / 100.f);
}
#endif