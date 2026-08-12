// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMetadata.h"
#include "VoxelMetadataRef.h"
#include "Sculpt/VoxelSculptVersion.h"
#include "Sculpt/Height/VoxelHeightHeightChunk.h"
#include "Sculpt/Height/VoxelHeightMetadataChunk.h"
#include "Sculpt/Height/VoxelHeightSurfaceTypeChunk.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

class FVoxelSculptHeightLegacyData
{
public:
	static TVoxelRefCountPtr<FVoxelHeightHeightChunk> SerializeHeightChunk(
		FArchive& Ar,
		const bool bRelativeHeights)
	{
		int32 Version = 0;
		Ar << Version;

		TVoxelStaticArray<float, FVoxelHeightChunkDefinitions::ChunkCount> Heights{ NoInit };
		Ar << Heights;

		return FVoxelHeightHeightChunk::CreatePacked(
			Heights,
			FIntPoint(0),
			FIntPoint(FVoxelHeightChunkDefinitions::ChunkSize),
			0.f,
			bRelativeHeights);
	}

	template<typename T>
	static void SerializeLegacyChunkTree(
		FArchive& Ar,
		TVoxelMap<FIntPoint, TVoxelRefCountPtr<T>>& OutKeyToChunk,
		TVoxelFunctionRef<TVoxelRefCountPtr<T>()> SerializeChunk)
	{
		VOXEL_FUNCTION_COUNTER();

		using FVersion = DECLARE_VOXEL_VERSION
		(
			FirstVersion,
			LegacyVersion
		);

		int32 Version = FVersion::LatestVersion;
		Ar << Version;

		TVoxelArray<FIntPoint> ChunkKeys;
		Ar << ChunkKeys;

		for (const FIntPoint& ChunkKey : ChunkKeys)
		{
			OutKeyToChunk.Add_EnsureNew(ChunkKey, SerializeChunk());
		}
	}

	static void SerializeLegacyData(
		FArchive& InAr,
		bool bRelativeHeights,
		TVoxelMap<FIntPoint, TVoxelRefCountPtr<FVoxelHeightHeightChunk>>& OutKeyToHeightChunk,
		TVoxelMap<FIntPoint, TVoxelRefCountPtr<FVoxelHeightSurfaceTypeChunk>>& OutKeyToSurfaceChunk,
		TVoxelMap<FVoxelMetadataRef, TVoxelMap<FIntPoint, TVoxelRefCountPtr<FVoxelHeightMetadataChunk>>>& OutMetadataToKeyToMetadataChunk)
	{
		VOXEL_FUNCTION_COUNTER();

		FObjectAndNameAsStringProxyArchive StringAr(InAr, true);

		FArchive& Ar = InAr.GetArchiveName() == "FVoxelArchive" ? StringAr : InAr;

		using FVersion = DECLARE_VOXEL_VERSION
		(
			FirstVersion,
			FirstRefactor
		);

		int32 Version = FVersion::LatestVersion;
		Ar << Version;

		if (Version < FVersion::FirstRefactor)
		{
			FIntPoint MinKey;
			FIntPoint MaxKey;
			Ar << MinKey;
			Ar << MaxKey;

			TVoxelArray<FIntPoint> ChunkKeys;
			Ar << ChunkKeys;

			for (const FIntPoint& ChunkKey : ChunkKeys)
			{
				OutKeyToHeightChunk.Add_EnsureNew(
					ChunkKey,
					SerializeHeightChunk(Ar, bRelativeHeights));
			}

			return;
		}

		SerializeLegacyChunkTree<FVoxelHeightHeightChunk>(
			Ar,
			OutKeyToHeightChunk,
			[&]
			{
				return SerializeHeightChunk(Ar, bRelativeHeights);
			});

		SerializeLegacyChunkTree<FVoxelHeightSurfaceTypeChunk>(
			Ar,
			OutKeyToSurfaceChunk,
			[&]
			{
				TVoxelRefCountPtr<FVoxelHeightSurfaceTypeChunk> Result = new FVoxelHeightSurfaceTypeChunk();
				Result->Serialize(Ar, FVoxelSculptVersion::AddUnifiedDistanceChunk);
				return Result;
			});

		TVoxelArray<UVoxelMetadata*> Metadatas;
		Ar << Metadatas;

		OutMetadataToKeyToMetadataChunk.Reset();
		OutMetadataToKeyToMetadataChunk.Reserve(Metadatas.Num());

		for (UVoxelMetadata* Metadata : Metadatas)
		{
			const FVoxelMetadataRef MetadataRef(Metadata);

			TVoxelMap<FIntPoint, TVoxelRefCountPtr<FVoxelHeightMetadataChunk>> KeyToChunk;
			SerializeLegacyChunkTree<FVoxelHeightMetadataChunk>(
				Ar,
				KeyToChunk,
				[&]
				{
					TVoxelRefCountPtr<FVoxelHeightMetadataChunk> Result = new FVoxelHeightMetadataChunk();
					Result->Serialize(Ar, FVoxelSculptVersion::AddUnifiedDistanceChunk);
					return Result;
				});

			if (!ensureVoxelSlow(Metadata))
			{
				continue;
			}

			OutMetadataToKeyToMetadataChunk.Add_EnsureNew(MetadataRef) = MoveTemp(KeyToChunk);
		}
	}
};