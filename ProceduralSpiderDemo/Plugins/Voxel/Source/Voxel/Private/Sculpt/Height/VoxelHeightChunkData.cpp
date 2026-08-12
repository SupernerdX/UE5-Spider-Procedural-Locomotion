// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Height/VoxelHeightChunkData.h"
#include "VoxelMetadata.h"
#include "Sculpt/Height/VoxelSculptHeightData.h"
#include "Sculpt/Height/VoxelHeightHeightChunk.h"
#include "Sculpt/VoxelSculptVersion.h"
#include "Surface/VoxelSurfaceTypeInterface.h"

bool FVoxelHeightChunkData::IsEmpty() const
{
	return
		!HeightChunk &&
		!SurfaceTypeChunk &&
		MetadataChunks.Num() == 0;
}

void FVoxelHeightChunkData::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	int32 Version = FVoxelSculptVersion::LatestVersion;
	Ar << Version;

	enum
	{
		Flags_HasHeightChunk = (1 << 0),
		Flags_HasSurfaceTypeChunk = (1 << 1)
	};
	uint8 Flags = 0;

	if (HeightChunk)
	{
		Flags |= Flags_HasHeightChunk;
	}
	if (SurfaceTypeChunk)
	{
		Flags |= Flags_HasSurfaceTypeChunk;
	}

	Ar << Flags;

	if (Flags & Flags_HasHeightChunk)
	{
		if (Ar.IsLoading())
		{
			HeightChunk = FVoxelHeightHeightChunk::Load(Ar);
		}
		else
		{
			HeightChunk->Save(Ar);
		}
	}
	if (Flags & Flags_HasSurfaceTypeChunk)
	{
		if (Ar.IsLoading())
		{
			SurfaceTypeChunk = new FVoxelHeightSurfaceTypeChunk();
		}

		ConstCast(*SurfaceTypeChunk).Serialize(Ar, Version);
	}

	int32 NumMetadatas = MetadataChunks.Num();
	Ar << NumMetadatas;

	MetadataChunks.SetNum(NumMetadatas);

	for (FMetadataChunk& MetadataChunk : MetadataChunks)
	{
		if (Ar.IsLoading())
		{
			MetadataChunk.Chunk = new FVoxelHeightMetadataChunk();
		}

		Ar << MetadataChunk.MetadataRef;
		ConstCast(*MetadataChunk.Chunk).Serialize(Ar, Version);
	}
}

void FVoxelHeightChunkData::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
{
	if (SurfaceTypeChunk.IsValid())
	{
		for (const FVoxelSurfaceType& SurfaceType : SurfaceTypeChunk->UsedSurfaceTypes)
		{
			OutObjects.Add(SurfaceType.GetSurfaceTypeInterface());
		}
	}

	for (const FMetadataChunk& MetadataChunk : MetadataChunks)
	{
		OutObjects.Add(MetadataChunk.MetadataRef.GetMetadata());
	}
}

FDoubleInterval FVoxelHeightChunkData::GetRange() const
{
	if (!HeightChunk)
	{
		return {};
	}

	return HeightChunk->GetData().GetRange();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelHeightChunkData FVoxelHeightChunkData::Downsample(
	const TVoxelMap<FVoxelHeightChunkKey, const FVoxelHeightChunkData*>& KeyToChunkData,
	const float TargetPrecision)
{
	VOXEL_FUNCTION_COUNTER();

	bool bHasHeights = false;
	bool bHasRelativeHeights = false;
	bool bHasSurfaceTypes = false;
	TVoxelInlineArray<FVoxelMetadataRef, 8> MetadataRefs;

	for (const auto& It : KeyToChunkData)
	{
		if (It.Value->HeightChunk)
		{
			bHasHeights = true;
			bHasRelativeHeights = It.Value->HeightChunk->bIsRelative;
		}
		if (It.Value->SurfaceTypeChunk)
		{
			bHasSurfaceTypes = true;
		}
		for (const FMetadataChunk& MetadataChunk : It.Value->MetadataChunks)
		{
			MetadataRefs.AddUnique(MetadataChunk.MetadataRef);
		}
	}

	FVoxelHeightChunkData Result;

	if (bHasHeights)
	{
		VOXEL_SCOPE_COUNTER("Heights");

		TVoxelStaticArray<float, ChunkCount> Heights{ FVoxelUtilities::NaNf() };

		for (const auto& It : KeyToChunkData)
		{
			if (!It.Value->HeightChunk)
			{
				continue;
			}

			Heights[It.Key.ToIndex()] = It.Value->HeightChunk->GetData().GetAverageHeight() / ChunkSize;
		}

		Result.HeightChunk = FVoxelHeightHeightChunk::CreatePacked(
			Heights,
			FIntPoint(0),
			FIntPoint(ChunkSize),
			TargetPrecision,
			bHasRelativeHeights);
	}

	if (bHasSurfaceTypes)
	{
		VOXEL_SCOPE_COUNTER("Surface types");

		TVoxelStaticArray<float, ChunkCount> Alphas{ ForceInit };
		TVoxelStaticArray<FVoxelSurfaceTypeBlend, ChunkCount> SurfaceTypes{ ForceInit };

		for (const auto& It : KeyToChunkData)
		{
			if (!It.Value->SurfaceTypeChunk)
			{
				continue;
			}

			It.Value->SurfaceTypeChunk->GetAverageSurfaceType(
				Alphas[It.Key.ToIndex()],
				SurfaceTypes[It.Key.ToIndex()]);
		}

		Result.SurfaceTypeChunk = FVoxelHeightSurfaceTypeChunk::Create(
			Alphas,
			SurfaceTypes,
			FIntPoint(0),
			FIntPoint(ChunkSize));
	}

	for (const FVoxelMetadataRef& MetadataRef : MetadataRefs)
	{
		VOXEL_SCOPE_COUNTER_FORMAT("Metadata %s", *MetadataRef.GetName());

		TVoxelStaticArray<float, ChunkCount> Alphas{ ForceInit };
		const TSharedRef<FVoxelBuffer> Buffer = MetadataRef.MakeDefaultBuffer(ChunkCount);

		for (const auto& It : KeyToChunkData)
		{
			for (const FMetadataChunk& MetadataChunk : It.Value->MetadataChunks)
			{
				if (MetadataChunk.MetadataRef != MetadataRef)
				{
					continue;
				}

				MetadataChunk.Chunk->GetAverageValue(
					MetadataRef,
					Alphas[It.Key.ToIndex()],
					*Buffer,
					It.Key.ToIndex());

				break;
			}
		}

		const TVoxelRefCountPtr<FVoxelHeightMetadataChunk> MetadataChunk = FVoxelHeightMetadataChunk::Create(
			Alphas,
			*Buffer,
			FIntPoint(0),
			FIntPoint(ChunkSize));

		if (!MetadataChunk)
		{
			continue;
		}

		Result.MetadataChunks.Add(FMetadataChunk
		{
			MetadataRef,
			MetadataChunk
		});
	}

	return Result;
}