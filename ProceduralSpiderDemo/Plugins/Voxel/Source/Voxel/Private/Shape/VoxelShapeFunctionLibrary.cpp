// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Shape/VoxelShapeFunctionLibrary.h"
#include "VoxelGraphMigration.h"
#include "VoxelShapeFunctionLibraryImpl.ispc.generated.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	REGISTER_VOXEL_FUNCTION_MIGRATION("CreateSphereDistanceField", UVoxelShapeFunctionLibrary, MakeSphere);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer UVoxelShapeFunctionLibrary::MakeSphere(
	const FVoxelDoubleVectorBuffer& Position,
	const FVoxelDoubleBuffer& Radius) const
{
	const int32 Num = ComputeVoxelBuffersNum_Return(Position, Radius);

	FVoxelFloatBuffer Result;
	Result.Allocate(Num);

	ispc::VoxelShapeFunctionLibrary_MakeSphere(
		Position.ISPC(),
		Radius.ISPC(),
		Result.GetData(),
		Num);

	return Result;
}

FVoxelBox UVoxelShapeFunctionLibrary::MakeSphereBounds(const double Radius) const
{
	return FVoxelBox(-Radius, Radius);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelFloatBuffer UVoxelShapeFunctionLibrary::MakeCube(
	const FVoxelDoubleVectorBuffer& Position,
	const FVoxelDoubleVectorBuffer& Size,
	const FVoxelDoubleBuffer& Roundness) const
{
	const int32 Num = ComputeVoxelBuffersNum_Return(Position, Size, Roundness);

	FVoxelFloatBuffer Result;
	Result.Allocate(Num);

	ispc::VoxelShapeFunctionLibrary_MakeCube(
		Position.ISPC(),
		Size.ISPC(),
		Roundness.ISPC(),
		Result.GetData(),
		Num);

	return Result;
}

FVoxelBox UVoxelShapeFunctionLibrary::MakeCubeBounds(const FVoxelDoubleVector& Size) const
{
	return FVoxelBox(-FVector(Size) / 2, FVector(Size) / 2);
}