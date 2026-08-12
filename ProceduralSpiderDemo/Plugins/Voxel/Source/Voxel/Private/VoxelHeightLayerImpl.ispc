// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"
#include "VoxelStampQueryImpl.isph"
#include "VoxelStampTransformImpl.isph"

// ReSharper disable CppCStyleCast

FORCEINLINE varying float ClampDistance(
	const varying float Distance,
	const uniform float MaxDistance)
{
	return select(
		abs(Distance) < MaxDistance,
		Distance,
		NaNf);
}

export void VoxelHeightLayer_HeightToVolume_Dense(
	const uniform FVoxelVolumeBulkQuery& Query,
	const uniform float Heights[],
	const uniform float MaxDistance)
{
	const uniform int32 SizeX = Query.EndIndex.x - Query.StartIndex.x;
	const uniform int32 SizeY = Query.EndIndex.y - Query.StartIndex.y;

	for (uniform int32 Y = 0; Y < SizeY; Y++)
	{
		FOREACH(X, 0, SizeX)
		{
			const varying int32 Index = (Query.StartIndex.x + X) + Query.StrideX * (Query.StartIndex.y + Y);
			const varying float Height = Heights[X + SizeX * Y];

			coherent_if (intbits(Height) == NaNf_uint)
			{
				continue;
			}

			const varying float BaseHeight = (float)(Height - Query.Start.z);

			for (uniform int32 IndexZ = Query.StartIndex.z; IndexZ < Query.EndIndex.z; IndexZ++)
			{
				Query.Distances[Index + Query.StrideXY * IndexZ] = ClampDistance(IndexZ * Query.Step - BaseHeight, MaxDistance);
			}
		}
	}
}

export void VoxelHeightLayer_HeightToVolume_Sparse(
	const uniform FVoxelVolumeSparseQuery& Query,
	const uniform float IndirectHeights[],
	const uniform float MaxDistance)
{
	FOREACH(Index, 0, Query.Num)
	{
		const varying double3 Position = MakeDouble3(
			Query.PositionsX[Index],
			Query.PositionsY[Index],
			Query.PositionsZ[Index]);

		if (Query.Indirection)
		{
			const varying int32 IndirectIndex = Query.Indirection[Index];

			IGNORE_PERF_WARNING
			const varying float Height = IndirectHeights[IndirectIndex];

			coherent_if (intbits(Height) == NaNf_uint)
			{
				continue;
			}

			IGNORE_PERF_WARNING
			Query.IndirectDistances[IndirectIndex] = ClampDistance((float)(Position.z - Height), MaxDistance);
		}
		else
		{
			const varying float Height = IndirectHeights[Index];

			coherent_if (intbits(Height) == NaNf_uint)
			{
				continue;
			}

			Query.IndirectDistances[Index] = ClampDistance((float)(Position.z - Height), MaxDistance);
		}
	}
}