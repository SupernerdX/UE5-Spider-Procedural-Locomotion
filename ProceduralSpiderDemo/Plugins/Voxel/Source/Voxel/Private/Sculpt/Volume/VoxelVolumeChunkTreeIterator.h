// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelStampQuery.h"
#include "VoxelStampTransform.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"

class FVoxelVolumeChunkTreeIterator : public FVoxelVolumeChunkDefinitions
{
public:
	static FVoxelVolumeChunkTreeIterator Create(
		const TVoxelMap<FIntVector, void*>& KeyToChunk,
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& SculptToQuery,
		double Offset);

	static FVoxelVolumeChunkTreeIterator Create(
		const TVoxelMap<FIntVector, void*>& KeyToChunk,
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& SculptToQuery,
		double Offset);

	static FVoxelVolumeChunkTreeIterator Create(
		const TVoxelMap<FIntVector, void*>& KeyToChunk,
		const FVoxelDoubleVectorBuffer& Positions,
		double Offset);

protected:
	TVoxelArray<const void*> Chunks;

	TVoxelArray<uint8> Flags_Array;
	TVoxelArray<int32> IndirectIndex_Array;
	TVoxelArray<float> AlphaX_Array;
	TVoxelArray<float> AlphaY_Array;
	TVoxelArray<float> AlphaZ_Array;
	TVoxelArray<int32> PositionX_Array;
	TVoxelArray<int32> PositionY_Array;
	TVoxelArray<int32> PositionZ_Array;

	FORCEINLINE static int32 GetIndex(
		const int32 X,
		const int32 Y,
		const int32 Z)
	{
		return FVoxelUtilities::Get3DIndex<int32>(ChunkSize, FIntVector(X, Y, Z) & ((1u << ChunkSizeLog2) - 1));
	}

	void Reserve(int32 Num);

	template<typename FindChunkType>
	void Set(
		int32 Index,
		int32 IndirectIndex,
		const FVector3d& Position,
		const FindChunkType& FindChunk);

	friend struct FVoxelVolumeChunkTreeImpl;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename ChunkType>
class TVoxelVolumeChunkTreeIterator : public FVoxelVolumeChunkTreeIterator
{
public:
	static TVoxelVolumeChunkTreeIterator Create(
		const TVoxelMap<FIntVector, ChunkType*>& KeyToChunk,
		const FVoxelVolumeBulkQuery& Query,
		const FVoxelVolumeTransform& SculptToQuery,
		const double Offset)
	{
		return ReinterpretCastRef<TVoxelVolumeChunkTreeIterator>(FVoxelVolumeChunkTreeIterator::Create(
			ReinterpretCastRef<TVoxelMap<FIntVector, void*>>(KeyToChunk),
			Query,
			SculptToQuery,
			Offset));
	}

	static TVoxelVolumeChunkTreeIterator Create(
		const TVoxelMap<FIntVector, ChunkType*>& KeyToChunk,
		const FVoxelVolumeSparseQuery& Query,
		const FVoxelVolumeTransform& SculptToQuery,
		const double Offset)
	{
		return ReinterpretCastRef<TVoxelVolumeChunkTreeIterator>(FVoxelVolumeChunkTreeIterator::Create(
			ReinterpretCastRef<TVoxelMap<FIntVector, void*>>(KeyToChunk),
			Query,
			SculptToQuery,
			Offset));
	}

	static TVoxelVolumeChunkTreeIterator Create(
		const TVoxelMap<FIntVector, ChunkType*>& KeyToChunk,
		const FVoxelDoubleVectorBuffer& Positions,
		const double Offset)
	{
		return ReinterpretCastRef<TVoxelVolumeChunkTreeIterator>(FVoxelVolumeChunkTreeIterator::Create(
			ReinterpretCastRef<TVoxelMap<FIntVector, void*>>(KeyToChunk),
			Positions,
			Offset));
	}

public:
	template<
		typename Interp0DType,
		typename Interp1DType,
		typename Interp2DType,
		typename Interp3DType>
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
			float)> &&

