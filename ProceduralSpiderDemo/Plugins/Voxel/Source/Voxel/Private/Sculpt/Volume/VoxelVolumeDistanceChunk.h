// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Sculpt/Volume/VoxelVolumeChunkDefinitions.h"

DECLARE_VOXEL_MEMORY_STAT(VOXEL_API, STAT_VoxelVolumeDistance_Memory, "Voxel Sculpt Volume Distance Memory");

struct alignas(8) FVoxelMovableDistances
{
	float Additive = 0.f;
	float Subtractive = 0.f;

	FVoxelMovableDistances() = default;

	explicit FVoxelMovableDistances(const float Distance)
		: Additive(Distance)
		, Subtractive(-Distance)
	{
	}
	FVoxelMovableDistances(const float Additive, const float Subtractive)
		: Additive(Additive)
		, Subtractive(Subtractive)
	{
	}

	FORCEINLINE FVoxelMovableDistances operator+(const FVoxelMovableDistances& Other) const
	{
		return { Additive + Other.Additive, Subtractive + Other.Subtractive };
	}
	FORCEINLINE FVoxelMovableDistances operator-(const FVoxelMovableDistances& Other) const
	{
		return { Additive - Other.Additive, Subtractive - Other.Subtractive };
	}
};

FORCEINLINE FVoxelMovableDistances operator*(const float Scale, const FVoxelMovableDistances& Distances)
{
	return { Scale * Distances.Additive, Scale * Distances.Subtractive };
}

struct FVoxelVolumeDistanceChunk
	: public FVoxelVolumeChunkDefinitions
	, public TVoxelRefCountThis<FVoxelVolumeDistanceChunk>
{
public:
	VOXEL_COUNT_INSTANCES();
	VOXEL_ALLOCATED_SIZE_TRACKER(STAT_VoxelVolumeDistance_Memory);

public:
	struct FPackedData
	{
		const int32 BitDepth;
		const float Start;
		const float Scale;
		const uint32 Mask;

		uint32 Data[0];

		FPackedData(
			const int32 BitDepth,
			const float Start,
			const float Scale)
			: BitDepth(BitDepth)
			, Start(Start)
			, Scale(Scale)
			, Mask((uint32(1) << BitDepth) - 1)
		{
			checkVoxelSlow(1 <= BitDepth && BitDepth <= 24);
		}

		FORCEINLINE static int32 GetNumWords(const int32 BitDepth)
		{
			return FMath::DivideAndRoundUp(ChunkCount * BitDepth, 32);
		}

		FORCEINLINE void SetDistance(
			const int32 Index,
			const float Distance)
		{
			checkVoxelSlow(0 <= Index && Index < ChunkCount);

			if (BitDepth == 0)
			{
				checkVoxelSlow(Start == Distance);
				return;
			}

			const float Min = Start;
			const float Max = Start + float(Mask) * Scale;

			ensureVoxelSlow(!FVoxelUtilities::IsNaN(Distance));
			//ensureVoxelSlow(Min <= Distance && Distance <= Max);

			// Clamp distance to valid range
			const float ClampedDistance = FMath::Clamp(Distance, Start, Start + float(Mask) * Scale);

			// Convert float distance to packed integer value
			const float NormalizedDistance = (ClampedDistance - Start) / Scale;
			const int32 UnclampedValue = FMath::RoundToInt(NormalizedDistance);
			ensureVoxelSlow(0 <= UnclampedValue && UnclampedValue <= int32(Mask));

			const uint32 PackedValue = uint32(FMath::Clamp(UnclampedValue, 0, int32(Mask)));

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

		FORCEINLINE float GetDistance(const int32 Index) const
		{
			checkVoxelSlow(0 <= Index && Index < ChunkCount);
			checkVoxelSlow(Mask == ((uint32(1) << BitDepth) - 1));

			// Calculate bit position
			const int32 BitIndex = Index * BitDepth;
			const int32 ByteIndex = BitIndex / 8;
			const int32 BitOffset = BitIndex % 8;

			// Perform unaligned 32-bit load
			// Since BitDepth <= 24 and BitOffset < 8, we need at most 24+7=31 bits,
			// which fits in a single 32-bit load
			uint32 RawData;
			memcpy(&RawData, reinterpret_cast<const uint8*>(Data) + ByteIndex, sizeof(uint32));

			// Extract the packed value with a simple shift and mask
			const uint32 PackedValue = (RawData >> BitOffset) & Mask;

			return Start + float(PackedValue) * Scale;
		}

		float GetAverageDistance() const;
	};

	struct FMovableData
	{
		TVoxelStaticArray<float, ChunkCount> AdditiveDistances{ NoInit };
		TVoxelStaticArray<float, ChunkCount> SubtractiveDistances{ NoInit };

		FMovableData() = delete;

		FVoxelMovableDistances GetAverageDistances() const;
	};

	bool bIsPacked = false;

	union
	{
		FPackedData PrivatePackedData;
		FMovableData PrivateMovableData;
	};

public:
	FORCEINLINE FPackedData& GetPacked()
	{
		checkVoxelSlow(bIsPacked);
		return PrivatePackedData;
	}
	FORCEINLINE FMovableData& GetMovable()
	{
		checkVoxelSlow(!bIsPacked);
		return PrivateMovableData;
	}

	FORCEINLINE const FPackedData& GetPacked() const
	{
		checkVoxelSlow(bIsPacked);
		return PrivatePackedData;
	}
	FORCEINLINE const FMovableData& GetMovable() const
	{
		checkVoxelSlow(!bIsPacked);
		return PrivateMovableData;
	}

public:
	FORCEINLINE float GetPackedDistance(const int32 Index) const
	{
		checkVoxelSlow(bIsPacked);
		return GetPacked().GetDistance(Index);
	}
	FORCEINLINE FVoxelMovableDistances GetMovableDistancesWithFallback(const int32 Index) const
	{
		if (bIsPacked)
		{
			const float Distance = GetPacked().GetDistance(Index);

			return FVoxelMovableDistances(Distance);
		}

		return FVoxelMovableDistances(
			GetMovable().AdditiveDistances[Index],
			GetMovable().SubtractiveDistances[Index]);
	}

private:
	enum class EInternal {};
	explicit FVoxelVolumeDistanceChunk(EInternal)
	{
	}
	UE_NONCOPYABLE(FVoxelVolumeDistanceChunk);

	static TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> NewPacked(
		int32 BitDepth,
		float Start,
		float Scale);

	static TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> NewMovable();

public:
	void Save(FArchive& Ar) const;
	static TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> Load(FArchive& Ar);

	int64 GetAllocatedSize() const;

private:
	struct FFlag
	{
		uint8 Version : 5;
		uint8 bIsPacked : 1;
		uint8 Padding : 2;

		FFlag()
		{
			ReinterpretCastRef<uint8>(*this) = 0;
		}
	};

public:
	static TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> CreatePacked(
		TConstVoxelArrayView<float> Distances,
		const FIntVector& Offset,
		const FIntVector& Size,
		float MaxErrorPercentage);

	static TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> CreateMovable(
		TConstVoxelArrayView<float> AdditiveDistances,
		TConstVoxelArrayView<float> SubtractiveDistances,
		const FIntVector& Offset,
		const FIntVector& Size);
};