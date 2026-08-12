// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadata.h"
#include "VoxelMetadataRef.h"
#include "Sculpt/VoxelSculptVersion.h"
#include "Sculpt/Volume/VoxelVolumeDistanceChunk.h"
#include "Sculpt/Volume/VoxelVolumeMetadataChunk.h"
#include "Sculpt/Volume/VoxelVolumeSurfaceTypeChunk.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

enum class EDistanceType : uint8
{
	Default,
	FullPrecision,
	Movable
};

inline TVoxelRefCountPtr<FVoxelVolumeDistanceChunk> SerializeDistanceChunk(
	FArchive& Ar,
	const int32 Version,
	const EDistanceType DistanceType)
{
	if (DistanceType == EDistanceType::Default)
	{
		TVoxelStaticArray<int8, FVoxelVolumeChunkDefinitions::ChunkCount> PackedDistances{ NoInit };
		Ar << PackedDistances;

		TVoxelStaticArray<float, FVoxelVolumeChunkDefinitions::ChunkCount> Distances{ NoInit };
		for (int32 Index = 0; Index < FVoxelVolumeChunkDefinitions::ChunkCount; Index++)
		{
			Distances[Index] = (PackedDistances[Index] + 0.5f) * 0.1087f;
		}

		return FVoxelVolumeDistanceChunk::CreatePacked(
			Distances,
			FIntVector(0),
			FIntVector(FVoxelVolumeChunkDefinitions::ChunkSize),
			0.f);
	}
	else if (DistanceType == EDistanceType::FullPrecision)
	{
		TVoxelStaticArray<float, FVoxelVolumeChunkDefinitions::ChunkCount> Distances{ NoInit };
		Ar << Distances;

		return FVoxelVolumeDistanceChunk::CreatePacked(
			Distances,
			FIntVector(0),
			FIntVector(FVoxelVolumeChunkDefinitions::ChunkSize),
			0.f);
	}
	else
	{
		check(DistanceType == EDistanceType::Movable);

		using FMovableVersion = DECLARE_VOXEL_VERSION
		(
			FirstVersion,
			InvertSubtractiveDistances,
			OptionalDiffing,
			RemoveOptionalDiffing
		);

		int32 LegacyVersion = FMovableVersion::LatestVersion;

		if (Version < FVoxelSculptVersion::MergeVersions)
		{
			Ar << LegacyVersion;

			if (LegacyVersion >= FMovableVersion::OptionalDiffing &&
				LegacyVersion < FMovableVersion::RemoveOptionalDiffing)
			{
				bool bEnableDiffing = true;
				Ar << bEnableDiffing;

				if (!ensure(bEnableDiffing))
				{
					Ar.SetError();
					return {};
				}
			}
		}

		TVoxelStaticArray<float, FVoxelVolumeChunkDefinitions::ChunkCount> AdditiveDistances{ NoInit };
		TVoxelStaticArray<float, FVoxelVolumeChunkDefinitions::ChunkCount> SubtractiveDistances{ NoInit };

		Ar << AdditiveDistances;
		Ar << SubtractiveDistances;

		if (LegacyVersion < FMovableVersion::InvertSubtractiveDistances)
		{
			for (float& Distance : SubtractiveDistances)
			{
				if (!FVoxelUtilities::IsNaN(Distance))
				{
					Distance = -Distance;
				}
			}
		}

		return FVoxelVolumeDistanceChunk::CreateMovable(
			AdditiveDistances,
			SubtractiveDistances,
			FIntVector(0),
			FIntVector(FVoxelVolumeChunkDefinitions::ChunkSize));
	}
}

template<typename T>
void SerializeLegacyChunkTree(
	FArchive& Ar,
	const int32 Version,
	TVoxelMap<FIntVector, TVoxelRefCountPtr<T>>& OutKeyToChunk,
	TVoxelFunctionRef<TVoxelRefCountPtr<T>()> SerializeChunk)
{
	VOXEL_FUNCTION_COUNTER();

	if (Version < FVoxelSculptVersion::MergeVersions)
	{
		using FVersion = DECLARE_VOXEL_VERSION
		(
			FirstVersion,
			LegacyVersion,
			RemoveMinMaxKey
		);

		int32 LegacyVersion = FVersion::LatestVersion;
		Ar << LegacyVersion;

		if (LegacyVersion < FVersion::RemoveMinMaxKey)
		{
			FIntVector MinKey;
			FIntVector MaxKey;

			Ar << MinKey;
			Ar << MaxKey;
		}
	}

	TVoxelArray<FIntVector> ChunkKeys;
	Ar << ChunkKeys;

	OutKeyToChunk.Reserve(ChunkKeys.Num());

	for (const FIntVector& ChunkKey : ChunkKeys)
	{
		OutKeyToChunk.Add_CheckNew(ChunkKey, SerializeChunk());
	}
}

