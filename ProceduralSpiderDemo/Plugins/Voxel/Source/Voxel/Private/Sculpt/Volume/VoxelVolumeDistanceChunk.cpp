// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeDistanceChunk.h"
#include "Sculpt/VoxelSculptVersion.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelVolumeDistanceChunk);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelVolumeDistance_Memory);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float FVoxelVolumeDistanceChunk::FPackedData::GetAverageDistance() const
{
	VOXEL_FUNCTION_COUNTER();

	float Sum = 0.f;
	int32 Count = 0;
	for (int32 Index = 0; Index < ChunkCount; Index++)
	{
		const float Distance = GetDistance(Index);

		if (FVoxelUtilities::IsNaN(Distance))
		{
			continue;
		}

		Sum += Distance;
		Count++;
	}

	if (Count == 0)
	{
		return FVoxelUtilities::NaNf();
	}

	return Sum / Count;
}

FVoxelMovableDistances FVoxelVolumeDistanceChunk::FMovableData::GetAverageDistances() const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelMovableDistances Result;

	{
		float Sum = 0.f;
		int32 Count = 0;
		for (const float Distance : AdditiveDistances)
		{
			if (FVoxelUtilities::IsNaN(Distance))
			{
				continue;
			}

			Sum += Distance;
			Count++;
		}

		Result.Additive = Count == 0 ? FVoxelUtilities::NaNf() : Sum / Count;
	}

	{
		float Sum = 0.f;
		int32 Count = 0;
		for (const float Distance : SubtractiveDistances)
		{
			if (FVoxelUtilities::IsNaN(Distance))
			{
				continue;
			}

			Sum += Distance;
			Count++;
		}

		Result.Subtractive = Count == 0 ? FVoxelUtilities::NaNf() : Sum / Count;
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> FVoxelVolumeDistanceChunk::NewPacked(
	const int32 BitDepth,
	const float Start,
	const float Scale)
{
	const int32 BaseSize = offsetof(FVoxelVolumeDistanceChunk, PrivatePackedData);
	check(BaseSize == offsetof(FVoxelVolumeDistanceChunk, PrivateMovableData));

	// Add padding to allow a safe memcpy at the end
	const int32 NumWordsToAllocate = FPackedData::GetNumWords(BitDepth) + 1;

	void* Allocation = FMemory::Malloc(BaseSize + sizeof(FPackedData) + NumWordsToAllocate * sizeof(uint32));

	FVoxelVolumeDistanceChunk* Result = new(Allocation) FVoxelVolumeDistanceChunk(EInternal{});
	Result->bIsPacked = true;
	new(&Result->PrivatePackedData) FPackedData(BitDepth, Start, Scale);
	return Result;
}

TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> FVoxelVolumeDistanceChunk::NewMovable()
{
	const int32 BaseSize = offsetof(FVoxelVolumeDistanceChunk, PrivatePackedData);
	check(BaseSize == offsetof(FVoxelVolumeDistanceChunk, PrivateMovableData));

	void* Allocation = FMemory::Malloc(BaseSize + sizeof(FMovableData));

	FVoxelVolumeDistanceChunk* Result = new(Allocation) FVoxelVolumeDistanceChunk(EInternal{});
	Result->bIsPacked = false;
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVolumeDistanceChunk::Save(FArchive& Ar) const
{
	VOXEL_FUNCTION_COUNTER();
	check(!Ar.IsLoading());

	FFlag Flag;
	Flag.Version = 0;
	Flag.bIsPacked = bIsPacked;
	Ar << ReinterpretCastRef<uint8>(Flag);

	if (Flag.bIsPacked)
	{
		Ar << ConstCast(GetPacked().BitDepth);
		Ar << ConstCast(GetPacked().Start);
		Ar << ConstCast(GetPacked().Scale);

		Ar.Serialize(&ConstCast(GetPacked().Data), FPackedData::GetNumWords(GetPacked().BitDepth) * sizeof(uint32));
	}
	else
	{
		Ar.Serialize(&ConstCast(GetMovable()), sizeof(FMovableData));
	}
}

TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> FVoxelVolumeDistanceChunk::Load(
	FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();
	check(Ar.IsLoading());

	FFlag Flag;
	Ar << ReinterpretCastRef<uint8>(Flag);
	ensure(Flag.Version == 0);

	if (Flag.bIsPacked)
	{
		int32 BitDepth = 0;
		float Start = 0;
		float Scale = 0;
		Ar << BitDepth;
		Ar << Start;
		Ar << Scale;

		TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> Result = NewPacked(BitDepth, Start, Scale);
		Ar.Serialize(Result->GetPacked().Data, FPackedData::GetNumWords(BitDepth) * sizeof(uint32));
		return Result;
	}
	else
	{
		TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> Result = NewMovable();
		Ar.Serialize(&Result->GetMovable(), sizeof(FMovableData));
		return Result;
	}
}

int64 FVoxelVolumeDistanceChunk::GetAllocatedSize() const
{
	if (bIsPacked)
	{
		return sizeof(FPackedData) + FPackedData::GetNumWords(GetPacked().BitDepth) * sizeof(uint32);
	}
	else
	{
		return sizeof(FMovableData);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> FVoxelVolumeDistanceChunk::CreatePacked(
	const TConstVoxelArrayView<float> Distances,
	const FIntVector& Offset,
	const FIntVector& Size,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();
	check(Distances.Num() == Size.X * Size.Y * Size.Z);
	ensure(0 <= MaxErrorPercentage && MaxErrorPercentage <= 100);

	float MinDistance = MAX_flt;
	float MaxDistance = -MAX_flt;
	for (int32 IndexZ = 0; IndexZ < ChunkSize; IndexZ++)
	{
		for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
		{
			for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
			{
				const int32 IndexInSculpt = FVoxelUtilities::Get3DIndex<int32>(
					Size,
					Offset.X + IndexX,
					Offset.Y + IndexY,
					Offset.Z + IndexZ);

				const float Distance = Distances[IndexInSculpt];
				if (!ensureVoxelSlow(!FVoxelUtilities::IsNaN(Distance)))
				{
					continue;
				}

				MinDistance = FMath::Min(MinDistance, Distance);
				MaxDistance = FMath::Max(MaxDistance, Distance);
			}
		}
	}

	if (MaxDistance < MinDistance)
	{
		return {};
	}

	const int32 BitDepth = INLINE_LAMBDA
	{
		const float MaxAllowedError = (MaxErrorPercentage / 100.f) * FMath::Max(FMath::Abs(MinDistance), FMath::Abs(MaxDistance));

		if (MaxAllowedError == 0.f)
		{
			// Max precision
			return 24;
		}

		// The quantization step with N bits is Range / (2^N - 1)
		// We need: Range / (2^N - 1) <= MaxAllowedError
		// Therefore: 2^N - 1 >= Range / MaxAllowedError
		// So: N >= log2(Range / MaxAllowedError + 1)

		const int32 Result = FMath::CeilToInt(FMath::Log2((MaxDistance - MinDistance) / MaxAllowedError + 1.f));
		return FMath::Clamp(Result, 1, 24);
	};

	const float PackedDataScale = INLINE_LAMBDA
	{
		const float Range = MaxDistance - MinDistance;
		if (Range == 0)
		{
			// Avoid NaNs
			return 1.f;
		}

		return Range / float((uint32(1) << BitDepth) - 1);
	};

	TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> Chunk = NewPacked(
		BitDepth,
		MinDistance,
		PackedDataScale);

	for (int32 IndexZ = 0; IndexZ < ChunkSize; IndexZ++)
	{
		for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
		{
			for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
			{
				const int32 IndexInChunk = FVoxelUtilities::Get3DIndex<int32>(
					ChunkSize,
					IndexX,
					IndexY,
					IndexZ);

				const int32 IndexInSculpt = FVoxelUtilities::Get3DIndex<int32>(
					Size,
					Offset.X + IndexX,
					Offset.Y + IndexY,
					Offset.Z + IndexZ);

				const float Distance = Distances[IndexInSculpt];
				ensureVoxelSlow(!FVoxelUtilities::IsNaN(Distance));
				Chunk->GetPacked().SetDistance(IndexInChunk, Distance);
			}
		}
	}

	return Chunk;
}

TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> FVoxelVolumeDistanceChunk::CreateMovable(
	const TConstVoxelArrayView<float> AdditiveDistances,
	const TConstVoxelArrayView<float> SubtractiveDistances,
	const FIntVector& Offset,
	const FIntVector& Size)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> Chunk = NewMovable();
	Chunk->GetMovable().AdditiveDistances.View().CopyFrom3D<ChunkSize>(AdditiveDistances, Offset, Size);
	Chunk->GetMovable().SubtractiveDistances.View().CopyFrom3D<ChunkSize>(SubtractiveDistances, Offset, Size);
	return Chunk;
}