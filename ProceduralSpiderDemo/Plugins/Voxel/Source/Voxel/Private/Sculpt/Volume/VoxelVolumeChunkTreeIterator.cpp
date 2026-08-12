// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeChunkTreeIterator.h"
#include "VoxelVolumeChunkTreeIteratorImpl.ispc.generated.h"

extern "C" void* VoxelVolumeChunkTreeIterator_FindChunk(
	const void* KeyToChunk,
	const int32 ChunkKeyX,
	const int32 ChunkKeyY,
	const int32 ChunkKeyZ)
{
	return static_cast<const TVoxelMap<FIntVector, void*>*>(KeyToChunk)->FindRef(FIntVector(
		ChunkKeyX,
		ChunkKeyY,
		ChunkKeyZ));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeChunkTreeIterator::Reserve(const int32 Num)
{
	VOXEL_FUNCTION_COUNTER();

	Chunks.Reserve(Num * 8);

	FVoxelUtilities::SetNumFast(Flags_Array, Num);
	FVoxelUtilities::SetNumFast(IndirectIndex_Array, Num);
	FVoxelUtilities::SetNumFast(AlphaX_Array, Num);
	FVoxelUtilities::SetNumFast(AlphaY_Array, Num);
	FVoxelUtilities::SetNumFast(AlphaZ_Array, Num);
	FVoxelUtilities::SetNumFast(PositionX_Array, Num);
	FVoxelUtilities::SetNumFast(PositionY_Array, Num);
	FVoxelUtilities::SetNumFast(PositionZ_Array, Num);
}

template<typename FindChunkType>
FORCEINLINE void FVoxelVolumeChunkTreeIterator::Set(
	const int32 Index,
	const int32 IndirectIndex,
	const FVector3d& Position,
	const FindChunkType& FindChunk)
{
	const FIntVector PositionMin = FVoxelUtilities::FloorToInt(Position);
	const FIntVector PositionMax = FVoxelUtilities::CeilToInt(Position); // Not +1, Position is likely to be an integer
	checkVoxelSlow(PositionMax.X == PositionMin.X || PositionMax.X == PositionMin.X + 1);
	checkVoxelSlow(PositionMax.Y == PositionMin.Y || PositionMax.Y == PositionMin.Y + 1);
	checkVoxelSlow(PositionMax.Z == PositionMin.Z || PositionMax.Z == PositionMin.Z + 1);

	const FIntVector ChunkKeyMin = FVoxelUtilities::DivideFloor_FastLog2(PositionMin, ChunkSizeLog2);
	const FIntVector ChunkKeyMax = FVoxelUtilities::DivideFloor_FastLog2(PositionMax, ChunkSizeLog2);

	const uint32 Flag =
		(uint32(ChunkKeyMin.X != ChunkKeyMax.X) << 0) |
		(uint32(ChunkKeyMin.Y != ChunkKeyMax.Y) << 1) |
		(uint32(ChunkKeyMin.Z != ChunkKeyMax.Z) << 2) |
		(uint32(PositionMin.X != PositionMax.X) << 3) |
		(uint32(PositionMin.Y != PositionMax.Y) << 4) |
		(uint32(PositionMin.Z != PositionMax.Z) << 5);

	Flags_Array[Index] = uint8(Flag);
	IndirectIndex_Array[Index] = IndirectIndex;
	AlphaX_Array[Index] = float(Position.X - PositionMin.X);
	AlphaY_Array[Index] = float(Position.Y - PositionMin.Y);
	AlphaZ_Array[Index] = float(Position.Z - PositionMin.Z);
	PositionX_Array[Index] = PositionMin.X;
	PositionY_Array[Index] = PositionMin.Y;
	PositionZ_Array[Index] = PositionMin.Z;

	for (int32 ChunkIndex = 0; ChunkIndex < 8; ChunkIndex++)
	{
		if ((ChunkIndex & Flag) != ChunkIndex)
		{
			continue;
		}

		Chunks.Add_EnsureNoGrow(FindChunk(FIntVector(
			ChunkIndex & 0b001 ? ChunkKeyMax.X : ChunkKeyMin.X,
			ChunkIndex & 0b010 ? ChunkKeyMax.Y : ChunkKeyMin.Y,
			ChunkIndex & 0b100 ? ChunkKeyMax.Z : ChunkKeyMin.Z)));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVolumeChunkTreeIterator FVoxelVolumeChunkTreeIterator::Create(
	const TVoxelMap<FIntVector, void*>& KeyToChunk,
	const FVoxelVolumeBulkQuery& Query,
	const FVoxelVolumeTransform& SculptToQuery,
	const double Offset)
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

#if VOXEL_DEBUG
	FVoxelVolumeChunkTreeIterator Iterator2;
	{
		Iterator2.Reserve(Query.Num());

		FIntVector LastChunkKey = FIntVector(MAX_int32);
		const void* LastChunk = nullptr;

		int32 WriteIndex = 0;
		for (int32 IndexZ = Query.Indices.Min.Z; IndexZ < Query.Indices.Max.Z; IndexZ++)
		{
			for (int32 IndexY = Query.Indices.Min.Y; IndexY < Query.Indices.Max.Y; IndexY++)
			{
				for (int32 IndexX = Query.Indices.Min.X; IndexX < Query.Indices.Max.X; IndexX++)
				{
					const FVector3d Position = SculptToQuery.InverseTransform(Query.GetPosition(IndexX, IndexY, IndexZ));
					const int32 IndirectIndex = Query.GetIndex(IndexX, IndexY, IndexZ);

					Iterator2.Set(WriteIndex, IndirectIndex, Position + Offset, [&](const FIntVector& ChunkKey)
					{
						if (ChunkKey != LastChunkKey)
						{
							LastChunk = KeyToChunk.FindRef(ChunkKey);
							LastChunkKey = ChunkKey;
						}
						return LastChunk;
					});

					WriteIndex++;
				}
			}
		}
		check(WriteIndex == Query.Num());
	}
#endif

	FVoxelVolumeChunkTreeIterator Iterator;
	Iterator.Reserve(Query.Num());

	Iterator.Chunks.SetNumUninitialized(Query.Num() * 8);

	int32 NumChunks = 0;
	ispc::VoxelVolumeChunkTreeIterator_CreateIterator_Bulk(
		Query.ISPC(),
		SculptToQuery.ISPC(),
		Offset,
		&KeyToChunk,
		Iterator.Flags_Array.GetData(),
		Iterator.IndirectIndex_Array.GetData(),
		Iterator.AlphaX_Array.GetData(),
		Iterator.AlphaY_Array.GetData(),
		Iterator.AlphaZ_Array.GetData(),
		Iterator.PositionX_Array.GetData(),
		Iterator.PositionY_Array.GetData(),
		Iterator.PositionZ_Array.GetData(),
		Iterator.Chunks.GetData(),
		&NumChunks);

	check(NumChunks <= Iterator.Chunks.Num());
	Iterator.Chunks.SetNum(NumChunks, EAllowShrinking::No);

#if VOXEL_DEBUG
	ensure(Iterator.Chunks == Iterator2.Chunks);
	ensure(Iterator.Flags_Array == Iterator2.Flags_Array);
	ensure(Iterator.IndirectIndex_Array == Iterator2.IndirectIndex_Array);
	ensure(Iterator.AlphaX_Array == Iterator2.AlphaX_Array);
	ensure(Iterator.AlphaY_Array == Iterator2.AlphaY_Array);
	ensure(Iterator.AlphaZ_Array == Iterator2.AlphaZ_Array);
	ensure(Iterator.PositionX_Array == Iterator2.PositionX_Array);
	ensure(Iterator.PositionY_Array == Iterator2.PositionY_Array);
	ensure(Iterator.PositionZ_Array == Iterator2.PositionZ_Array);
#endif

	return Iterator;
}

FVoxelVolumeChunkTreeIterator FVoxelVolumeChunkTreeIterator::Create(
	const TVoxelMap<FIntVector, void*>& KeyToChunk,
	const FVoxelVolumeSparseQuery& Query,
	const FVoxelVolumeTransform& SculptToQuery,
	const double Offset)
{
	VOXEL_FUNCTION_COUNTER_NUM(Query.Num());

	FVoxelVolumeChunkTreeIterator Iterator;
	Iterator.Reserve(Query.Num());

	FIntVector LastChunkKey = FIntVector(MAX_int32);
	const void* LastChunk = nullptr;

	for (int32 Index = 0; Index < Query.Num(); Index++)
	{
		const FVector3d Position = SculptToQuery.InverseTransform(Query.Positions[Index]);

		Iterator.Set(Index, Query.GetIndirectIndex(Index), Position + Offset, [&](const FIntVector& ChunkKey)
		{
			if (ChunkKey != LastChunkKey)
			{
				LastChunk = KeyToChunk.FindRef(ChunkKey);
				LastChunkKey = ChunkKey;
			}
			return LastChunk;
		});
	}

	return Iterator;
}

FVoxelVolumeChunkTreeIterator FVoxelVolumeChunkTreeIterator::Create(
	const TVoxelMap<FIntVector, void*>& KeyToChunk,
	const FVoxelDoubleVectorBuffer& Positions,
	const double Offset)
{
	VOXEL_FUNCTION_COUNTER_NUM(Positions.Num());

	FVoxelVolumeChunkTreeIterator Iterator;
	Iterator.Reserve(Positions.Num());

	FIntVector LastChunkKey = FIntVector(MAX_int32);
	const void* LastChunk = nullptr;

	for (int32 Index = 0; Index < Positions.Num(); Index++)
	{
		Iterator.Set(Index, Index, Positions[Index] + Offset, [&](const FIntVector& ChunkKey)
		{
			if (ChunkKey != LastChunkKey)
			{
				LastChunk = KeyToChunk.FindRef(ChunkKey);
				LastChunkKey = ChunkKey;
			}
			return LastChunk;
		});
	}

	return Iterator;
}