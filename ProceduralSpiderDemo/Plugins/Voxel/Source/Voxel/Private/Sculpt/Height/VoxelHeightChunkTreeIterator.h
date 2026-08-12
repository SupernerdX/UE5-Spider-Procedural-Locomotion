// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampQuery.h"
#include "VoxelStampTransform.h"
#include "Sculpt/Height/VoxelHeightChunkDefinitions.h"

class FVoxelHeightChunkTreeIterator : public FVoxelHeightChunkDefinitions
{
public:
	static FVoxelHeightChunkTreeIterator Create(
		const TVoxelMap<FIntPoint, void*>& KeyToChunk,
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& SculptToQuery,
		double Offset);

	static FVoxelHeightChunkTreeIterator Create(
		const TVoxelMap<FIntPoint, void*>& KeyToChunk,
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& SculptToQuery,
		double Offset);

	static FVoxelHeightChunkTreeIterator Create(
		const TVoxelMap<FIntPoint, void*>& KeyToChunk,
		const FVoxelDoubleVector2DBuffer& Positions,
		double Offset);

protected:
	TVoxelArray<const void*> Chunks;

	TVoxelArray<uint8> Flags_Array;
	TVoxelArray<int32> IndirectIndex_Array;
	TVoxelArray<float> AlphaX_Array;
	TVoxelArray<float> AlphaY_Array;
	TVoxelArray<int32> PositionX_Array;
	TVoxelArray<int32> PositionY_Array;

	FORCEINLINE static int32 GetIndex(
		const int32 X,
		const int32 Y)
	{
		return FVoxelUtilities::Get2DIndex<int32>(ChunkSize, FIntPoint(
			X & ((1u << ChunkSizeLog2) - 1),
			Y & ((1u << ChunkSizeLog2) - 1)));
	}

	void Reserve(int32 Num);

	template<typename FindChunkType>
	void Set(
		int32 Index,
		int32 IndirectIndex,
		const FVector2d& Position,
		const FindChunkType& FindChunk);

	friend struct FVoxelHeightChunkTreeImpl;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename ChunkType>
class TVoxelHeightChunkTreeIterator : public FVoxelHeightChunkTreeIterator
{
public:
	static TVoxelHeightChunkTreeIterator Create(
		const TVoxelMap<FIntPoint, ChunkType*>& KeyToChunk,
		const FVoxelHeightBulkQuery& Query,
		const FVoxelHeightTransform& SculptToQuery,
		const double Offset)
	{
		return ReinterpretCastRef<TVoxelHeightChunkTreeIterator>(FVoxelHeightChunkTreeIterator::Create(
			ReinterpretCastRef<TVoxelMap<FIntPoint, void*>>(KeyToChunk),
			Query,
			SculptToQuery,
			Offset));
	}

	static TVoxelHeightChunkTreeIterator Create(
		const TVoxelMap<FIntPoint, ChunkType*>& KeyToChunk,
		const FVoxelHeightSparseQuery& Query,
		const FVoxelHeightTransform& SculptToQuery,
		const double Offset)
	{
		return ReinterpretCastRef<TVoxelHeightChunkTreeIterator>(FVoxelHeightChunkTreeIterator::Create(
			ReinterpretCastRef<TVoxelMap<FIntPoint, void*>>(KeyToChunk),
			Query,
			SculptToQuery,
			Offset));
	}

	static TVoxelHeightChunkTreeIterator Create(
		const TVoxelMap<FIntPoint, ChunkType*>& KeyToChunk,
		const FVoxelDoubleVector2DBuffer& Positions,
		const double Offset)
	{
		return ReinterpretCastRef<TVoxelHeightChunkTreeIterator>(FVoxelHeightChunkTreeIterator::Create(
			ReinterpretCastRef<TVoxelMap<FIntPoint, void*>>(KeyToChunk),
			Positions,
			Offset));
	}

public:
	template<
		typename Interp0DType,
		typename Interp1DType,
		typename Interp2DType>
		requires
		(
			LambdaHasSignature_V<Interp0DType, void(
				int32,
				const ChunkType*, int32)> &&

			LambdaHasSignature_V<Interp1DType, void(
				int32,
				const ChunkType*, int32,
				const ChunkType*, int32,
				float)> &&

			LambdaHasSignature_V<Interp2DType, void(
				int32,
				const ChunkType*, int32,
				const ChunkType*, int32,
				const ChunkType*, int32,
				const ChunkType*, int32,
				float,
				float)>
		)
	void Iterate(
		Interp0DType Interp0,
		Interp1DType Interp1,
		Interp2DType Interp2) const
	{
		VOXEL_FUNCTION_COUNTER_NUM(Flags_Array.Num());

		int32 ChunkIndex = 0;
		for (int32 Index = 0; Index < Flags_Array.Num(); Index++)
		{
			const uint8 Flag = Flags_Array[Index];
			const int32 IndirectIndex = IndirectIndex_Array[Index];
			const float AlphaX = AlphaX_Array[Index];
			const float AlphaY = AlphaY_Array[Index];
			const int32 PositionX = PositionX_Array[Index];
			const int32 PositionY = PositionY_Array[Index];

			const uint32 HasMaxPosition = Flag >> 2;
			const uint32 HasMaxChunk = Flag & 0b11;
			checkVoxelSlow((HasMaxPosition & HasMaxChunk) == HasMaxChunk);

			switch (HasMaxPosition)
			{
			default: VOXEL_ASSUME(false);
			case 0b00:
			{
				const ChunkType* Chunk0;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b00:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX + 0, PositionY);

				Interp0(IndirectIndex, Chunk0, Index0);
			}
			break;
			case 0b01:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b00:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
				}
				break;
				case 0b01:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX + 0, PositionY);
				const int32 Index1 = GetIndex(PositionX + 1, PositionY);

				Interp1(
					IndirectIndex,
					Chunk0, Index0,
					Chunk1, Index1,
					AlphaX);
			}
			break;
			case 0b10:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b00:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
				}
				break;
				case 0b10:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX, PositionY + 0);
				const int32 Index1 = GetIndex(PositionX, PositionY + 1);

				Interp1(
					IndirectIndex,
					Chunk0, Index0,
					Chunk1, Index1,
					AlphaY);
			}
			break;
			case 0b11:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;
				const ChunkType* Chunk2;
				const ChunkType* Chunk3;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b00:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = Chunk0;
					Chunk3 = Chunk0;
				}
				break;
				case 0b01:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = Chunk0;
					Chunk3 = Chunk1;
				}
				break;
				case 0b10:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = Chunk2;
				}
				break;
				case 0b11:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX + 0, PositionY + 0);
				const int32 Index1 = GetIndex(PositionX + 1, PositionY + 0);
				const int32 Index2 = GetIndex(PositionX + 0, PositionY + 1);
				const int32 Index3 = GetIndex(PositionX + 1, PositionY + 1);

				Interp2(
					IndirectIndex,
					Chunk0, Index0,
					Chunk1, Index1,
					Chunk2, Index2,
					Chunk3, Index3,
					AlphaX,
					AlphaY);
			}
			break;
			}
		}
		checkVoxelSlow(ChunkIndex == Chunks.Num());
	}
};