// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"
#include "VoxelStampQueryImpl.isph"
#include "VoxelStampTransformImpl.isph"

// ReSharper disable CppCStyleCast

extern "C" const void* uniform VoxelVolumeChunkTreeIterator_FindChunk(
	const void* uniform KeyToChunk,
	uniform int32 ChunkKeyX,
	uniform int32 ChunkKeyY,
	uniform int32 ChunkKeyZ);

struct FCache
{
	uniform int32 ChunkKeysX[TARGET_WIDTH];
	uniform int32 ChunkKeysY[TARGET_WIDTH];
	uniform int32 ChunkKeysZ[TARGET_WIDTH];
	const void* uniform Chunks[TARGET_WIDTH];
	uniform int32 InsertIndex;
	const void* uniform KeyToChunk;
};

FORCEINLINE const void* uniform FindChunk(
	uniform FCache& Cache,
	const uniform int32 ChunkKeyX,
	const uniform int32 ChunkKeyY,
	const uniform int32 ChunkKeyZ)
{
	const varying bool bEqual =
		Cache.ChunkKeysX[programIndex] == ChunkKeyX &&
		Cache.ChunkKeysY[programIndex] == ChunkKeyY &&
		Cache.ChunkKeysZ[programIndex] == ChunkKeyZ;

	const uniform int32 Mask = packmask(bEqual);

	if (Mask != 0)
	{
		const uniform int32 CacheIndex = count_trailing_zeros(Mask);

		check(
			Cache.ChunkKeysX[CacheIndex] == ChunkKeyX &&
			Cache.ChunkKeysY[CacheIndex] == ChunkKeyY &&
			Cache.ChunkKeysZ[CacheIndex] == ChunkKeyZ);

		return Cache.Chunks[CacheIndex];
	}

	const void* uniform Chunk = VoxelVolumeChunkTreeIterator_FindChunk(
		Cache.KeyToChunk,
		ChunkKeyX,
		ChunkKeyY,
		ChunkKeyZ);

	Cache.ChunkKeysX[Cache.InsertIndex] = ChunkKeyX;
	Cache.ChunkKeysY[Cache.InsertIndex] = ChunkKeyY;
	Cache.ChunkKeysZ[Cache.InsertIndex] = ChunkKeyZ;
	Cache.Chunks[Cache.InsertIndex] = Chunk;
	Cache.InsertIndex = (Cache.InsertIndex + 1) % TARGET_WIDTH;

	return Chunk;
}

export void VoxelVolumeChunkTreeIterator_CreateIterator_Bulk(
	const uniform FVoxelVolumeBulkQuery& Query,
	const uniform FVoxelVolumeTransform& SculptToQuery,
	const uniform double Offset,
	const void* uniform KeyToChunk,
	uniform uint8 OutHasMaxChunk[],
	uniform int32 OutIndirectIndex[],
	uniform float OutAlphaX[],
	uniform float OutAlphaY[],
	uniform float OutAlphaZ[],
	uniform int32 OutPositionX[],
	uniform int32 OutPositionY[],
	uniform int32 OutPositionZ[],
	const void* uniform OutChunks[],
	int32* uniform OutNumChunks)
{
	const uniform int3 Size = Query.EndIndex - Query.StartIndex;

	uniform FCache Cache;
	Cache.ChunkKeysX[programIndex] = MAX_int32;
	Cache.ChunkKeysY[programIndex] = MAX_int32;
	Cache.ChunkKeysZ[programIndex] = MAX_int32;
	Cache.Chunks[programIndex] = nullptr;
	Cache.InsertIndex = 0;
	Cache.KeyToChunk = KeyToChunk;

	uniform int32 ChunkIndex = 0;

	uniform int32 GlobalWriteIndex = 0;
	for (uniform int32 IndexZ = Query.StartIndex.z; IndexZ < Query.EndIndex.z; IndexZ++)
	{
		for (uniform int32 IndexY = Query.StartIndex.y; IndexY < Query.EndIndex.y; IndexY++)
		{
			FOREACH(Index, 0, Size.x)
			{
				const varying int32 IndexX = Query.StartIndex.x + Index;
				const varying int32 IndirectIndex = GetIndex(Query, IndexX, IndexY, IndexZ);
				const varying double3 Position = InverseTransform_double(SculptToQuery, GetPosition(Query, IndexX, IndexY, IndexZ)) + Offset;
				const varying int32 WriteIndex = GlobalWriteIndex + Index;

				const varying int3 PositionMin = MakeInt3(floor(Position));
				const varying int3 PositionMax = MakeInt3(ceil(Position)); // Not +1, Position is likely to be an integer

				const varying int3 ChunkKeyMin = PositionMin >> 3;
				const varying int3 ChunkKeyMax = PositionMax >> 3;

				const varying uint32 Flag =
					((uint32)(ChunkKeyMin.x != ChunkKeyMax.x) << 0) |
					((uint32)(ChunkKeyMin.y != ChunkKeyMax.y) << 1) |
					((uint32)(ChunkKeyMin.z != ChunkKeyMax.z) << 2) |
					((uint32)(PositionMin.x != PositionMax.x) << 3) |
					((uint32)(PositionMin.y != PositionMax.y) << 4) |
					((uint32)(PositionMin.z != PositionMax.z) << 5);

				OutHasMaxChunk[WriteIndex] = (uint8)Flag;
				OutIndirectIndex[WriteIndex] = IndirectIndex;
				OutAlphaX[WriteIndex] = (float)(Position.x - PositionMin.x);
				OutAlphaY[WriteIndex] = (float)(Position.y - PositionMin.y);
				OutAlphaZ[WriteIndex] = (float)(Position.z - PositionMin.z);
				OutPositionX[WriteIndex] = PositionMin.x;
				OutPositionY[WriteIndex] = PositionMin.y;
				OutPositionZ[WriteIndex] = PositionMin.z;

				// Read the current active lanes
				const uniform int64 LaneMask = lanemask_unsafe();

				// Reset active lanes
				unmasked_unsafe
				{
					UNROLL
					for (uniform int32 Lane = 0; Lane < programCount; Lane++)
					{
						if (!(LaneMask & (1u << Lane)))
						{
							continue;
						}

						const uniform uint32 Flag_Lane = extract(Flag, Lane);
						const uniform int32 ChunkKeyMinX_Lane = extract(ChunkKeyMin.x, Lane);
						const uniform int32 ChunkKeyMinY_Lane = extract(ChunkKeyMin.y, Lane);
						const uniform int32 ChunkKeyMinZ_Lane = extract(ChunkKeyMin.z, Lane);

						UNROLL
						for (uniform int32 It = 0; It < 8; It++)
						{
							if ((It & Flag_Lane) != It)
							{
								continue;
							}

							OutChunks[ChunkIndex++] = FindChunk(
								Cache,
								ChunkKeyMinX_Lane + ((It & 0b001) >> 0),
								ChunkKeyMinY_Lane + ((It & 0b010) >> 1),
								ChunkKeyMinZ_Lane + ((It & 0b100) >> 2));
						}
					}
				}
			}

			GlobalWriteIndex += Size.x;
		}
	}
	check(GlobalWriteIndex == Size.x * Size.y * Size.z);

	*OutNumChunks = ChunkIndex;
}