inline void SerializeLegacyData(
	FArchive& InAr,
	TVoxelMap<FIntVector, TVoxelRefCountPtr<FVoxelVolumeDistanceChunk>>& OutKeyToDistanceChunk,
	TVoxelMap<FIntVector, TVoxelRefCountPtr<FVoxelVolumeSurfaceTypeChunk>>& OutKeyToSurfaceChunk,
	TVoxelMap<FVoxelMetadataRef, TVoxelMap<FIntVector, TVoxelRefCountPtr<FVoxelVolumeMetadataChunk>>>& OutMetadataToKeyToMetadataChunk)
{
	VOXEL_FUNCTION_COUNTER();

	FObjectAndNameAsStringProxyArchive StringAr(InAr, true);

	FArchive& Ar = InAr.GetArchiveName() == "FVoxelArchive" ? StringAr : InAr;

	using FVersion = DECLARE_VOXEL_VERSION
	(
		FirstVersion,
		AddDiffing,
		AddChunkTree,
		AddMetadata,
		AddSculptVersion,
		AddFastDistances,
		AddDistanceType
	);

	const int64 Offset = Ar.Tell();

	int32 Version = FVersion::LatestVersion;
	Ar << Version;

	EDistanceType DistanceType;

	if (Version >= FVersion::AddDistanceType)
	{
		Ar << DistanceType;
	}
	else
	{
		if (Version >= FVersion::AddFastDistances)
		{
			bool bUseFastDistances = false;
			Ar << bUseFastDistances;

			DistanceType = bUseFastDistances ? EDistanceType::Default : EDistanceType::Movable;
		}
		else
		{
			DistanceType = EDistanceType::Movable;
		}
	}

	int32 SculptVersion = FVoxelSculptVersion::LatestVersion;

	if (Version < FVersion::AddSculptVersion)
	{
		SculptVersion = FVoxelSculptVersion::FirstVersion;
	}
	else
	{
		Ar << SculptVersion;
	}

	if (Version < FVersion::AddDiffing)
	{
		return;
	}

	if (Version < FVersion::AddChunkTree)
	{
		Ar.Seek(Offset);

		SerializeLegacyChunkTree<FVoxelVolumeDistanceChunk>(Ar, SculptVersion, OutKeyToDistanceChunk, [&]() -> TVoxelRefCountPtr<FVoxelVolumeDistanceChunk>
		{
			return SerializeDistanceChunk(Ar, SculptVersion, EDistanceType::Movable);
		});

		return;
	}

	SerializeLegacyChunkTree<FVoxelVolumeDistanceChunk>(Ar, SculptVersion, OutKeyToDistanceChunk, [&]
	{
		return SerializeDistanceChunk(Ar, SculptVersion, DistanceType);
	});

	SerializeLegacyChunkTree<FVoxelVolumeSurfaceTypeChunk>(Ar, SculptVersion, OutKeyToSurfaceChunk, [&]
	{
		TVoxelRefCountPtr<FVoxelVolumeSurfaceTypeChunk> Result = new FVoxelVolumeSurfaceTypeChunk();
		Result->Serialize(Ar, SculptVersion);
		return Result;
	});

	if (Version < FVersion::AddMetadata)
	{
		return;
	}

	TVoxelArray<UVoxelMetadata*> Metadatas;
	Ar << Metadatas;

	for (UVoxelMetadata* Metadata : Metadatas)
	{
		const FVoxelMetadataRef MetadataRef(Metadata);

		TVoxelMap<FIntVector, TVoxelRefCountPtr<FVoxelVolumeMetadataChunk>> KeyToChunk;
		SerializeLegacyChunkTree<FVoxelVolumeMetadataChunk>(Ar, SculptVersion, KeyToChunk, [&]
		{
			TVoxelRefCountPtr<FVoxelVolumeMetadataChunk> Result = new FVoxelVolumeMetadataChunk();
			Result->Serialize(Ar, SculptVersion);
			return Result;
		});

		if (!ensureVoxelSlow(Metadata))
		{
			continue;
		}

		OutMetadataToKeyToMetadataChunk.Add_EnsureNew(MetadataRef) = MoveTemp(KeyToChunk);
	}
}