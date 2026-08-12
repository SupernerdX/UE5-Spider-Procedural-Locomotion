// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"
#include "VoxelSampleMesh.isph"

export void VoxelStaticMeshFunctionLibrary_SampleVoxelStaticMesh(
	const uniform float PositionX[],
	const uniform float PositionY[],
	const uniform float PositionZ[],
	uniform float Result[],
	const uniform int32 Num,
	const uniform float Scale,
	const uniform bool bUseTricubic,
	const uniform int3& Size,
	const uniform float VoxelSize,
	const uniform float3& MeshOrigin,
	const uniform float MeshDistances[])
{
	const uniform float MaxPositionX = Size.x - 2.0001f;
	const uniform float MaxPositionY = Size.y - 2.0001f;
	const uniform float MaxPositionZ = Size.z - 2.0001f;

	const uniform float VoxelSizeAndScale = VoxelSize * Scale;
	const uniform float InvVoxelSizeAndScale = 1.f / VoxelSizeAndScale;

	const uniform int32 SizeX = Size.x;
	const uniform int32 SizeXY = Size.x * Size.y;
	const uniform int32 SizeXYZ = Size.x * Size.y * Size.z;

	FOREACH(Index, 0, Num)
	{
		const varying float3 Position = MakeFloat3(PositionX[Index], PositionY[Index], PositionZ[Index]) * InvVoxelSizeAndScale - MeshOrigin;

		const varying float Distance = SampleMeshDistances(
			Position.x,
			Position.y,
			Position.z,
			MaxPositionX,
			MaxPositionY,
			MaxPositionZ,
			SizeX,
			SizeXY,
			SizeXYZ,
			Size,
			bUseTricubic,
			MeshDistances);

		Result[Index] = Distance * VoxelSizeAndScale;
	}
}