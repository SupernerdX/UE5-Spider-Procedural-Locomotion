// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.isph"
#include "VoxelStampQueryImpl.isph"
#include "VoxelStampTransformImpl.isph"

// ReSharper disable CppCStyleCast

extern "C" const void* uniform VoxelHeightChunkTreeIterator_FindChunk(
	const void* uniform KeyToChunk,
	uniform int32 ChunkKeyX,
	uniform int32 ChunkKeyY);

struct FCache
{
	uniform int32 ChunkKeysX[TARGET_WIDTH];
	uniform int32 ChunkKeysY[TARGET_WIDTH];
	const void* uniform Chunks[TARGET_WIDTH];
	uniform int32 InsertIndex;
	const void* uniform KeyToChunk;
};

FORCEINLINE const void* uniform FindChunk(
	uniform FCache& Cache,
	const uniform int32 ChunkKeyX,
	const uniform int32 ChunkKeyY)
{
	const varying bool bEqual =
		Cache.ChunkKeysX[programIndex] == ChunkKeyX &&
		Cache.ChunkKeysY[programIndex] == ChunkKeyY;

	const uniform int32 Mask = packmask(bEqual);

	if (Mask != 0)
	{
		const uniform int32 CacheIndex = count_trailing_zeros(Mask);

		check(
			Cache.ChunkKeysX[CacheIndex] == ChunkKeyX &&
			Cache.ChunkKeysY[CacheIndex] == ChunkKeyY);

		return Cache.Chunks[CacheIndex];
	}

	const void* uniform Chunk = VoxelHeightChunkTreeIterator_FindChunk(
		Cache.KeyToChunk,
		ChunkKeyX,
		ChunkKeyY);

	Cache.ChunkKeysX[Cache.InsertIndex] = ChunkKeyX;
	Cache.ChunkKeysY[Cache.InsertIndex] = ChunkKeyY;
	Cache.Chunks[Cache.InsertIndex] = Chunk;
	Cache.InsertIndex = (Cache.InsertIndex + 1) % TARGET_WIDTH;

	return Chunk;
}

export void VoxelHeightChunkTreeIterator_CreateIterator_Bulk(
	const uniform FVoxelHeightBulkQuery& Query,
	const uniform FVoxelHeightTransform& SculptToQuery,
	const uniform double Offset,
	const void* uniform KeyToChunk,
	uniform uint8 OutHasMaxChunk[],
	uniform int32 OutIndirectIndex[],
	uniform float OutAlphaX[],
	uniform float OutAlphaY[],
	uniform int32 OutPositionX[],
	uniform int32 OutPositionY[],
	const void* uniform OutChunks[],
	int32* uniform OutNumChunks)
{
	const uniform int2 Size = Query.EndIndex - Query.StartIndex;

	uniform FCache Cache;
	Cache.ChunkKeysX[programIndex] = MAX_int32;
	Cache.ChunkKeysY[programIndex] = MAX_int32;
	Cache.Chunks[programIndex] = nullptr;
	Cache.InsertIndex = 0;
	Cache.KeyToChunk = KeyToChunk;

	uniform int32 ChunkIndex = 0;

	uniform int32 GlobalWriteIndex = 0;
	for (uniform int32 IndexY = Query.StartIndex.y; IndexY < Query.EndIndex.y; IndexY++)
	{
		FOREACH(Index, 0, Size.x)
		{
			const varying int32 IndexX = Query.StartIndex.x + Index;
			const varying int32 IndirectIndex = GetIndex(Query, IndexX, IndexY);
			const varying double2 Position = InverseTransform_double(SculptToQuery, GetPosition(Query, IndexX, IndexY)) + Offset;
			const varying int32 WriteIndex = GlobalWriteIndex + Index;

			const varying int2 PositionMin = MakeInt2(floor(Position));
			const varying int2 PositionMax = MakeInt2(ceil(Position)); // Not +1, Position is likely to be an integer

			const varying int2 ChunkKeyMin = PositionMin >> 4;
			const varying int2 ChunkKeyMax = PositionMax >> 4;

			const varying uint32 Flag =
				((uint32)(ChunkKeyMin.x != ChunkKeyMax.x) << 0) |
				((uint32)(ChunkKeyMin.y != ChunkKeyMax.y) << 1) |
				((uint32)(PositionMin.x != PositionMax.x) << 2) |
				((uint32)(PositionMin.y != PositionMax.y) << 3);

			OutHasMaxChunk[WriteIndex] = (uint8)Flag;
			OutIndirectIndex[WriteIndex] = IndirectIndex;
			OutAlphaX[WriteIndex] = (float)(Position.x - PositionMin.x);
			OutAlphaY[WriteIndex] = (float)(Position.y - PositionMin.y);
			OutPositionX[WriteIndex] = PositionMin.x;
			OutPositionY[WriteIndex] = PositionMin.y;

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

					UNROLL
					for (uniform int32 It = 0; It < 4; It++)
					{
						if ((It & Flag_Lane) != It)
						{
							continue;
						}

						OutChunks[ChunkIndex++] = FindChunk(
							Cache,
							ChunkKeyMinX_Lane + ((It & 0b001) >> 0),
							ChunkKeyMinY_Lane + ((It & 0b010) >> 1));
					}
				}
			}
		}

		GlobalWriteIndex += Size.x;
	}
	check(GlobalWriteIndex == Size.x * Size.y);

	*OutNumChunks = ChunkIndex;
}