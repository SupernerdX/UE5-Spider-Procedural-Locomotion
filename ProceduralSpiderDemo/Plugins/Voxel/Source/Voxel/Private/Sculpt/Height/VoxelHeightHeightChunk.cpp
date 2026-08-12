// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightHeightChunk.h"
#include "Sculpt/VoxelSculptVersion.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelHeightHeightChunk);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelHeight_Memory);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float FVoxelHeightHeightChunk::FData::GetAverageHeight() const
{
	VOXEL_FUNCTION_COUNTER();

	float Sum = 0.f;
	int32 Count = 0;
	for (int32 Index = 0; Index < ChunkCount; Index++)
	{
		const float Height = GetHeight(Index);

		if (FVoxelUtilities::IsNaN(Height))
		{
			continue;
		}

		Sum += Height;
		Count++;
	}

	if (Count == 0)
	{
		return FVoxelUtilities::NaNf();
	}

	return Sum / Count;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelRefCountPtr<FVoxelHeightHeightChunk> FVoxelHeightHeightChunk::NewPacked(
	const int32 BitDepth,
	const float Start,
	const float Scale,
	const bool bRelative)
{
	const int32 BaseSize = offsetof(FVoxelHeightHeightChunk, PrivateData);

	// Add padding to allow a safe memcpy at the end
	const int32 NumWordsToAllocate = FData::GetNumWords(BitDepth) + 1;

	void* Allocation = FMemory::Malloc(BaseSize + sizeof(FData) + NumWordsToAllocate * sizeof(uint32));

	FVoxelHeightHeightChunk* Result = new(Allocation) FVoxelHeightHeightChunk(EInternal{});
	Result->bIsRelative = bRelative;
	new(&Result->PrivateData) FData(BitDepth, Start, Scale);
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelHeightHeightChunk::Save(FArchive& Ar) const
{
	VOXEL_FUNCTION_COUNTER();
	check(!Ar.IsLoading());

	FFlag Flag;
	Flag.Version = 0;
	Flag.bIsRelative = bIsRelative;
	Ar << ReinterpretCastRef<uint8>(Flag);

	Ar << ConstCast(GetData().BitDepth);
	Ar << ConstCast(GetData().Start);
	Ar << ConstCast(GetData().Scale);

	Ar.Serialize(&ConstCast(GetData().Data), FData::GetNumWords(GetData().BitDepth) * sizeof(uint32));
}

TVoxelRefCountPtr<FVoxelHeightHeightChunk> FVoxelHeightHeightChunk::Load(
	FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();
	check(Ar.IsLoading());

	FFlag Flag;
	Ar << ReinterpretCastRef<uint8>(Flag);
	ensure(Flag.Version == 0);

	int32 BitDepth = 0;
	float Start = 0;
	float Scale = 0;
	Ar << BitDepth;
	Ar << Start;
	Ar << Scale;

	TVoxelRefCountPtr<FVoxelHeightHeightChunk> Result = NewPacked(BitDepth, Start, Scale, Flag.bIsRelative);
	Ar.Serialize(Result->GetData().Data, FData::GetNumWords(BitDepth) * sizeof(uint32));
	return Result;
}

int64 FVoxelHeightHeightChunk::GetAllocatedSize() const
{
	return sizeof(FData) + FData::GetNumWords(GetData().BitDepth) * sizeof(uint32);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelRefCountPtr<FVoxelHeightHeightChunk> FVoxelHeightHeightChunk::CreatePacked(
	const TConstVoxelArrayView<float> Heights,
	const FIntPoint& Offset,
	const FIntPoint& Size,
	const float TargetPrecision,
	const bool bRelative)
{
	VOXEL_FUNCTION_COUNTER();
	check(Heights.Num() == Size.X * Size.Y);
	ensure(TargetPrecision >= 0);

	float MinHeight = MAX_flt;
	float MaxHeight = -MAX_flt;
	for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
	{
		for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
		{
			const int32 IndexInSculpt = FVoxelUtilities::Get2DIndex<int32>(
				Size,
				Offset.X + IndexX,
				Offset.Y + IndexY);

			const float Height = Heights[IndexInSculpt];
			if (FVoxelUtilities::IsNaN(Height))
			{
				continue;
			}

			MinHeight = FMath::Min(MinHeight, Height);
			MaxHeight = FMath::Max(MaxHeight, Height);
		}
	}

	if (MaxHeight < MinHeight)
	{
		return {};
	}

	int32 BitDepth = INLINE_LAMBDA
	{
		if (TargetPrecision == 0.f)
		{
			// Max precision
			return 25;
		}

		const int32 Result = FMath::CeilToInt(FMath::Log2((MaxHeight - MinHeight) / TargetPrecision + 1.f));
		return FMath::Clamp(Result, 1, 25);
	};

	const float PackedDataScale = INLINE_LAMBDA
	{
		const float Range = MaxHeight - MinHeight;
		if (Range == 0)
		{
			// Avoid NaNs
			return 1.f;
		}

		int32 NumValidValues = (uint32(1) << BitDepth) - 2;
		if (NumValidValues <= 0)
		{
			BitDepth++;
			NumValidValues = (uint32(1) << BitDepth) - 2;
		}

		return Range / float(NumValidValues);
	};

	TVoxelRefCountPtr<FVoxelHeightHeightChunk> Chunk = NewPacked(
		BitDepth,
		MinHeight,
		PackedDataScale,
		bRelative);

	for (int32 IndexY = 0; IndexY < ChunkSize; IndexY++)
	{
		for (int32 IndexX = 0; IndexX < ChunkSize; IndexX++)
		{
			const int32 IndexInChunk = FVoxelUtilities::Get2DIndex<int32>(
				ChunkSize,
				IndexX,
				IndexY);

			const int32 IndexInSculpt = FVoxelUtilities::Get2DIndex<int32>(
				Size,
				Offset.X + IndexX,
				Offset.Y + IndexY);

			Chunk->GetData().SetHeight(IndexInChunk, Heights[IndexInSculpt]);
		}
	}

	return Chunk;
}