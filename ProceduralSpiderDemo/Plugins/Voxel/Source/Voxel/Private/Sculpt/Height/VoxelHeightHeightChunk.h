// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Height/VoxelHeightChunkDefinitions.h"

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelHeight_Memory, "Voxel Sculpt Height Memory");

struct FVoxelHeightHeightChunk
	: public FVoxelHeightChunkDefinitions
	, public TVoxelRefCountThis<FVoxelHeightHeightChunk>
{
public:
	VOXEL_COUNT_INSTANCES();
	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelHeight_Memory);

public:
	struct FData
	{
		const int32 BitDepth;
		const float Start;
		const float Scale;
		const uint32 Mask;

		uint32 Data[0];

		FData(
			const int32 BitDepth,
			const float Start,
			const float Scale)
			: BitDepth(BitDepth)
			, Start(Start)
			, Scale(Scale)
			, Mask((uint32(1) << BitDepth) - 1)
		{
			checkVoxelSlow(1 <= BitDepth && BitDepth <= 25);
		}

		FORCEINLINE static int32 GetNumWords(const int32 BitDepth)
		{
			return FMath::DivideAndRoundUp(ChunkCount * BitDepth, 32);
		}

		FORCEINLINE FDoubleInterval GetRange() const
		{
			return FDoubleInterval(Start, Start + float(Mask) * Scale);
		}

		FORCEINLINE void SetHeight(
			const int32 Index,
			const float Height)
		{
			checkVoxelSlow(0 <= Index && Index < ChunkCount);

			if (BitDepth == 0)
			{
				checkVoxelSlow(Start == Height);
				return;
			}

			uint32 PackedValue;
			if (!FVoxelUtilities::IsNaN(Height))
			{
				const FDoubleInterval Range = GetRange();

				//ensureVoxelSlow(Min <= Height && Height <= Max);

				// Clamp height to valid range
				const float ClampedHeight = FMath::Clamp(Height, Range.Min, Range.Max);

				// Convert float height to packed integer value
				const float NormalizedHeight = (ClampedHeight - Start) / Scale;
				const int32 UnclampedValue = FMath::RoundToInt(NormalizedHeight);
				ensureVoxelSlow(0 <= UnclampedValue && UnclampedValue <= int32(Mask));

				PackedValue = uint32(FMath::Clamp(UnclampedValue, 0, int32(Mask)));
			}
			else
			{
				PackedValue = Mask;
			}

			// Calculate bit position
			const int32 BitIndex = Index * BitDepth;
			const int32 ByteIndex = BitIndex / 8;
			const int32 BitOffset = BitIndex % 8;

			// Perform unaligned 32-bit load
			uint32 RawData;
			memcpy(&RawData, reinterpret_cast<uint8*>(Data) + ByteIndex, sizeof(uint32));

			// Clear the bits we're about to write
			const uint32 ClearMask = ~(Mask << BitOffset);
			RawData = (RawData & ClearMask) | (PackedValue << BitOffset);

			// Write back the modified data
			memcpy(reinterpret_cast<uint8*>(Data) + ByteIndex, &RawData, sizeof(uint32));
		}

		FORCEINLINE float GetHeight(const int32 Index) const
		{
			checkVoxelSlow(0 <= Index && Index < ChunkCount);
			checkVoxelSlow(Mask == ((uint32(1) << BitDepth) - 1));

			// Calculate bit position
			const int32 BitIndex = Index * BitDepth;
			const int32 ByteIndex = BitIndex / 8;
			const int32 BitOffset = BitIndex % 8;

			// Perform unaligned 32-bit load
			// Since BitDepth <= 16 and BitOffset < 8, we need at most 16+7=23 bits,
			// which fits in a single 32-bit load
			uint32 RawData;
			memcpy(&RawData, reinterpret_cast<const uint8*>(Data) + ByteIndex, sizeof(uint32));

			// Extract the packed value with a simple shift and mask
			const uint32 PackedValue = (RawData >> BitOffset) & Mask;

			if (PackedValue == Mask)
			{
				return FVoxelUtilities::NaNf();
			}

			return Start + float(PackedValue) * Scale;
		}

		float GetAverageHeight() const;
	};

	bool bIsRelative = false;
	union
	{
		FData PrivateData;
		uint8 Storage;
	};

public:
	FORCEINLINE FData& GetData()
	{
		return PrivateData;
	}
	FORCEINLINE const FData& GetData() const
	{
		return PrivateData;
	}

public:
	FORCEINLINE float GetHeight(const int32 Index) const
	{
		return GetData().GetHeight(Index);
	}

private:
	enum class EInternal {};
	explicit FVoxelHeightHeightChunk(EInternal)
	{
	}
	UE_NONCOPYABLE(FVoxelHeightHeightChunk);

	static TVoxelRefCountPtr<FVoxelHeightHeightChunk> NewPacked(
		int32 BitDepth,
		float Start,
		float Scale,
		bool bRelative);

public:
	void Save(FArchive& Ar) const;
	static TVoxelRefCountPtr<FVoxelHeightHeightChunk> Load(FArchive& Ar);

	int64 GetAllocatedSize() const;

private:
	struct FFlag
	{
		uint8 Version : 5;
		uint8 bIsRelative : 1;
		uint8 Padding : 2;

		FFlag()
		{
			ReinterpretCastRef<uint8>(*this) = 0;
		}
	};

public:
	static TVoxelRefCountPtr<FVoxelHeightHeightChunk> CreatePacked(
		TConstVoxelArrayView<float> Heights,
		const FIntPoint& Offset,
		const FIntPoint& Size,
		float TargetPrecision,
		bool bRelative);
};