		LambdaHasSignature_V<Interp3DType, void(
			int32,
			const ChunkType*, int32,
			const ChunkType*, int32,
			const ChunkType*, int32,
			const ChunkType*, int32,
			const ChunkType*, int32,
			const ChunkType*, int32,
			const ChunkType*, int32,
			const ChunkType*, int32,
			float,
			float,
			float)>
	)
	void Iterate(
		Interp0DType Interp0,
		Interp1DType Interp1,
		Interp2DType Interp2,
		Interp3DType Interp3) const
	{
		VOXEL_FUNCTION_COUNTER_NUM(Flags_Array.Num());

		int32 ChunkIndex = 0;
		for (int32 Index = 0; Index < Flags_Array.Num(); Index++)
		{
			const uint8 Flag = Flags_Array[Index];
			const int32 IndirectIndex = IndirectIndex_Array[Index];
			const float AlphaX = AlphaX_Array[Index];
			const float AlphaY = AlphaY_Array[Index];
			const float AlphaZ = AlphaZ_Array[Index];
			const int32 PositionX = PositionX_Array[Index];
			const int32 PositionY = PositionY_Array[Index];
			const int32 PositionZ = PositionZ_Array[Index];

			const uint32 HasMaxPosition = Flag >> 3;
			const uint32 HasMaxChunk = Flag & 0b111;
			checkVoxelSlow((HasMaxPosition & HasMaxChunk) == HasMaxChunk);

			switch (HasMaxPosition)
			{
			default: VOXEL_ASSUME(false);
			case 0b000:
			{
				const ChunkType* Chunk0;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b000:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX + 0, PositionY, PositionZ);

				Interp0(IndirectIndex, Chunk0, Index0);
			}
			break;
			case 0b001:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b000:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
				}
				break;
				case 0b001:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX + 0, PositionY, PositionZ);
				const int32 Index1 = GetIndex(PositionX + 1, PositionY, PositionZ);

				Interp1(
					IndirectIndex,
					Chunk0, Index0,
					Chunk1, Index1,
					AlphaX);
			}
			break;
			case 0b010:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b000:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
				}
				break;
				case 0b010:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX, PositionY + 0, PositionZ);
				const int32 Index1 = GetIndex(PositionX, PositionY + 1, PositionZ);

				Interp1(
					IndirectIndex,
					Chunk0, Index0,
					Chunk1, Index1,
					AlphaY);
			}
			break;
			case 0b011:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;
				const ChunkType* Chunk2;
				const ChunkType* Chunk3;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b000:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = Chunk0;
					Chunk3 = Chunk0;
				}
				break;
				case 0b001:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = Chunk0;
					Chunk3 = Chunk1;
				}
				break;
				case 0b010:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = Chunk2;
				}
				break;
				case 0b011:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX + 0, PositionY + 0, PositionZ);
				const int32 Index1 = GetIndex(PositionX + 1, PositionY + 0, PositionZ);
				const int32 Index2 = GetIndex(PositionX + 0, PositionY + 1, PositionZ);
				const int32 Index3 = GetIndex(PositionX + 1, PositionY + 1, PositionZ);

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
			case 0b100:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b000:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
				}
				break;
				case 0b100:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX, PositionY, PositionZ + 0);
				const int32 Index1 = GetIndex(PositionX, PositionY, PositionZ + 1);

				Interp1(
					IndirectIndex,
					Chunk0, Index0,
					Chunk1, Index1,
					AlphaZ);
			}
			break;
			case 0b101:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;
				const ChunkType* Chunk2;
				const ChunkType* Chunk3;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b000:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = Chunk0;
					Chunk3 = Chunk0;
				}
				break;
				case 0b001:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = Chunk0;
					Chunk3 = Chunk1;
				}
				break;
				case 0b100:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = Chunk2;
				}
				break;
				case 0b101:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX + 0, PositionY, PositionZ + 0);
				const int32 Index1 = GetIndex(PositionX + 1, PositionY, PositionZ + 0);
				const int32 Index2 = GetIndex(PositionX + 0, PositionY, PositionZ + 1);
				const int32 Index3 = GetIndex(PositionX + 1, PositionY, PositionZ + 1);

				Interp2(
					IndirectIndex,
					Chunk0, Index0,
					Chunk1, Index1,
					Chunk2, Index2,
					Chunk3, Index3,
					AlphaX,
					AlphaZ);
			}
			break;
			case 0b110:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;
				const ChunkType* Chunk2;
				const ChunkType* Chunk3;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b000:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = Chunk0;
					Chunk3 = Chunk0;
				}
				break;
				case 0b010:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = Chunk0;
					Chunk3 = Chunk1;
				}
				break;
				case 0b100:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = Chunk2;
				}
				break;
				case 0b110:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX, PositionY + 0, PositionZ + 0);
				const int32 Index1 = GetIndex(PositionX, PositionY + 1, PositionZ + 0);
				const int32 Index2 = GetIndex(PositionX, PositionY + 0, PositionZ + 1);
				const int32 Index3 = GetIndex(PositionX, PositionY + 1, PositionZ + 1);

				Interp2(
					IndirectIndex,
					Chunk0, Index0,
					Chunk1, Index1,
					Chunk2, Index2,
					Chunk3, Index3,
					AlphaY,
					AlphaZ);
			}
			break;
			case 0b111:
			{
				const ChunkType* Chunk0;
				const ChunkType* Chunk1;
				const ChunkType* Chunk2;
				const ChunkType* Chunk3;
				const ChunkType* Chunk4;
				const ChunkType* Chunk5;
				const ChunkType* Chunk6;
				const ChunkType* Chunk7;

				switch (HasMaxChunk)
				{
				default: VOXEL_ASSUME(false);
				case 0b000:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = Chunk0;
					Chunk3 = Chunk0;
					Chunk4 = Chunk0;
					Chunk5 = Chunk0;
					Chunk6 = Chunk0;
					Chunk7 = Chunk0;
				}
				break;
				case 0b001:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = Chunk0;
					Chunk3 = Chunk1;
					Chunk4 = Chunk0;
					Chunk5 = Chunk1;
					Chunk6 = Chunk0;
					Chunk7 = Chunk1;
				}
				break;
				case 0b010:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = Chunk2;
					Chunk4 = Chunk0;
					Chunk5 = Chunk0;
					Chunk6 = Chunk2;
					Chunk7 = Chunk2;
				}
				break;
				case 0b011:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk4 = Chunk0;
					Chunk5 = Chunk1;
					Chunk6 = Chunk2;
					Chunk7 = Chunk3;
				}
				break;
				case 0b100:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = Chunk0;
					Chunk3 = Chunk0;
					Chunk4 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk5 = Chunk4;
					Chunk6 = Chunk4;
					Chunk7 = Chunk4;
				}
				break;
				case 0b101:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = Chunk0;
					Chunk3 = Chunk1;
					Chunk4 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk5 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk6 = Chunk4;
					Chunk7 = Chunk5;
				}
				break;
				case 0b110:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = Chunk0;
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = Chunk2;
					Chunk4 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk5 = Chunk4;
					Chunk6 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk7 = Chunk6;
				}
				break;
				case 0b111:
				{
					Chunk0 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk1 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk2 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk3 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk4 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk5 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk6 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
					Chunk7 = static_cast<const ChunkType*>(Chunks[ChunkIndex++]);
				}
				break;
				}

				const int32 Index0 = GetIndex(PositionX + 0, PositionY + 0, PositionZ + 0);
				const int32 Index1 = GetIndex(PositionX + 1, PositionY + 0, PositionZ + 0);
				const int32 Index2 = GetIndex(PositionX + 0, PositionY + 1, PositionZ + 0);
				const int32 Index3 = GetIndex(PositionX + 1, PositionY + 1, PositionZ + 0);
				const int32 Index4 = GetIndex(PositionX + 0, PositionY + 0, PositionZ + 1);
				const int32 Index5 = GetIndex(PositionX + 1, PositionY + 0, PositionZ + 1);
				const int32 Index6 = GetIndex(PositionX + 0, PositionY + 1, PositionZ + 1);
				const int32 Index7 = GetIndex(PositionX + 1, PositionY + 1, PositionZ + 1);

				Interp3(
					IndirectIndex,
					Chunk0, Index0,
					Chunk1, Index1,
					Chunk2, Index2,
					Chunk3, Index3,
					Chunk4, Index4,
					Chunk5, Index5,
					Chunk6, Index6,
					Chunk7, Index7,
					AlphaX,
					AlphaY,
					AlphaZ);
			}
			break;
			}
		}
		checkVoxelSlow(ChunkIndex == Chunks.Num());
	}
};