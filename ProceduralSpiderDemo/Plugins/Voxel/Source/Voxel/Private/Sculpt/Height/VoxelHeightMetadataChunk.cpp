// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightMetadataChunk.h"
#include "Sculpt/VoxelSculptVersion.h"

DEFINE_VOXEL_INSTANCE_COUNTER(FVoxelHeightMetadataChunk);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelHeightMetadata_Memory);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelHeightMetadataChunk::FVoxelHeightMetadataChunk()
{
	UpdateStats();
}

void FVoxelHeightMetadataChunk::Serialize(
	FArchive& Ar,
	const int32 Version)
{
	VOXEL_FUNCTION_COUNTER();

	if (Version < FVoxelSculptVersion::MergeHeightSculptVersions)
	{
		using FVersion = DECLARE_VOXEL_VERSION
		(
			FirstVersion
		);

		int32 LegacyVersion = FVersion::LatestVersion;
		Ar << LegacyVersion;
	}

	Ar << Alphas;
	FVoxelBuffer::Serialize(Ar, Buffer);
}

void FVoxelHeightMetadataChunk::GetAverageValue(
	const FVoxelMetadataRef& MetadataRef,
	float& OutAlpha,
	FVoxelBuffer& OutBuffer,
	const int32 IndexInBuffer) const
{
	VOXEL_FUNCTION_COUNTER();

	uint8 MaxAlpha = 0;
	{
		int32 AlphaSum = 0;
		for (const uint8 Alpha : Alphas)
		{
			AlphaSum += Alpha;
			MaxAlpha = FMath::Max(MaxAlpha, Alpha);
		}
		OutAlpha = AlphaSum / (255.f * ChunkCount);

		checkVoxelSlow(0.f <= OutAlpha && OutAlpha <= 1.f);
	}

	if (!Buffer)
	{
		return;
	}

	SwitchType(MetadataRef, [&]<typename InnerType>()
	{
		const TVoxelBufferType<InnerType>& TypedBuffer = Buffer->AsChecked<TVoxelBufferType<InnerType>>();

		InnerType Sum = FVoxelUtilities::MakeSafe<InnerType>();

		for (int32 Index = 0; Index < ChunkCount; Index++)
		{
			Sum += TypedBuffer[Index] * Alphas[Index] / float(MaxAlpha);
		}

		TVoxelBufferType<InnerType>& TypedOutBuffer = OutBuffer.AsChecked<TVoxelBufferType<InnerType>>();
		TypedOutBuffer.Set(IndexInBuffer, Sum / float(ChunkCount));
	});
}

int64 FVoxelHeightMetadataChunk::GetAllocatedSize() const
{
	int64 AllocatedSize = 0;
	AllocatedSize += Alphas.GetAllocatedSize();
	if (Buffer)
	{
		AllocatedSize += Buffer->GetAllocatedSize();
	}
	return AllocatedSize;
}

TVoxelRefCountPtr<FVoxelHeightMetadataChunk> FVoxelHeightMetadataChunk::Clone() const
{
	VOXEL_FUNCTION_COUNTER();

	FVoxelHeightMetadataChunk* Result = new FVoxelHeightMetadataChunk();
	Result->Alphas = Alphas;
	if (ensure(Buffer))
	{
		Result->Buffer = Buffer->MakeDeepCopy();
	}

	Result->UpdateStats();
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelRefCountPtr<FVoxelHeightMetadataChunk> FVoxelHeightMetadataChunk::Create(
	const TConstVoxelArrayView<float> Alphas,
	const FVoxelBuffer& Buffer,
	const FIntPoint& Offset,
	const FIntPoint& Size)
{
	VOXEL_FUNCTION_COUNTER();

	TVoxelRefCountPtr<FVoxelHeightMetadataChunk> Chunk = new FVoxelHeightMetadataChunk();

	TVoxelArray<int32> Indirection;
	FVoxelUtilities::SetNumFast(Indirection, ChunkCount);
	{
		VOXEL_SCOPE_COUNTER("Build indirection");

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

				Chunk->Alphas[IndexInChunk] = FVoxelUtilities::FloatToUINT8(Alphas[IndexInSculpt]);
				Indirection[IndexInChunk] = IndexInSculpt;
			}
		}
	}

	if (FVoxelUtilities::AllEqual(Chunk->Alphas, 0))
	{
		return nullptr;
	}

	Chunk->Buffer = Buffer.Gather(Indirection);
	return Chunk;
}