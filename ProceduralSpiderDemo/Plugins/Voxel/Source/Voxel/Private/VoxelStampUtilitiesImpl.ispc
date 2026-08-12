// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"
#include "VoxelStampQueryImpl.isph"
#include "VoxelStampTransformImpl.isph"

export void VoxelStampUtilities_ComputePositions_BulkHeight(
	const uniform FVoxelHeightBulkQuery& Query,
	const uniform FVoxelHeightTransform& StampToQuery,
	uniform double OutPositionsX[],
	uniform double OutPositionsY[])
{
	const uniform int2 Size = Query.EndIndex - Query.StartIndex;

	uniform int32 WriteIndex = 0;
	for (uniform int32 IndexY = Query.StartIndex.y; IndexY < Query.EndIndex.y; IndexY++)
	{
		FOREACH(Index, 0, Size.x)
		{
			const varying int32 IndexX = Query.StartIndex.x + Index;
			const varying double2 Position = InverseTransform_double(StampToQuery, GetPosition(Query, IndexX, IndexY));

			OutPositionsX[WriteIndex + Index] = Position.x;
			OutPositionsY[WriteIndex + Index] = Position.y;
		}

		WriteIndex += Size.x;
	}
	check(WriteIndex == Size.x * Size.y);
}

export void VoxelStampUtilities_ComputePositions_SparseHeight(
	const uniform FVoxelHeightSparseQuery& Query,
	const uniform FVoxelHeightTransform& StampToQuery,
	uniform double OutPositionsX[],
	uniform double OutPositionsY[])
{
	FOREACH(Index, 0, Query.Num)
	{
		const varying double2 WorldPosition = MakeDouble2(
			Query.PositionsX[Index],
			Query.PositionsY[Index]);

		const varying double2 Position = InverseTransform_double(StampToQuery, WorldPosition);

		OutPositionsX[Index] = Position.x;
		OutPositionsY[Index] = Position.y;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

export void VoxelStampUtilities_ComputePositions_BulkVolume(
	const uniform FVoxelVolumeBulkQuery& Query,
	const uniform FVoxelVolumeTransform& StampToQuery,
	uniform double OutPositionsX[],
	uniform double OutPositionsY[],
	uniform double OutPositionsZ[])
{
	const uniform int3 Size = Query.EndIndex - Query.StartIndex;

	uniform int32 WriteIndex = 0;
	for (uniform int32 IndexZ = Query.StartIndex.z; IndexZ < Query.EndIndex.z; IndexZ++)
	{
		for (uniform int32 IndexY = Query.StartIndex.y; IndexY < Query.EndIndex.y; IndexY++)
		{
			FOREACH(Index, 0, Size.x)
			{
				const varying int32 IndexX = Query.StartIndex.x + Index;
				const varying double3 Position = InverseTransform_double(StampToQuery, GetPosition(Query, IndexX, IndexY, IndexZ));

				OutPositionsX[WriteIndex + Index] = Position.x;
				OutPositionsY[WriteIndex + Index] = Position.y;
				OutPositionsZ[WriteIndex + Index] = Position.z;
			}

			WriteIndex += Size.x;
		}
	}
	check(WriteIndex == Size.x * Size.y * Size.z);
}

export void VoxelStampUtilities_ComputePositions_SparseVolume(
	const uniform FVoxelVolumeSparseQuery& Query,
	const uniform FVoxelVolumeTransform& StampToQuery,
	uniform double OutPositionsX[],
	uniform double OutPositionsY[],
	uniform double OutPositionsZ[])
{
	FOREACH(Index, 0, Query.Num)
	{
		const varying double3 WorldPosition = MakeDouble3(
			Query.PositionsX[Index],
			Query.PositionsY[Index],
			Query.PositionsZ[Index]);

		const varying double3 Position = InverseTransform_double(StampToQuery, WorldPosition);

		OutPositionsX[Index] = Position.x;
		OutPositionsY[Index] = Position.y;
		OutPositionsZ[Index] = Position.z;
	}
}