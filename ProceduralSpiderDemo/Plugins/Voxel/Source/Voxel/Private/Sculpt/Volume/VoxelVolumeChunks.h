// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "Bulk/VoxelBulkPtr.h"
#include "Bulk/VoxelBulkHint.h"
#include "Sculpt/Volume/VoxelVolumeChunkKey.h"
#include "Sculpt/Volume/VoxelVolumeChunkData.h"
#include "VoxelVolumeChunks.generated.h"

USTRUCT()
struct FVoxelVolumeNearChunkHint final : public FVoxelBulkHintData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	FVoxelBulkHash RootHash;
	FIntVector FarChunkKey;
	FIntVector MidChunkKey;
	FIntVector NearChunkKey;

	//~ Begin FVoxelBulkHintData Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelBulkHintData Interface
};

USTRUCT()
struct FVoxelVolumeNearChunk final : public FVoxelBulkData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	FVoxelVolumeChunkData ChunkData;

	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const override;
	//~ End FVoxelBulkData Interface
};

USTRUCT()
struct FVoxelVolumeMidChunkHint final : public FVoxelBulkHintData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	FVoxelBulkHash RootHash;
	FIntVector FarChunkKey;
	FIntVector MidChunkKey;

	//~ Begin FVoxelBulkHintData Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelBulkHintData Interface
};

USTRUCT()
struct FVoxelVolumeMidChunk final : public FVoxelBulkData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	FVoxelVolumeChunkData ChunkData;
	TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeNearChunk>> KeyToNearChunk;

	static TVoxelBulkRef<FVoxelVolumeMidChunk> Create(
		TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeNearChunk>> KeyToNearChunk,
		float MaxErrorPercentage);

	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const override;
	//~ End FVoxelBulkData Interface
};

USTRUCT()
struct FVoxelVolumeFarChunkHint final : public FVoxelBulkHintData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

	FVoxelBulkHash RootHash;
	FIntVector FarChunkKey;

	//~ Begin FVoxelBulkHintData Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End FVoxelBulkHintData Interface
};

USTRUCT()
struct FVoxelVolumeFarChunk final : public FVoxelBulkData
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	FVoxelVolumeChunkData ChunkData;
	TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeMidChunk>> KeyToMidChunk;

	static TVoxelBulkRef<FVoxelVolumeFarChunk> Create(
		TVoxelMap<FVoxelVolumeChunkKey, TVoxelBulkPtr<FVoxelVolumeMidChunk>> KeyToMidChunk,
		float MaxErrorPercentage);

	//~ Begin FVoxelBulkData Interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void GatherObjects(TVoxelSet<TVoxelObjectPtr<UObject>>& OutObjects) const override;
	//~ End FVoxelBulkData Interface
};