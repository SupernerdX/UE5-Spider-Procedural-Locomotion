// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"
#include "VoxelStampTransformImpl.isph"

export void VoxelSamplePreviousStampsNodes_TransformHeightPositions(
	const uniform FVoxelHeightTransform& StampToQuery,
	const uniform double PositionsX[],
	const uniform double PositionsY[],
	uniform double OutPositionsX[],
	uniform double OutPositionsY[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying double2 Position = Transform_double(StampToQuery, MakeDouble2(
			PositionsX[Index],
			PositionsY[Index]));

		OutPositionsX[Index] = Position.x;
		OutPositionsY[Index] = Position.y;
	}
}

export void VoxelSamplePreviousStampsNodes_TransformHeights(
	const uniform FVoxelHeightTransform& StampToQuery,
	const uniform double PositionsX[],
	const uniform double PositionsY[],
	uniform float Heights[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Heights[Index] = InverseTransformHeight(
			StampToQuery,
			Heights[Index],
			MakeFloat2(
				PositionsX[Index],
				PositionsY[Index]));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelSamplePreviousStampsNodes_TransformVolumePositions(
	const uniform FVoxelVolumeTransform& StampToQuery,
	const uniform double PositionsX[],
	const uniform double PositionsY[],
	const uniform double PositionsZ[],
	uniform double OutPositionsX[],
	uniform double OutPositionsY[],
	uniform double OutPositionsZ[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		const varying double3 Position = Transform_double(StampToQuery, MakeDouble3(
			PositionsX[Index],
			PositionsY[Index],
			PositionsZ[Index]));

		OutPositionsX[Index] = Position.x;
		OutPositionsY[Index] = Position.y;
		OutPositionsZ[Index] = Position.z;
	}
}

export void VoxelSamplePreviousStampsNodes_TransformDistances(
	const uniform FVoxelVolumeTransform& StampToQuery,
	uniform float Distances[],
	const uniform int32 Num)
{
	FOREACH(Index, 0, Num)
	{
		Distances[Index] /= StampToQuery.DistanceScale;
	}
}