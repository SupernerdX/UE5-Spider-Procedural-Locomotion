// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "Sculpt/Volume/VoxelVolumeChunkData.h"
#include "Sculpt/Volume/VoxelSculptVolumeData.h"
#include "Sculpt/Volume/VoxelVolumeDistanceChunk.h"
#include "Sculpt/VoxelSculptVersion.h"
#include "Surface/VoxelSurfaceTypeInterface.h"
#include "Sculpt/Volume/VoxelSculptVolumeLegacyData.h"

bool FVoxelVolumeChunkData::IsEmpty() const
{
	return
		!DistanceChunk &&
		!SurfaceTypeChunk &&
		MetadataChunks.Num() == 0;
}

void FVoxelVolumeChunkData::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();

	int32 Version = FVoxelSculptVersion::LatestVersion;
	Ar << Version;

	enum
	{
		Flags_HasDistanceChunk = (1 << 0),
		Flags_HasSurfaceTypeChunk = (1 << 1)
	};
	uint8 Flags = 0;

	if (DistanceChunk)
	{
		Flags |= Flags_HasDistanceChunk;
	}
	if (SurfaceTypeChunk)
	{
		Flags |= Flags_HasSurfaceTypeChunk;
	}

	Ar << Flags;

	if (Flags & Flags_HasDistanceChunk)
	{
		if (Version < FVoxelSculptVersion::AddUnifiedDistanceChunk)
		{
			EDistanceType DistanceType = {};
			Ar << DistanceType;

			DistanceChunk = SerializeDistanceChunk(Ar, Version, DistanceType);

			if (!ensure(DistanceChunk))
			{
				Ar.SetError();
				return;
			}
		}
		else
		{
			if (Ar.IsLoading())
			{
				DistanceChunk = FVoxelVolumeDistanceChunk::Load(Ar);
			}
			else
			{
				DistanceChunk->Save(Ar);
			}
		}
	}
	if (Flags & Flags_HasSurfaceTypeChunk)
	{
		if (Ar.IsLoading())
		{
			SurfaceTypeChunk = new FVoxelVolumeSurfaceTypeChunk();
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
			MetadataChunk.Chunk = new FVoxelVolumeMetadataChunk();
		}

		Ar << MetadataChunk.MetadataRef;
		ConstCast(*MetadataChunk.Chunk).Serialize(Ar, Version);
	}
}

void FVoxelVolumeChunkData::GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelVolumeChunkData FVoxelVolumeChunkData::Downsample(
	const TVoxelMap<FVoxelVolumeChunkKey, const FVoxelVolumeChunkData*>& KeyToChunkData,
	const float MaxErrorPercentage)
{
	VOXEL_FUNCTION_COUNTER();

	bool bHasDistances = false;
	bool bHasMovableDistances = false;
	bool bHasSurfaceTypes = false;
	TVoxelInlineArray<FVoxelMetadataRef, 8> MetadataRefs;

	for (const auto& It : KeyToChunkData)
	{
		if (It.Value->DistanceChunk)
		{
			bHasDistances = true;

			if (!It.Value->DistanceChunk->bIsPacked)
			{
				bHasMovableDistances = true;
			}
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

	FVoxelVolumeChunkData Result;

	if (bHasMovableDistances)
	{
		VOXEL_SCOPE_COUNTER("Movable distances");

		TVoxelStaticArray<float, ChunkCount> AdditiveDistances{ FVoxelUtilities::NaNf() };
		TVoxelStaticArray<float, ChunkCount> SubtractiveDistances{ FVoxelUtilities::NaNf() };

		for (const auto& It : KeyToChunkData)
		{
			if (!It.Value->DistanceChunk)
			{
				continue;
			}

			FVoxelMovableDistances MovableDistances;
			if (It.Value->DistanceChunk->bIsPacked)
			{
				MovableDistances = FVoxelMovableDistances(It.Value->DistanceChunk->GetPacked().GetAverageDistance());
			}
			else
			{
				MovableDistances = It.Value->DistanceChunk->GetMovable().GetAverageDistances();
			}

			AdditiveDistances[It.Key.ToIndex()] = MovableDistances.Additive / ChunkSize;
			SubtractiveDistances[It.Key.ToIndex()] = MovableDistances.Subtractive / ChunkSize;
		}

		Result.DistanceChunk = FVoxelVolumeDistanceChunk::CreateMovable(
			AdditiveDistances,
			SubtractiveDistances,
			FIntVector(0),
			FIntVector(ChunkSize));
	}
	else if (bHasDistances)
	{
		VOXEL_SCOPE_COUNTER("Distances");

		TVoxelStaticArray<float, ChunkCount> Distances{ FVoxelUtilities::NaNf() };

		for (const auto& It : KeyToChunkData)
		{
			if (!It.Value->DistanceChunk)
			{
				continue;
			}

			Distances[It.Key.ToIndex()] = It.Value->DistanceChunk->GetPacked().GetAverageDistance() / ChunkSize;
		}

		Result.DistanceChunk = FVoxelVolumeDistanceChunk::CreatePacked(
			Distances,
			FIntVector(0),
			FIntVector(ChunkSize),
			MaxErrorPercentage);
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

		Result.SurfaceTypeChunk = FVoxelVolumeSurfaceTypeChunk::Create(
			Alphas,
			SurfaceTypes,
			FIntVector(0),
			FIntVector(ChunkSize));
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

		const TVoxelRefCountPtr<FVoxelVolumeMetadataChunk> MetadataChunk = FVoxelVolumeMetadataChunk::Create(
			Alphas,
			*Buffer,
			FIntVector(0),
			FIntVector(ChunkSize));

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